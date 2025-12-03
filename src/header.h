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
//
#define		VERSION		"V6.0"

#include <stdlib.h>
#include <Arduino.h>
#include <coredecls.h>
#include <time.h>
#include <TZ.h>
#include <sntp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_SSD1306.h>		// Adafruit SSD1306 Wemos Mini OLED
#include <ArduinoJson.h>			// ArduinoJson
#include <si5351.h>					// Etherkit
#include <JTEncode.h>				// Etherkit
#include <QTHLocator.h>				// Get the QTH locator from the internet
#include <WiFi_SSID.h>				// WiFi SSID's from know networks
#ifdef FEATURE_OTA					// OTA & mDNS (included)
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#endif


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

// Program config information
struct	config_t
{
	String		user_call;
	String		user_prefix;
	String		user_suffix;
	String		user_locator;
	uint8_t		user_power;

	String		system_version;
	uint32_t	system_chipid;
	String		system_hostname;
	uint32_t	system_display_off;
	uint32_t	system_randomSeed;

	bool		si5351_enable;			// Si5351 enable
	int			si5351_xtal_freq;		// Si5351 Xtal frequency 
	int			si5351_calibration;		// Si5351 frequency calibration in ppm
	int			si5351_drive_strength;	// Si5351 drive strength in mA

	bool		wspr_enable;			// WSPR TX enable
	float		wspr_band_freq;			// Current WSPR band frequency (0..200 Hz), 0 = random
	float		wspr_tone_mul;			// WSPR TX tone spacing multiply factor (1.0 ... 4.0), DEBUG

	bool		temp_enable;			// Temperature TX enable
	int			temp_band;				// Temperature band frequency
	float		temp_offset;			// Temperature offset correction
	int			temp_clk;				// Temperature Si5351 CLK output

	String		loc_lat_lon;			// Latitude and Longitude string
};

extern	config_t 	config;	// Config data from json file

#define		MYTZ				TZ_Europe_Amsterdam	// TZ string for currect location
#define		WSPR_SLOTS_HOUR		30			// 30 times 2min slots in a hour

#define		BUTTON_INPUT		D6			// GPIO 12, INPUT_PULLUP
#ifndef ONE_WIRE_BUS
#define		ONE_WIRE_BUS		D2			// D2-->GPIO 16, D1 Mini-->GPIO 04
#endif

// SSD1306 Display
#define		SCREEN_WIDTH		128							// OLED display width, in pixels
#define		SCREEN_HEIGHT		64							// OLED display height, in pixels
#define		OLED_RESET			-1							// Reset pin # (or -1 if sharing Arduino reset pin)
#define		SCREEN_ADDRESS		0x3C						// See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


//#define   CARRIER_FREQUENCY		(WSPR_TX_FREQ_17m + 100)
#define   CARRIER_FREQUENCY		(WSPR_TX_FREQ_40m + 100)
// #define	CARRIER_FREQUENCY		10000000UL
// #define	CARRIER_FREQUENCY		5000000UL
#define		CARRIER_SI5351_CLK		SI5351_CLK0

// *********************************************

extern		Adafruit_SSD1306	display;
extern		Si5351				si5351_clockgen;

const		uint32_t			value_ms_20ms_loop		= 20;				// 20ms interval check time ntp sec
const		uint32_t			value_us_wspr_bit		= 8192.0 / 12000.0 * 1000000.0;	 // Delay value for WSPR
const		uint32_t			value_us_one_second		= 1000000UL;		// micro second (us)
const		uint32_t			value_no_network		= 4 * 60 * 1000;	// 4 min (ntp must be init updated in this time)
const		uint32_t			value_ms_led_blink_on	= 3141UL;			// 4sec interval blink led
const		uint32_t			value_ms_led_blink_off	= 3UL;				// 3ms interval blink led
const		uint32_t			value_ms_reboot			= 1000UL;			// 1sec interval reboot

extern		uint32_t			timer_us_one_second;						// micros()
// extern		uint32_t			timer_ms_reboot;

extern		volatile bool		semaphore_wifi_connected;
extern		volatile bool		semaphore_wifi_ip_address;
extern		volatile bool		semaphore_wifi_ntp_received;

const		uint32_t			sntp_update_delay		= 3600 * 1000UL;	// NTP update every 1h

enum	wspr_type_t { WSPR_TX_NONE, WSPR_TX_TYPE_1, WSPR_TX_TYPE_2, WSPR_TX_TYPE_3 };

const		uint32_t			wspr_free_second		= 8192.0 / 12000.0 * WSPR_SYMBOL_COUNT + 1.0;
extern		uint32_t			wspr_tx_counter;
extern		uint32_t			wspr_symbol_index;


extern		QTHLocator			QTH;										// Get the QTH locator from the internet
extern		JsonDocument		jsonDoc;									// Allocate the JSON document
extern		uint8_t				wspr_slot_type[WSPR_SLOTS_HOUR];			// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
extern		uint32_t			wspr_slot_freq[WSPR_SLOTS_HOUR][3];			// TX frequency for every CLK output (0..2)
extern		uint32_t			wspr_slot_band[WSPR_SLOTS_HOUR];			// Band freqency, 0 .. 200 Hz


extern		uint8_t				hour_now;								// Time now in hour (0..23)
extern		uint8_t				slot_now;								// Time now in slot (0..29)
extern		uint8_t				slot_sec;								// 
extern		float				temperature_now;

typedef enum { DISPLAY_OFF,	DISPLAY_ON }	display_status_t;
extern		display_status_t display_status;

// Forward reference
void		setup_wifi();
void		init_wifi();
void		loop_wifi();
void		setup_display();
void		loop_display();
void		setup_si5351();
void		loop_si5351();
void		setup_wspr_tx();
void		loop_wspr_tx();
void		setup_ds18b20();
void		loop_ds18b20();
void		setup_button();
void		loop_button();

void		ssd1306_center_string(const char* buffer, uint8_t Y, uint8_t size);
void		ssd1306_background();
void		ssd1306_printf_P(int wait, PGM_P format, ...);
void		ssd1306_main_window();
void		ssd1306_display_on();
void		ssd1306_display_off();

void		readTemperature();
void		ssd1306_text(uint16_t delay_ms, const char* txt1, const char* txt2=NULL);
void		setupWifiStatioMode();
void		stopWifiStatioMode();
void		SetupSi5351();
bool		si5351_ready();
void		loop_wspr_tx();
void		loop_1s_tick();
void		wspr_tx_bit();
void		wspr_tx_init(const char* call);
void		wspr_tx_disable();
void		ssd1306_center_string(const char* buffer, uint8_t y, uint8_t size=1);
void		ssd1306_background();
void		makeSlotPlan();
void		setSlotTime();
bool		loadWebConfigData();
void		stop_wifi();

#ifdef DEBUG_ESP_PORT
// Put the strings in PROGMEM, slow but free some (constant) ram memory.
//  #define LOG_I(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
#ifdef NDEBUG
  #define LOG_I(F, ...)		{ if (display_status == DISPLAY_ON) DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
#else
  #define LOG_I(F, ...)		{ DEBUG_ESP_PORT.printf(PSTR(F), ## __VA_ARGS__); }
#endif
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
