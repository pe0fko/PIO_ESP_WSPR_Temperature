// *********************************************
// WSPR Clock TX ESP8266.
// 01/05/2021 Fred Krom, pe0fko
//
// Board: ESP8266	- LOLIN(WeMos) D1 R1 & mini
//					- CPU freq 80MHz
//					- Set Debug port on Serial1
//          - Erease Flash: "All Flash contents"
//
// WSPR type 1: CALL, LOC4, dBm
// WSPR type 2: p/CALL/s, dBm
// WSPR type 3: <p/CALL/s>, LOC6, dBm
// Update JTEncode.cpp library at line 1000 for 1 char prefix!
//
// *********************************************
// WSPR Type 1:
// The standard message is <callsign> + <4 character locator> + <dBm transmit power>;
// for example “K1ABC FN20 37” is a signal from station K1ABC in Maidenhead grid cell
// “FN20”, sending 37 dBm, or about 5.0 W (legal limit for 630 m).
// Messages with a compound callsign and/or 6 digit locator use a two-transmission sequence.
// WSPR Type 2:
// The <first transmission> carries compound callsign and power level, or standard callsign,
// 4 digit locator, and power level.
// WSPR Type 3:
// The <second transmission> carries a hashed callsign, 6 digit locator, and power level.
// Add-on prefixes can be up to three alphanumeric characters; add-on suffixes can be a
// single letter or one or two digits.
//

#define		VERSION		"V4.0"

// #include <stdlib.h>
// #include <Arduino.h>
// #include <coredecls.h>
// #include <time.h>
// #include <TZ.h>
// #include <sntp.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266WiFiMulti.h>		// Include the Wi-Fi-Multi library
// #include <ESP8266HTTPClient.h>
// #include <ArduinoJson.h>			// ArduinoJson
// #include <Adafruit_GFX.h>			// Adafruit GFX Library               1.11.5
// #include <Adafruit_SSD1306.h>		// Adafruit SSD1306 Wemos Mini OLED
// #include <si5351.h>					// Etherkit
// #include <JTEncode.h>				// Etherkit
// #include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20
// #include <WiFi_SSID.h>				// WiFi SSID's from know networks
#include <QTHLocator.h>				// Get the QTH locator from the internet

// #ifdef FEATURE_OTA					// OTA & mDNS (included)
// #include <ArduinoOTA.h>
// #include <ESP8266mDNS.h>
// #endif

//#define	URL_BASE	"http://sample.nl/wspr/client_"
//#define	URL_ID		"NL"

struct	config_t
{		String		call;
		String		prefix;
		String		suffix;
		String		qth;
		uint8_t		power;
		uint32_t	freq_cal_factor;
		uint32_t	randomSeed;
		String		hostname;
		int			displayOff;
		float		temp_cor;

};


#define	WSPR_TX_FREQ_160m		1838000UL		// 160m  1.838000 -  1.838200
#define	WSPR_TX_FREQ_80m		3570000UL		// 80m   3.570000 -  3.570200
#define	WSPR_TX_FREQ_60m		5288600UL		// 60m   5.288600 -  5.288800
#define	WSPR_TX_FREQ_40m		7040000UL		// 40m   7.040000 -  7.040200
#define	WSPR_TX_FREQ_30m		10140100UL		// 30m  10.140100 - 10.140300
#define	WSPR_TX_FREQ_20m		14097000UL		// 20m  14.097000 - 14.097200
#define	WSPR_TX_FREQ_17m		18106000UL		// 17m  18.106000 - 18.106200
#define	WSPR_TX_FREQ_15m		21096000UL		// 15m  21.096000 - 21.096200
#define	WSPR_TX_FREQ_12m		24926000UL		// 12m  24.926000 - 24.926200
#define	WSPR_TX_FREQ_10m		28126000UL		// 10m  28.126000 - 28.126200
#define	WSPR_TX_FREQ_6m			50294400UL		// 6m   50.294400 - 50.294600
#define	WSPR_TX_FREQ_4m			70092400UL		// 4m   70.092400 - 70.092600  ??
#define	WSPR_TX_FREQ_2m			144489900UL		// 2m  144.489900 - 144.490100
#define	WSPR_TX_FREQ_NONE		0UL				// No TX mode

#define		MYTZ				TZ_Europe_Amsterdam	// TZ string for currect location
#define		WSPR_SLOTS_HOUR		30			// 30 times 2min slots in a hour
#define		BUTTON_INPUT		D6			// GPIO 12, INPUT_PULLUP
#ifndef ONE_WIRE_BUS
#define		ONE_WIRE_BUS		D2			// D2-->GPIO 16, D1 Mini-->GPIO 04
#endif

//#define   CARRIER_FREQUENCY		(WSPR_TX_FREQ_17m + 100)
//#define   CARRIER_FREQUENCY		(WSPR_TX_FREQ_40m + 100)
#define		CARRIER_FREQUENCY		10000000UL
#define		CARRIER_SI5351_CLK		SI5351_CLK0

// *********************************************

#ifdef DEBUG_ESP_PORT
// Put the strings in PROGMEM, slow but free some (constant) ram memory.
//  #define LOG_I(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
//#define LOG_I(F, ...)		{ if (display_status == DISPLAY_ON) DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
  #define LOG_I(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
  #define LOG_D(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("DEBUG: " F), ## __VA_ARGS__); }
  #define LOG_W(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("WARNING: " F), ## __VA_ARGS__); }
  #define LOG_E(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("ERROR: " F), ## __VA_ARGS__); DEBUG_ESP_PORT.flush(); delay(100); }	// delay!!
  #define LOG_F(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR("FAIL: "  F), ## __VA_ARGS__); DEBUG_ESP_PORT.flush(); ESP.restart(); while(1); }
#else
  #define LOG_I(...)		{ }
  #define LOG_D(...)		{ }
  #define LOG_W(...)		{ }
  #define LOG_E(...)		{ }
  #define LOG_F(...)		{ }
#endif


			enum { WSPR_TX_NONE, WSPR_TX_TYPE_1, WSPR_TX_TYPE_2, WSPR_TX_TYPE_3 };


const		uint32_t			wspr_free_second		= 8192.0 / 12000.0 * WSPR_SYMBOL_COUNT + 1.0;

extern		QTHLocator			QTH;										// Get the QTH locator from the internet
extern		JsonDocument		jsonDoc;									// Allocate the JSON document
extern		int					wspr_slot_type[WSPR_SLOTS_HOUR];				// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
extern		uint32_t			wspr_slot_band[WSPR_SLOTS_HOUR];				// Band freqency, 0 .. 200 Hz
extern		uint32_t			wspr_slot_freq[WSPR_SLOTS_HOUR][3];			// TX frequency for every CLK output (0..2)

extern		uint8_t				hour_now;								// Time now in hour (0..23)
extern		uint8_t				slot_now;								// Time now in slot (0..29)
extern		uint8_t				slot_sec;								// 
extern		float				temperature_now;



// Forward reference
void		ReadTemperature();
void		ssd1306_display_on();
void		ssd1306_display_off();
void		ssd1306_text(uint16_t delay_ms, const char* txt1, const char* txt2=NULL);
void		ssd1306_printf_P(int wait, const char* formatP, ...);
void		ssd1306_wifi_page();
void		setupWifiStatioMode();
void		stopWifiStatioMode();
void		onWifiConnect(const WiFiEventStationModeConnected& ssid);
void		onWiFiGotIP(const WiFiEventStationModeGotIP& ipInfo);
void		onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo);
void		init_si5351();
bool		si5351_ready();
void		ssd1306_main_window();
void		sntp_time_is_set(bool from_sntp);
void		loop_wspr_tx();
void		loop_1s_tick();
void		loop_20ms_tick();
void		loop_wifi_tick();
void		loop_led_tick();
void		wspr_tx_bit();
void		wspr_tx_init(String& call);
void		wspr_tx_disable(si5351_clock clk);
void		wspr_tx_freq(si5351_clock clk);
void		wspr_tx_enable(si5351_clock clk);
void		ssd1306_center_string(const char* buffer, uint8_t y, uint8_t size=1);
void		ssd1306_background();
void		makeSlotPlan();
// void		makeSlotPlanEmpty();
// void		makeSlotPlanZone();
// void		makeSlotPlanClk(int clk, const char* zone);
// void		makeSlotPlanTemp();
int			getWsprSlotType(int type);
uint32_t	getWsprBandFreq(int band);
void		jsonSetConfig(String json);
const char* getJsonClkArray(int clk);
void		setSlotTime();
bool		loadWebConfigData();
// void		printJsonDoc(JsonObject& jsonDoc);
// void		printJsonDoc(String txt, JsonObject& jsonDoc);
// void		printJsonDoc(String txt, JsonDocument& jsonDoc);

