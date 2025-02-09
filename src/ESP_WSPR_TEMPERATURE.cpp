// *********************************************
// WSPR Clock TX ESP8266.
// 01/05/2021 Fred Krom, pe0fko
//
// Board: ESP8266	- LOLIN(WeMos) D1 R1 & mini
//					- CPU freq 80MHz
//					- Set Debug port on Serial1
//          - Erease Flash: "All Flash contents"
//
// *********************************************

#define		VERSION		"V3.2"

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

// WSPR type 1: CALL, LOC4, dBm
// WSPR type 2: p/CALL/s, dBm
// WSPR type 3: <p/CALL/s>, LOC6, dBm

// Update JTEncode.cpp library at line 1000 for 1 char prefix!

#if defined LOC_PE0FKO_40m
	#define	HAM_PREFIX      ""				// Prefix of the ham call
	#define	HAM_CALL        "PE0FKO"        // Ham radio call sign
	#define	HAM_SUFFIX      ""				// Suffix of the ham call
	#define	HAM_LOCATOR     "JO32cd"		// JO32CD 40OJ
	#define	HAM_POWER       10				// Power TX in dBm, 9dBm measure
	#define	WSPR_TX_FREQ_0	WSPR_TX_FREQ_40m	// TX freqency Si5351 OSC 0
	#define	WSPR_TX_FREQ_1	WSPR_TX_FREQ_none	// TX freqency Si5351 OSC 1
	#define	WSPR_TX_FREQ_2	WSPR_TX_FREQ_none	// TX freqency Si5351 OSC 2
	#define	WSPR_OUT_CLK	0				// Si5351 Clock output
#elif defined LOC_PA_PE0FKO
	#define	HAM_PREFIX      "PA/"			// Prefix of the ham call
	#define	HAM_CALL        "PE0FKO"		// Ham radio call sign
	#define	HAM_SUFFIX      ""				// Suffix of the ham call
	#define	HAM_LOCATOR     "JO32cd"		// JO32CD 40OJ
	#define	HAM_POWER       10				// Power TX in dBm
	#define	WSPR_TX_FREQ_0	WSPR_TX_FREQ_40m
	#define	WSPR_TX_FREQ_1	WSPR_TX_FREQ_none
	#define	WSPR_TX_FREQ_2	WSPR_TX_FREQ_none
	#define	WSPR_OUT_CLK	0				// Si5351 Clock output
#elif defined LOC_LA_MOTHE_40m
	#define	HAM_PREFIX      "F/"			// Prefix of the ham call
	#define	HAM_CALL        "PE0FKO"		// Ham radio call sign
	#define	HAM_SUFFIX      ""				// Suffix of the ham call
	#define	HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
	#define	HAM_POWER       10				// Power TX in dBm
	#define	WSPR_TX_FREQ_0	WSPR_TX_FREQ_40m
	#define	WSPR_TX_FREQ_1	WSPR_TX_FREQ_none
	#define	WSPR_TX_FREQ_2	WSPR_TX_FREQ_none
	#define	WSPR_OUT_CLK	0				// Si5351 Clock output
#elif defined LOC_LA_MOTHE_30m
	#define	HAM_PREFIX      "F/"			// Prefix of the ham call
	#define	HAM_CALL        "PE0FKO"		// Ham radio call sign
	#define	HAM_SUFFIX      ""				// Suffix of the ham call
	#define	HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
	#define	HAM_POWER       5				// Power TX in dBm
	#define	WSPR_TX_FREQ_0	WSPR_TX_FREQ_40m
	#define	WSPR_TX_FREQ_1	WSPR_TX_FREQ_none
	#define	WSPR_TX_FREQ_2	WSPR_TX_FREQ_none
	#define	WSPR_OUT_CLK	0				// Si5351 Clock output
#elif defined LOC_LA_MOTHE_20m
	#define	HAM_PREFIX      "F/"			// Prefix of the ham call
	#define	HAM_CALL        "PE0FKO"		// Ham radio call sign
	#define	HAM_SUFFIX      ""				// Suffix of the ham call
	#define	HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
	#define	HAM_POWER       4				// Power TX in dBm
	#define	WSPR_TX_FREQ_0	WSPR_TX_FREQ_40m
	#define	WSPR_TX_FREQ_1	WSPR_TX_FREQ_none
	#define	WSPR_TX_FREQ_2	WSPR_TX_FREQ_none
	#define	WSPR_OUT_CLK	0				// Si5351 Clock output
#else
  #error    "Specify the Location..."
#endif

#define	WSPR_TX_FREQ_160m		1838000UL	// 160m  1.838000 -  1.838200
#define	WSPR_TX_FREQ_80m		3570000UL	// 80m   3.570000 -  3.570200
#define	WSPR_TX_FREQ_60m		5288600UL	// 60m   5.288600 -  5.288800
#define	WSPR_TX_FREQ_40m		7040000UL	// 40m   7.040000 -  7.040200
#define	WSPR_TX_FREQ_30m		10140100UL	// 30m  10.140100 - 10.140300
#define	WSPR_TX_FREQ_20m		14097000UL	// 20m  14.097000 - 14.097200
#define	WSPR_TX_FREQ_17m		18106000UL	// 17m  18.106000 - 18.106200
#define	WSPR_TX_FREQ_15m		21096000UL	// 15m  21.096000 - 21.096200
#define	WSPR_TX_FREQ_12m		24926000UL	// 12m  24.926000 - 24.926200
#define	WSPR_TX_FREQ_10m		28126000UL	// 10m  28.126000 - 28.126200
#define	WSPR_TX_FREQ_6m			50294400UL	// 6m   50.294400 - 50.294600
#define	WSPR_TX_FREQ_2m			144489900UL	// 2m  144.489900 - 144.490100
#define	WSPR_TX_FREQ_none		0UL			// No TX mode

#define		MYTZ				TZ_Europe_Amsterdam	// TZ string for currect location
#define		WSPR_SLOTS_MAX		30			// 30 times 2min slots in a hour
#define		BUTTON_INPUT		D6			// GPIO 12, INPUT_PULLUP
#define		ONE_WIRE_BUS		D7			// GPIO 13

//#define   CARRIER_FREQUENCY		(WSPR_TX_FREQ_17m + 100)
//#define   CARRIER_FREQUENCY		(WSPR_TX_FREQ_40m + 100)
#define		CARRIER_FREQUENCY		10000000UL
#define		CARRIER_SI5351_CLK		SI5351_CLK0

// *********************************************
#include <stdlib.h>
#include <Arduino.h>
#include <coredecls.h>
#include <time.h>
#include <TZ.h>
#include <sntp.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>			// Adafruit GFX Library               1.11.5
#include <Adafruit_SSD1306.h>		// Adafruit SSD1306 Wemos Mini OLED
#include <si5351.h>					// Etherkit
#include <JTEncode.h>				// Etherkit
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20
#include <ESP8266WiFiMulti.h>		// Include the Wi-Fi-Multi library
#include <WiFi_SSID.h>				// WiFi SSID's from know networks

#ifdef FEATURE_mDNS
#include <ESP8266mDNS.h>
#endif
#ifdef FEATURE_OTA
#include <ArduinoOTA.h>
#endif

#ifdef DEBUG_ESP_PORT
// Put the strings in PROGMEM, slow but free some (constant) ram memory.
 #define PRINT(S)			{ DEBUG_ESP_PORT.print(S); }
 #define PRINT_P(S)			{ DEBUG_ESP_PORT.print(PSTR(S)); }
// #define PRINTF(F,...)		{ DEBUG_ESP_PORT.printf_P(PSTR(F), ##__VA_ARGS__); }

 #define PRINTF(F,...)		{ if (display_status == DISPLAY_ON) DEBUG_ESP_PORT.printf(PSTR(F), ##__VA_ARGS__); }

// #define LOG_I(...)			{ if (display_status == DISPLAY_ON) DEBUG_ESP_PORT.printf( "I>" __VA_ARGS__ ); }
// #define LOG_W(...)			{ DEBUG_ESP_PORT.printf( "W>" __VA_ARGS__ ); }

#else
 #define PRINT(S)			{ }
 #define PRINT_P(...)		{ }
 #define PRINTF(...)		{ }
#endif

#define	SCREEN_WIDTH	128			// OLED display width, in pixels
#define	SCREEN_HEIGHT	64			// OLED display height, in pixels

#define OLED_RESET     -1 	// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C	///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

static		Adafruit_SSD1306	display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static		Si5351				si5351;
static		JTEncode			wspr;

static		WiFiEventHandler	mConnectHandler;							// WiFi event handler for the Connect
static		WiFiEventHandler	mDisConnectHandler;							// WiFi event handler for the Disconnect
static		WiFiEventHandler	mGotIpHandler;								// WiFi event handler for the GotIP
static		OneWire				oneWire(ONE_WIRE_BUS);						// Temp sensor
static		DallasTemperature	sensors(&oneWire);							// Dallas DS18B20

const		uint32_t			value_ms_20ms_loop		= 20;				// 20ms interval check time ntp sec
static		uint32_t			timer_ms_20ms_loop		= 0;
const		uint32_t			value_us_wspr_bit		= 8192.0 / 12000.0 * 1000000.0;	 // Delay value for WSPR
static		uint32_t			timer_us_wspr_bit		= 0;
const		uint32_t			value_us_one_second		= 1000000UL;		// micro second (us)
static		uint32_t			timer_us_one_second		= 0;				// micros()
static		uint32_t			timer_ms_display_auto_off;
const		uint32_t			value_ms_ntp_faild_reboot	= 4 * 60 * 1000;	// 4 min (ntp must be init updated in this time)
static		uint32_t			timer_ms_ntp_faild_reboot;
const		uint32_t			value_ms_led_blink_on	= 3141UL;			// 4sec interval blink led
static		uint32_t			timer_ms_led_blink_on	= 0;
const		uint32_t			value_ms_led_blink_off	= 3UL;				// 3ms led on
static		uint32_t			timer_ms_led_blink_off	= 0;
const		uint32_t			sntp_update_delay		= 3600 * 1000UL;	// NTP update every 1h
volatile	bool				ntp_time_sync			= false;
static		float				temperature_now			= 0.0;
static		uint8_t				switchStatusLast		= HIGH;				// last status hardware switch
static		enum { DISPLAY_OFF,	DISPLAY_ON }	display_status = DISPLAY_ON;

static		uint8_t				wspr_symbols[WSPR_SYMBOL_COUNT];
static		uint32_t			wspr_symbol_index		= 0;
static		uint32_t			wspr_tx_counter        	= 0;

static		uint8_t				now_hour;								// Time now in hour (0..23)
static		uint8_t				now_slot;								// Time now in slot (0..29)
static		uint8_t				now_slot_sec;							// Time now in hour (0..119)

static		enum { WSPR_TX_NONE, WSPR_TX_TYPE_1, WSPR_TX_TYPE_2, WSPR_TX_TYPE_3 }
								wspr_slot_tx[WSPR_SLOTS_MAX];				// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
static		uint32_t			wspr_slot_band[WSPR_SLOTS_MAX];				// Band freqency, 0 .. 200 Hz
static		uint32_t			wspr_slot_freq[WSPR_SLOTS_MAX][3];			// TX frequency for every CLK output (0..2)

const		uint32_t			wspr_free_second		= 8192.0 / 12000.0 * WSPR_SYMBOL_COUNT + 1.0;

const	int32_t      	wspr_sym_freq[4] =
{	static_cast<uint32_t> ( 0.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 1.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 2.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 3.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
};

static		uint32				chipIdIndex				= 0;
static	struct {	uint32_t	ChipId;					// ESP Chip ID
					int32_t 	FreqCorrection;			// Si5351 frequency correction, PPB
					uint32_t	RandomSeed;				// Daily pseudo random number, freq
					uint32_t	DisplayAutoOff;			// Switch display off timeout
					const char*	Hostname;				// mDNS & OTA hostname
					float		TempCorrection;			// DS18B20 temp correction, at 18/08/2022
				} ESPChipInfo[] 
=
{	{ 0x007b1372, -195,		0x19570215,	1*60000, "WsprTX", 	-3.7 }	// Arduino shield, TCXO
,	{ 0x0062df37, 620+142,	0x19561113, 5*60000, "WsprTST",	-1.0 }	// Breadboard, TCXO
,	{ 0xffffffff,	0,			0X5555,		1*60000, "WsprESP",  0.0 }	// Default
};

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

static void	init_mdns();
static void	init_si5351();
bool		si5351_ready();
void		ssd1306_main_window();
void		sntp_time_is_set(bool from_sntp);
void		loop_wspr_tx();
void		loop_1s_tick();
void		loop_20ms_tick();
void		loop_wifi_tick();
void		loop_led_tick();
void		wspr_tx_bit();
void		wspr_tx_init(const char* call);
void		wspr_tx_disable(si5351_clock clk);
void		wspr_tx_freq(si5351_clock clk);
void		wspr_tx_enable(si5351_clock clk);
void		ssd1306_center_string(const char* buffer, uint8_t y, uint8_t size=1);
void		ssd1306_background();

//
// Make a plan to TX in one of the 30 slots (2min inteval in a hour).
//
void make_slot_plan(bool setup)
{
	int rnd0 = random(20, 180);				// All TX in the hour on the same band
	for (int i = 0; i < WSPR_SLOTS_MAX; i++)
	{
		wspr_slot_tx  [i]		= WSPR_TX_NONE;
		wspr_slot_band[i]		= rnd0;
		wspr_slot_freq[i][0]	= WSPR_TX_FREQ_0;		// _none
		wspr_slot_freq[i][1]	= WSPR_TX_FREQ_1;
		wspr_slot_freq[i][2]	= WSPR_TX_FREQ_2;
	}

#ifdef FEATURE_1H_FAST_TX
	if (setup)
	{
		// Every odd slot a TX until the first hour.
		for (int i = 0; i < WSPR_SLOTS_MAX; i += 6)
		{
			wspr_slot_tx[i+0] = WSPR_TX_TYPE_2;	// 0 - Min:  0, 12, 24, 36, 48
			wspr_slot_tx[i+1] = WSPR_TX_NONE;	// 1 - Min:  2, 14, 26, 38, 50
			wspr_slot_tx[i+2] = WSPR_TX_TYPE_2;	// 2 - Min:  4, 16, 28, 40, 52
			wspr_slot_tx[i+3] = WSPR_TX_TYPE_3;	// 3 - Min:  6, 18, 30, 42, 54
			wspr_slot_tx[i+4] = WSPR_TX_NONE;	// 4 - Min:  8, 20, 32, 44, 56
			wspr_slot_tx[i+5] = WSPR_TX_NONE;	// 5 - Min: 10, 22, 34, 46, 58
		}
	}
	else
#endif

#if 1
	// 241226:	Three slot tx.	40m, 80m, 20m.
	{
		for (int i = 0; i < WSPR_SLOTS_MAX;  i++)
		{
			wspr_slot_tx[i]	= WSPR_TX_TYPE_2;

			switch( i % 3 ) {
			case 0:
				wspr_slot_band[i]	= 50;
				wspr_slot_freq[i][WSPR_OUT_CLK]	= WSPR_TX_FREQ_40m;
				break;
			case 1:
				wspr_slot_band[i]	= 100;
				wspr_slot_freq[i][WSPR_OUT_CLK]	= WSPR_TX_FREQ_80m;
				break;
			case 2:
				wspr_slot_band[i]	= 150;
				wspr_slot_freq[i][WSPR_OUT_CLK]	= WSPR_TX_FREQ_20m;
				break;
			}
		}

		// TX ones every hour the 6 char QTH locator every band.
		wspr_slot_tx[0]		= WSPR_TX_TYPE_3;
		wspr_slot_tx[1]		= WSPR_TX_TYPE_3;
		wspr_slot_tx[2]		= WSPR_TX_TYPE_3;
	}
#elif 0
	//	Even slot 40m, odd 20m.
	{
//		int bnd = 25;					// Use audio band 25--175 Hz
		for (int i = 0; i < WSPR_SLOTS_MAX; 
				i += 1)
		{
			wspr_slot_band[i]		= (i & 1) == 1 ? 150 : 50;
//			wspr_slot_band[i]		= (i & 2) == 2 ? 150 : 50;
//			wspr_slot_band[i]		= bnd;
//			bnd += 5;
//			if (bnd >= 175) bnd = 25; 	// Step size if 5 Hz

			wspr_slot_tx  [i]		= WSPR_TX_TYPE_2;
			wspr_slot_freq[i][WSPR_OUT_CLK]	= i & 1 ? WSPR_TX_FREQ_20m : WSPR_TX_FREQ_40m;
		}

		// TX ones every hour the 6 char QTH locator.	
		wspr_slot_tx  [2]			= WSPR_TX_TYPE_3;
	}
#endif

#if 1
//	Add the temperature coding in a seperate hf band.
	{
//		int   s0,s1,s2,s3,t;
		int   s1,s2,s3,t;
		float tf;

		ReadTemperature();

		//Convert temperature to integer value
		// >-20 ... <50 ==> 0 ... 70 ==> *10 = 0 ... 700
		tf = temperature_now + 20.0;			//== Negative start offset
		if (tf < 0.0) tf = 0.0;
		if (tf >= 70.0) tf = 70.0 - 0.1;
		tf *= 10;								//== Decimal steps
		t = (int)(tf + 0.5);					// Rounding

		s1 =  0 + t / 100;	t %= 100;			// 0-6 :  0 .. 12 min
		s2 = 10 + t /  10;	t %= 10;			// 0-9 : 20 .. 38 min
		s3 = 20 + t /   1;						// 0-9 : 40 .. 58 min
//		s0 = s1 + 1;							// After first digit tx

//		PRINTF("TX Slots: loc[%d], temp[%d, %d, %d]\n", s0, s1, s2, s3);
		PRINTF("TX Slots: temperature code [%d, %d, %d]\n", s1, s2, s3);

//		wspr_slot_tx[s0]		= WSPR_TX_TYPE_3;	// TX locator 6
		wspr_slot_tx[s1]		= WSPR_TX_TYPE_2;	// Comp, no locator
		wspr_slot_tx[s2]		= WSPR_TX_TYPE_2;	// Comp, no locator
		wspr_slot_tx[s3]		= WSPR_TX_TYPE_2;	// Comp, no locator

//		wspr_slot_freq[s0][WSPR_OUT_CLK]	= WSPR_TX_FREQ_15m;
		wspr_slot_freq[s1][WSPR_OUT_CLK]	= WSPR_TX_FREQ_15m;
		wspr_slot_freq[s2][WSPR_OUT_CLK]	= WSPR_TX_FREQ_15m;
		wspr_slot_freq[s3][WSPR_OUT_CLK]	= WSPR_TX_FREQ_15m;
	}
#endif


#ifdef FEATURE_PRINT_TIMESLOT
	PRINT_P("Time Slot:\n");
	for(uint8_t i=0; i < WSPR_SLOTS_MAX; ++i) {
		if (wspr_slot_tx[i] != WSPR_TX_NONE)
			PRINTF("\t%02d:WSPR%d-[%d,%d,%d]+%d\n", i, wspr_slot_tx[i], wspr_slot_freq[i][0], wspr_slot_freq[i][1], wspr_slot_freq[i][2], wspr_slot_band[i]);
	}
	PRINT_P("\n");
#endif
}

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);				// LED on the ESP
	digitalWrite(LED_BUILTIN, HIGH);			// High is off!

	// Find the ESP chip-id specific data.
	chipIdIndex = 0;
	while (ESPChipInfo[chipIdIndex].ChipId != ESP.getChipId() && ESPChipInfo[chipIdIndex].ChipId != 0xffffffff)
		chipIdIndex++;

	randomSeed(ESPChipInfo[chipIdIndex].RandomSeed);

	Serial.begin(115200);
	Serial.setTimeout(2000);
	while(!Serial) yield();
#ifdef DEBUG_ESP_PORT
	Serial.setDebugOutput(true);
#endif
	delay(1000);

	PRINT_P ("\n=== PE0FKO, TX WSPR temperature coded\n");
	PRINT_P ("=== Version: " VERSION ", Build at: " __DATE__ " " __TIME__ "\n");
	PRINTF("=== Config: " HAM_PREFIX HAM_CALL HAM_SUFFIX " - " HAM_LOCATOR " - %ddBm\n", HAM_POWER);
	PRINTF("=== ChipId: 0x%08x, Host:%s, FreqCor=%d, TempCor=%f\n", 
			ESP.getChipId(),
			ESPChipInfo[chipIdIndex].Hostname, 
			ESPChipInfo[chipIdIndex].FreqCorrection,
			ESPChipInfo[chipIdIndex].TempCorrection
	);

	PRINTF("value_us_wspr_bit: %ld\n", value_us_wspr_bit);
	PRINTF("SSD1306: %dx%d addr:0x%02x\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH, SCREEN_ADDRESS);

	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

//	display.setRotation(2);						// Display upside down...

	pinMode(BUTTON_INPUT, INPUT_PULLUP);		// Button for display on/off
	ssd1306_display_on();						// Start the display ON timer

//RAM:   [====      ]  40.2% (used 32928 bytes from 81920 bytes)
//Flash: [====      ]  35.5% (used 370267 bytes from 1044464 bytes)
	ssd1306_printf_P(800, PSTR("Hostname\n%s\n.local"), ESPChipInfo[chipIdIndex].Hostname);

//RAM:   [====      ]  40.2% (used 32944 bytes from 81920 bytes)
//Flash: [====      ]  35.5% (used 370263 bytes from 1044464 bytes)
//	ssd1306_printf_P(800, "Hostname\n%s\n.local", ESPChipInfo[chipIdIndex].Hostname);

	setupWifiStatioMode();

#ifdef FEATURE_mDNS
	init_mdns();						// Init the broadcast DNS server (.local)
#endif

#ifdef FEATURE_OTA
	// Start OTA server.
	ArduinoOTA.setHostname(ESPChipInfo[chipIdIndex].Hostname);
	ArduinoOTA.onStart([]() { ssd1306_printf_P(100, PSTR("OTA update\nRunning")); });
	ArduinoOTA.onEnd([]()   { ssd1306_printf_P(100, PSTR("OTA update\nReboot")); ESP.restart(); });
	ArduinoOTA.setPassword(OtaPassword);
	ArduinoOTA.begin();
#endif

	sensors.begin();					// Init the onewire for the DS18B20 temp sensor
	ReadTemperature();					// Read the Dallas temperature one-wire sensor

	init_si5351();						// Init the frequency generator SI5351

	make_slot_plan(true);				// The first hour slot plan definition

#ifdef FEATURE_CARRIER
	PRINTF("CW Carrier on: %fMHz (CLK%d)\n", (float)CARRIER_FREQUENCY/1e6, CARRIER_SI5351_CLK);
	si5351.set_freq( SI5351_FREQ_MULT * CARRIER_FREQUENCY, CARRIER_SI5351_CLK );
	wspr_tx_enable(CARRIER_SI5351_CLK);
#endif

	ssd1306_printf_P(300, PSTR("Start\nLooping"));
	PRINT_P("=== Start looping...\n");

	ssd1306_main_window();
}

void setupWifiStatioMode()
{
	timer_ms_ntp_faild_reboot = millis();						// Keep track of the first received NTP
	ntp_time_sync = false;

	WiFi.disconnect(false);										// Cleanup old info
	WiFi.mode(WIFI_STA);										// Set WiFi to station mode

	WiFi.setHostname(ESPChipInfo[chipIdIndex].Hostname);		// Set Hostname.
	wifi_station_set_hostname(ESPChipInfo[chipIdIndex].Hostname);

	WiFi.setAutoReconnect(true);								// Keep WiFi connected

	// Register WiFi event handlers
	mConnectHandler		= WiFi.onStationModeConnected(onWifiConnect);
	mGotIpHandler		= WiFi.onStationModeGotIP(onWiFiGotIP);
	mDisConnectHandler	= WiFi.onStationModeDisconnected(onWifiDisconnect);

	// Try to startup the WiFi Multi connection with the strongest AP found.
	for(int i = 0; i < WifiApListNumber; i++)
		wifiMulti.addAP(WifiApList[i].ssid, WifiApList[i].passphrase);
}

void stopWifiStatioMode()
{
	WiFi.disconnect(true);		// WiFi off set true
	WiFi.mode(WIFI_OFF);		// Needed??
}

void onWifiConnect(const WiFiEventStationModeConnected& ssid)
{
	PRINTF("WiFi connected: SSID %s, channel %d\n", ssid.ssid.c_str(), ssid.channel);
}

void onWiFiGotIP(const WiFiEventStationModeGotIP& ipInfo)
{
	PRINTF("WiFi IP %s, mask %s, GW:%s\n",
		ipInfo.ip.toString().c_str(),
		ipInfo.mask.toString().c_str(),
		ipInfo.gw.toString().c_str()
	);

	settimeofday_cb(sntp_time_is_set);			// Call-back NTP function
	configTime(MYTZ, "time.google.com", "nl.pool.ntp.org");

//	timer_ms_ntp_faild_reboot = millis();		// Keep track of the first received NTP
}

// callback routine - arrive here whenever a successful NTP update has occurred
void sntp_time_is_set(bool from_sntp)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);					// Get the current time in sec and usec
	timer_us_one_second = micros();				// Initialize the timer only ones!
	timer_us_one_second -= tv.tv_usec;			// Correct the us to the sec tick

//	ntp_time_sync_first = true;					// There is some time, not the Unix Epoch.
	ntp_time_sync = true;						// and the new time is set

	timer_ms_ntp_faild_reboot = 0;				// NTP Received ok, no reboot needed!

//	stopWifiStatioMode();						// Stop the WiFi connection

	PRINTF("NTP update at [%dus] [%d] %s", tv.tv_usec, timer_us_one_second, ctime(&tv.tv_sec));
}

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 ()
{
	return sntp_update_delay;
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo)
{
	PRINTF("WiFi disconnected from SSID: %s, Reason: %d\n"
	,	disconnectInfo.ssid.c_str()
	,	disconnectInfo.reason
	);

	sntp_stop();

	ntp_time_sync = false;
}

void ReadTemperature()
{
	sensors.requestTemperatures();
	temperature_now = sensors.getTempCByIndex(0) + ESPChipInfo[chipIdIndex].TempCorrection;

	PRINTF("Sensor DS18B20 temperature %.1fºC\n", temperature_now);
}

#ifdef FEATURE_mDNS
static void init_mdns()
{
	ssd1306_printf_P(200, PSTR("mDNS\n%s"), ESPChipInfo[chipIdIndex].Hostname);

	if (MDNS.begin(ESPChipInfo[chipIdIndex].Hostname))
		MDNS.addService("http", "tcp", 80);
	else
		PRINT_P("mDNS ERROR!\n");
}
#endif

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------
void loop()
{
	// Wait for the (NTP) time is known!
	if (timer_ms_ntp_faild_reboot == 0)
	{
		loop_wspr_tx();
		loop_1s_tick();
		loop_20ms_tick();
	}
	else
	{
		if ((millis() - timer_ms_ntp_faild_reboot) >= value_ms_ntp_faild_reboot)
		{
			ssd1306_printf_P(100, PSTR("REBOOT\nNTP sync"));
			delay(5000);
			PRINT_P("REBOOT: No NTP time received.\n");
			Serial.flush();
			ESP.restart();
		}

		// Maak langere loops met timer
//		ssd1306_printf_P(200, PSTR("Waiting\nNTP first sync"));
	}

	loop_wifi_tick();
	loop_led_tick();

#ifdef FEATURE_OTA
	ArduinoOTA.handle();
#endif

#ifdef FEATURE_mDNS
	MDNS.update();
#endif
}

void loop_wspr_tx()
{
	// Send the WSPR bits into the air if active TX!
	// When started it will wait for the bit time and start a next bit.

#ifdef FEATURE_CHECK_TIMING
	if (wspr_symbol_index != 0) 
	{
		uint32_t	diff = micros() - timer_us_wspr_bit;
		if (diff >= value_us_wspr_bit)
		{
			timer_us_wspr_bit += value_us_wspr_bit;
			wspr_tx_bit();										// Ok, transmit the net tone bit

			if (diff >= (value_us_wspr_bit+500UL))
			{
				Serial.printf("WSPT-Bit %u overflow %d us.\n", wspr_symbol_index, diff - value_us_wspr_bit);
			}
		}
	}
#else
	if ((wspr_symbol_index != 0) 
	&&  (micros() - timer_us_wspr_bit) >= value_us_wspr_bit)
	{
		timer_us_wspr_bit += value_us_wspr_bit;
		wspr_tx_bit();											// Ok, transmit the net tone bit
	}
#endif
}

// Secure one second function call
void loop_1s_tick() 
{
	if ((micros() - timer_us_one_second) < value_us_one_second)
		return;

	struct timeval tv;
	gettimeofday(&tv, NULL);								// Get the current time in usec

	timer_us_one_second +=	value_us_one_second;

	now_hour		= tv.tv_sec % (3600 * 24)	/ 3600;
	now_slot		= tv.tv_sec % (3600) 		/ 120;
	now_slot_sec	= tv.tv_sec % (120);

	//++ At every 2 minute interval start a WSPR message, if slot is richt.
	if (now_slot_sec == 0)											// First second of the 2 minute interval clock
	{
		if (now_slot == 0)
			make_slot_plan(false);

		if (wspr_slot_tx[now_slot] == WSPR_TX_TYPE_1)			// Type 1 message: CALL, LOC4, dBm
			wspr_tx_init(HAM_CALL);

		else if (wspr_slot_tx[now_slot] == WSPR_TX_TYPE_2)		// Type 2 message: pre/CALL/suff, dBm
			wspr_tx_init(HAM_PREFIX HAM_CALL HAM_SUFFIX);

		else if (wspr_slot_tx[now_slot] == WSPR_TX_TYPE_3)		// Type 3 message: hash <pre/CALL/suff>, LOC6, dBm
			wspr_tx_init("<" HAM_PREFIX HAM_CALL HAM_SUFFIX ">");

		Serial.printf("WSPR-Time: %2u:%02u\n", now_hour, now_slot);
	}

	// Only read the temp once every 2min
	if (display_status == DISPLAY_ON && now_slot_sec == wspr_free_second) 
		ReadTemperature();

	ssd1306_main_window();

	//++ Set the random seed ones every day.
	//   Posible track of (semi) random numbers!
	if (now_hour == 23 && now_slot == 29 && now_slot_sec == wspr_free_second)
	{
		PRINTF("Set the const ramdom seed number 0x%08x\n", ESPChipInfo[chipIdIndex].RandomSeed);
		randomSeed(ESPChipInfo[chipIdIndex].RandomSeed);
	}

//DEBUG:
//	if (now_slot_sec % 10 == 0)
//		Serial.printf("SECOND: %06ld us %s", tv.tv_usec, ctime(&tv.tv_sec));	//TEST
}


// Used for slower processing, timing from the cpu xtal
void loop_20ms_tick() 
{
	if ((millis() - timer_ms_20ms_loop) < value_ms_20ms_loop)	// Every 20ms...
		return;

	if (timer_ms_20ms_loop == 0) {
		timer_ms_20ms_loop = millis();
		return;
	}

	timer_ms_20ms_loop += value_ms_20ms_loop;

	// Check if button is pressed to lightup the display
	int switchStatus = digitalRead(BUTTON_INPUT);			// read status of switch
	if (switchStatus != switchStatusLast)					// if status of button has changed
	{
		  switchStatusLast = switchStatus;
		  if (switchStatus == LOW)
		  {
			display_status = display_status == DISPLAY_ON ? DISPLAY_OFF : DISPLAY_ON;

			if (display_status == DISPLAY_ON)
			{
				ssd1306_display_on();				  		// Start the display ON timer
				digitalWrite(LED_BUILTIN, HIGH);			// Switch the ESP LED off
				ReadTemperature();							// Get temperature
				ssd1306_main_window();						// Write to display
			}
			else
				ssd1306_display_off();

			PRINTF("Button pressed, display_status=%d\n", display_status);
		  }
	}
#if 0
	// Blink the ESP LED every 4s if display is off
	if (display_status == DISPLAY_OFF)
	{
		static uint8_t led_pwm;
		led_pwm %= 200;
		digitalWrite(LED_BUILTIN, led_pwm++ == 0 ? LOW : HIGH);
		delay(3);
		digitalWrite(LED_BUILTIN, HIGH);
	}
#endif
}

void loop_wifi_tick() 
{
	if (wspr_symbol_index != 0)			// In wspr tx no Wifi actions
		return;

	if (wifiMulti.run((120 - 1 - wspr_free_second) * 1000UL) == WL_CONNECTED)
	{
		if (timer_ms_led_blink_on == 0)
		{
			timer_ms_led_blink_on = millis();
			ssd1306_wifi_page();							// WIFI down after NTP???
		}
	}
	else
	{
		timer_ms_led_blink_on = 0;
	}
}


void loop_led_tick() 
{
	if (timer_ms_led_blink_on != 0
	&&  (millis() - timer_ms_led_blink_on) >= value_ms_led_blink_on)
	{
		timer_ms_led_blink_on += value_ms_led_blink_on;
		digitalWrite(LED_BUILTIN, LOW);							// LED On
		timer_ms_led_blink_off = millis();
	}

	if (timer_ms_led_blink_off != 0
	&&	(millis() - timer_ms_led_blink_off) >= value_ms_led_blink_off)
	{
		timer_ms_led_blink_off = 0;
		digitalWrite(LED_BUILTIN, HIGH);						// LED Off
	}
}

static void init_si5351()
{
	PRINTF("SI5351 init: xtal:%d, correction:%d\n", SI5351_XTAL_FREQ, ESPChipInfo[chipIdIndex].FreqCorrection);

	ssd1306_printf_P(200, PSTR("SI5351\nSetup"));

	// TCXO input to xtal pin 1?
	if ( si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, ESPChipInfo[chipIdIndex].FreqCorrection) )
	{
		// Disable the clock initially...
		wspr_tx_disable(SI5351_CLK0);
		wspr_tx_disable(SI5351_CLK1);
		wspr_tx_disable(SI5351_CLK2);

		ssd1306_printf_P(200, PSTR("SI5351\nOk"));
	}
	else
	{
		ssd1306_printf_P(200, PSTR("SI5351\nERROR"));
	}
}

// Check if SI5351 is ready and connected
bool si5351_ready()
{
	uint8_t reg_val;

	Wire.begin();		// Start I2C comms
	// Check for a device on the bus, bail out if it is not there
	Wire.beginTransmission(SI5351_BUS_BASE_ADDR);
	reg_val = Wire.endTransmission();
	if(reg_val != 0)
	{
		ssd1306_printf_P(10*1000, PSTR("NO SI5351\n0x%04X"), reg_val);
		return false;
	}

	reg_val = si5351.si5351_read(SI5351_DEVICE_STATUS);
	reg_val &= 0b11100000;	// SYS_INIT | LOL_A | LOL_B
	if (reg_val)
	{
		ssd1306_printf_P(5*1000, PSTR("SI5351 Error\n0x%02x"), reg_val);
		PRINTF("WSPR TX, the SI5351 is not ready, 0x%02x\n", reg_val);
		return false;
	}

	return true;
}


void wspr_tx_init(const char* call)
{
	if (si5351_ready())
	{
		PRINTF("WSPR TX with Call: %s, Loc:%s, Power:%ddBm\n", call, HAM_LOCATOR, HAM_POWER);
		wspr.wspr_encode(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
		print_wspr_symbols(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);
#endif

		timer_us_wspr_bit = micros();
		wspr_tx_bit();
	}
}

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2
void print_wspr_symbols(const char* call, const char* loc, uint8_t power, uint8_t symbols[])
{
	PRINTF("%s %s %ddBm:\n  ", call, loc, power);
	for (uint8_t i = 0; i < WSPR_SYMBOL_COUNT; )
	{
		PRINTF("%d,", symbols[i++]);
		if (i % 41 == 0) PRINT_P("\n  ");
	}
	PRINT_P("\n");
}
#endif

void wspr_tx_bit()
{
	if (wspr_symbol_index != WSPR_SYMBOL_COUNT)
	{
		if (wspr_symbol_index == 0)   							// On first bit enable the tx output.
		{
			wspr_tx_enable(SI5351_CLK0);
			wspr_tx_enable(SI5351_CLK1);
			wspr_tx_enable(SI5351_CLK2);
		}

		wspr_tx_freq(SI5351_CLK0);
		wspr_tx_freq(SI5351_CLK1);
		wspr_tx_freq(SI5351_CLK2);

		wspr_symbol_index += 1;
	}
	else
	{
		wspr_tx_disable(SI5351_CLK0);
		wspr_tx_disable(SI5351_CLK1);
		wspr_tx_disable(SI5351_CLK2);

		wspr_symbol_index = 0;
		wspr_tx_counter += 1;

		PRINTF("TX WSPR %d Ended.\n", wspr_tx_counter);
	}
}

void wspr_tx_freq(si5351_clock clk)
{
	if (wspr_slot_tx[now_slot] != WSPR_TX_NONE && 
		wspr_slot_freq[now_slot][clk] != 0)
	{
		uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[now_slot][clk] + wspr_slot_band[now_slot]);
		if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) )
			PRINTF("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
	}
}

void wspr_tx_enable(si5351_clock clk)
{
	if (wspr_slot_tx[now_slot] != WSPR_TX_NONE && 
		wspr_slot_freq[now_slot][clk] != 0)
	{
		PRINTF("TX WSPR start %d: slot %d, freq %.6fMHz + %dHz\n", 
				clk, now_slot, 
				wspr_slot_freq[now_slot][clk] / 1000000.0, 
				wspr_slot_band[now_slot]);

		si5351.drive_strength(clk, SI5351_DRIVE_8MA); 			// 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
		si5351.set_clock_pwr(clk, 1);
		si5351.output_enable(clk, 1);
	}
 }

void wspr_tx_disable(si5351_clock clk)
{
	si5351.set_clock_pwr(clk, 0);
	si5351.output_enable(clk, 0);
}

void ssd1306_main_window()
{
	struct timeval	tv;
	char			buffer[40];
	int				ns = now_slot;	// next slot


	if (display_status == DISPLAY_OFF)
		return;

	if ((millis() - timer_ms_display_auto_off) >= ESPChipInfo[chipIdIndex].DisplayAutoOff)
	{
		ssd1306_display_off();
		return;
	}

	gettimeofday(&tv, NULL);									// Get the current time in usec

	ssd1306_background();

	if (ntp_time_sync)
	{
		// Get local time
		struct tm* timeinfo = localtime(&tv.tv_sec);

		// Disply the actual time
		strftime (buffer ,sizeof buffer, "%H:%M:%S", timeinfo);
		ssd1306_center_string(buffer, 12, 2);

		// Display the actual date and temperature
		char date[20];
		strftime (date ,sizeof date, "%d/%m/%Y", timeinfo);
		sprintf(buffer, "%s - %.1f", date, temperature_now);
		ssd1306_center_string(buffer, 32);
	}
	else
	{
		if (WiFi.status() != WL_CONNECTED)
			ssd1306_center_string("WiFi disconnect", 16+2);
		else
			ssd1306_center_string("sNTP Waiting", 16+2);
		ssd1306_center_string("- Syncing -", 32-2);
	}

	// Display the WSPR Call, Locator and Power in dBm
	sprintf(buffer, "%s/%s/%ddBm", HAM_CALL, HAM_LOCATOR, HAM_POWER);
	ssd1306_center_string(buffer, SCREEN_HEIGHT-10);

	if (wspr_symbol_index != 0)
	{
		uint16_t B,E;
		B = 4;
		E = B + tv.tv_sec % 120;
		display.drawLine(B, display.height()-14-4, E, display.height()-14-4, WHITE);
		display.drawLine(B, display.height()-16-4, E, display.height()-16-4, WHITE);

		// At TX invert the display collors
		display.invertDisplay(true);
	}
	else
	{
		// Calculate the next tx time and slot
		uint16_t w = 120 - (tv.tv_sec % 120);
		while (wspr_slot_tx[ns++] == WSPR_TX_NONE) 
		{
			if (ns >= WSPR_SLOTS_MAX) ns = 0;
			w += 120;
		}
		if (w < 3600)
		{
			// Display time next tx
			sprintf(buffer, "TX at %02d:%02d", w/60, w%60);
			ssd1306_center_string(buffer, display.height()-20);
		}

		// Non TX normal display
		display.invertDisplay(false);
	}

	// Display the used freq bands
	sprintf(buffer, "%s/%s/%s"
			, wspr_slot_freq[ns][0] == 0 ? "x" : String(wspr_slot_freq[ns][0]/1000000).c_str()
			, wspr_slot_freq[ns][1] == 0 ? "x" : String(wspr_slot_freq[ns][1]/1000000).c_str()
			, wspr_slot_freq[ns][2] == 0 ? "x" : String(wspr_slot_freq[ns][2]/1000000).c_str()
			);

	int16_t   		x,y;
	uint16_t  		w,h;

	display.getTextBounds(buffer, 0, 0, &x, &y, &w, &h);
	x = display.width() * 1 / 4;
	x -= w / 2; if (x < 10) x = 10;
	display.fillRect(x, y, w, h, BLACK);
	display.setCursor(x, y);
	display.print(buffer);

	display.display();
}

void ssd1306_center_string(const char* buffer, uint8_t Y, uint8_t size)
{
	int16_t   x, y;
	uint16_t  h, w;

	display.setTextSize(size);
	display.getTextBounds(buffer, 0, Y, &x, &y, &w, &h);
	display.setCursor((display.width() - w) / 2, y);
	display.print(buffer);
}


void ssd1306_background()
{
	char buf_count[20];
	int16_t   x, y;
	uint16_t  w, h;

	sprintf(buf_count, "%d", wspr_tx_counter);

	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.getTextBounds(buf_count, 0, 0, &x, &y, &w, &h);

	display.clearDisplay();
	display.drawRoundRect(x, h/2-1, display.width(), display.height() - h/2-1, 8, WHITE);

	x = display.width() * 3 / 4;
	x -= w / 2;
	display.fillRect(x, y, w, h, BLACK);
	display.setCursor(x, y);
	display.print(buf_count);
}

void ssd1306_wifi_page()
{
	PRINTF("Connected to %s (IP:%s/%s, GW:%s, RSSI %d)\n"
		, WiFi.SSID().c_str()
		, WiFi.localIP().toString().c_str()							// Send the IP address of the ESP8266 to the computer
		, WiFi.subnetMask().toString().c_str()
		, WiFi.gatewayIP().toString().c_str()
		, WiFi.RSSI());

	ssd1306_background();
	display.setTextSize(1);

	display.setCursor(6, 16+10*0);
 	display.printf("SSID:%s", WiFi.SSID().c_str());
	display.setCursor(6, 16+10*1);
 	display.printf("  IP:%s", WiFi.localIP().toString().c_str());
	display.setCursor(6, 16+10*2);
 	display.printf("RSSI:%d", WiFi.RSSI());

	ssd1306_display_on();
	display.display();

	delay(2000);
}

// PSTR("%lu;%s;%ld;%S")
// Note that the %S has a capital S because header_1 is in PROGMEM.
void ssd1306_printf_P(int wait, PGM_P format, ...)
{
	char buffer[100];
	va_list arg;

	va_start (arg, format);
	vsnprintf(buffer, sizeof(buffer), (const char *)format, arg);
	va_end (arg);

	char *txt1,*txt2,*txt3;
	txt1 = buffer;
	txt2 = strchr(txt1, '\n');
	if (txt2 != NULL) *txt2++ = '\0';
	txt3 = strchr(txt2, '\n');
	if (txt3 != NULL) *txt3++ = '\0';

	display.invertDisplay(false);
	ssd1306_background();

	if (txt1 != NULL)
		ssd1306_center_string(txt1, 16);
	if (txt2 != NULL)
		ssd1306_center_string(txt2, 16+12);
	if (txt3 != NULL)
		ssd1306_center_string(txt3, 16+12+12);

	if (WiFi.SSID().length() > 0)
	{
		String ssid("SSID:"); ssid += WiFi.SSID();
		ssd1306_center_string(ssid.c_str(), 16+12+12+12);
	}

	ssd1306_display_on();
	display.display();

#ifdef DEBUG_ESP_PORT
	PRINTF("SSD1306[[ ");
	if (txt1 != NULL)	PRINT(txt1);
	if (txt2 != NULL) {	PRINT_P(" / "); PRINT(txt2); }
	if (txt3 != NULL) {	PRINT_P(" / "); PRINT(txt3); }
	PRINTF(" ]](%dms)\n", wait);
#endif

	if (wait >= 0)
		delay(wait);
}

void ssd1306_display_on()
{
	PRINTF("Display On for %dsec.\n", ESPChipInfo[chipIdIndex].DisplayAutoOff/1000);

	timer_ms_display_auto_off = millis();		// Start the display ON timer
	display_status  = DISPLAY_ON;
}

void ssd1306_display_off()
{
	PRINTF("Display auto Off at %d sec.\n", ESPChipInfo[chipIdIndex].DisplayAutoOff/1000);

	display.clearDisplay();
	display.invertDisplay(false);
	display.display();
	display_status = DISPLAY_OFF;
}
