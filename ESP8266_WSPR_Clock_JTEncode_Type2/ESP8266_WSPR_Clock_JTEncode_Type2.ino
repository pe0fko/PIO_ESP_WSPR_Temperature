/* *********************************************
// WSPR Clock TX ESP8266.
// 01/05/2021 Fred Krom, pe0fko
//
// Board: ESP8266	- LOLIN(WeMos) D1 R1 & mini
//					- CPU freq 80MHz
//					- Set Debug port on Serial1
//          - Erease Flash: "All Flash contents"
//
// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2
// "PE0FKO JN13 10"
// SYMBOL: 3,1,2,2,2,2,0,2,3,2,2,0,3,3,3,0,0,0,1,0,0,3,0,1,1,3,3,2,0,0,0,2,2,0,1,0,0,3,2,3,2,0,0,2,2,0,3,0,1,1,2,2,1,3,0,3,0,0,2,3,1,0,1,0,0,0,0,3,1,2,1,0,3,0,3,2,1,0,0,1,0,2,1,0,3,3,0,0,2,1,1,2,3,2,1,2,0,2,1,0,0,0,0,2,1,0,2,3,2,0,1,3,1,2,3,1,2,2,1,3,2,3,0,0,0,1,1,1,2,2,2,2,2,3,0,1,2,0,3,3,0,0,0,2,2,0,2,3,1,2,3,0,1,3,2,0,0,3,3,2,2,0,
//
Using library ESP8266WiFi at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ESP8266WiFi
Using library DNSServer at version 1.1.1 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/DNSServer
Using library SPI at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/SPI
Using library Wire at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/Wire
Using library Adafruit_GFX_Library at version 1.11.9 in folder: /home/fred/Arduino/libraries/Adafruit_GFX_Library
Using library Adafruit_BusIO at version 1.15.0 in folder: /home/fred/Arduino/libraries/Adafruit_BusIO
Using library Adafruit_SSD1306_Wemos_Mini_OLED at version 1.1.2 in folder: /home/fred/Arduino/libraries/Adafruit_SSD1306_Wemos_Mini_OLED
Using library Etherkit_Si5351 at version 2.1.4 in folder: /home/fred/Arduino/libraries/Etherkit_Si5351
Using library Etherkit_JTEncode at version 1.3.1 in folder: /home/fred/Arduino/libraries/Etherkit_JTEncode
Using library OneWire at version 2.3.7 in folder: /home/fred/Arduino/libraries/OneWire
Using library DallasTemperature at version 3.9.0 in folder: /home/fred/Arduino/libraries/DallasTemperature
Using library ESP8266mDNS at version 1.2 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ESP8266mDNS
Using library ArduinoOTA at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ArduinoOTA
//*/

#define		VERSION		"V2.0"

#define	LOC_PE0FKO
//#define	LOC_PA_PE0FKO
//#define	LOC_PE0FKO_NR
//#define	LOC_LA_MOTHE_40m
//#define	LOC_LA_MOTHE_30m
//#define	LOC_LA_MOTHE_20m

#define		FEATURE_OTA
#define		FEATURE_mDNS
//#define		FEATURE_CARRIER
//#define		FEATURE_1H_FAST_TX
//#define		FEATURE_PRINT_TIMESLOT
//#define		FEATURE_PRINT_WSPR_SIMBOLS
#define     FEATURE_WIFIMULTI

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

#if defined LOC_PE0FKO
  #define	WSPR_TX_FREQ_0	WSPR_TX_FREQ_40m
  #define	WSPR_TX_FREQ_1	WSPR_TX_FREQ_20m
  #define   HAM_PREFIX      ""				// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"        // Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JO32cd"		// JO32CD 40OJ
  #define   HAM_POWER       11 //2 //10				// Power TX in dBm, 9dBm measure
//  #define	WIFI_SSID_01	"pe0fko_ziggo",	"NetwerkBeheer114"
#elif defined LOC_PA_PE0FKO
  #define	WSPR_TX_FREQ	WSPR_TX_FREQ_40m	// 40m   7.040000 -  7.040200
  #define   HAM_PREFIX      "PA/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JO32cd"		// JO32CD 40OJ
  #define   HAM_POWER       10				// Power TX in dBm
#elif defined LOC_LA_MOTHE_40m
  #define	WSPR_TX_FREQ	WSPR_TX_FREQ_40m	// 40m   7.040000 -  7.040200
  #define   HAM_PREFIX      "F/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
  #define   HAM_POWER       10				// Power TX in dBm
#elif defined LOC_LA_MOTHE_30m
  #define	WSPR_TX_FREQ	WSPR_TX_FREQ_30m	// 30m  10.140100 - 10.140300
  #define   HAM_PREFIX      "F/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
  #define   HAM_POWER       5				// Power TX in dBm
#elif defined LOC_LA_MOTHE_20m
  #define	WSPR_TX_FREQ	WSPR_TX_FREQ_20m	// 20m  14.097000 - 14.097200
  #define   HAM_PREFIX      "F/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
  #define   HAM_POWER       4				// Power TX in dBm

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

#define		MYTZ				TZ_Europe_Amsterdam	// TZ string for currect location
#define		WSPR_SLOTS_MAX		30			// 30 times 2min slots in a hour
#define		BUTTON_INPUT		D6			// GPIO 12, INPUT_PULLUP
#define		ONE_WIRE_BUS		D7			// GPIO 13

// *********************************************
#include <stdlib.h>
#include <Arduino.h>
#include <coredecls.h>
#include <time.h>
#include <TZ.h>
#include <sntp.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>         // Adafruit GFX Library               1.11.5
#include <Adafruit_SSD1306.h>     // Adafruit SSD1306 Wemos Mini OLED
#include <si5351.h>               // Etherkit
#include <JTEncode.h>             // Etherkit
#include <OneWire.h>
#include <DallasTemperature.h>
#ifdef FEATURE_mDNS
#include <ESP8266mDNS.h>
#endif
#ifdef FEATURE_OTA
#include <ArduinoOTA.h>
#endif
#ifdef FEATURE_WIFIMULTI
#include <ESP8266WiFiMulti.h>   // Include the Wi-Fi-Multi library
ESP8266WiFiMulti wifiMulti;
#endif
#include "WiFi_SSID.h"

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix the file Adafruit_SSD1306.h, enable the SSD1306_128_64!");
#endif

#ifdef DEBUG_ESP_PORT
// Put the strings in PROGMEM, slow but free some (constant) ram memory.
 #define PRINT(S)			{ DEBUG_ESP_PORT.print(S); }
 #define PRINT_P(S)			{ DEBUG_ESP_PORT.print(PSTR(S)); }
 #define PRINTF_P(F,...)	{ DEBUG_ESP_PORT.printf_P(PSTR(F), ##__VA_ARGS__); }
#else
 #define PRINT(S)			{ }
 #define PRINT_P(...)		{ }
 #define PRINTF_P(...)		{ }
#endif

boolean						connectioWasAlive = true;	// ESP8266WiFiMulti
Adafruit_SSD1306			display(-1);
Si5351						si5351;
JTEncode					wspr;

static 	WiFiEventHandler	wifiConnectHandler;			// WiFi connect event handler
static	WiFiEventHandler	wifiDisconnectHandler;		// WiFi disconnect event handler
static	String				HostName;
static	OneWire				oneWire(ONE_WIRE_BUS);
static	DallasTemperature	sensors(&oneWire);

static	uint8_t				wifi_status_previous	= WL_DISCONNECTED;

//static esp8266::polledTimeout::periodicMs showTimeNow(60000);  // Checkout

const   uint32_t		value_20ms_loop			= 20;   			// 20ms interval check time ntp sec
static  uint32_t		timer_20ms_loop;
const	float			value_wspr_bit_ms_incr	= 8192.0/12000.0*1000.0; // Delay value for WSPR, 682.66666 ms
static	float			value_wspr_bit_ms_float;
static	uint32_t		value_wspr_bit_ms;							// Delay value for WSPR
static	uint32_t		timer_wspr_bit_ms;
static	uint32_t		value_display_auto_off;						// Display on time, load per chip
static	uint32_t		timer_display_auto_off;
const	uint32_t		value_ntp_faild_reboot	= 3600 * 1000;		// 1 hour (ntp must be updated in this time)
static	uint32_t		timer_ntp_faild_reboot;
const	uint32_t		sntp_update_delay		= 30 * 60000;		// NTP update every 30min
volatile bool			ntp_time_sync;
//char					PASSWORD[32]			= "12345678";		// Will use the CPU ID for the password!
static	float			temperature_now			= 0.0;
static  uint8_t			switchStatusLast		= HIGH;				// last status hardware switch
static	enum { DISPLAY_OFF,	DISPLAY_ON } display_status;

static	uint8_t			wspr_symbols[WSPR_SYMBOL_COUNT];
static	uint32_t		wspr_symbol_index		= 0;
static	uint32_t		wspr_tx_counter        	= 0;

static	uint32_t		wspr_slot;									// Slot in the hour, 0..29
static	enum { WSPR_TX_NONE, WSPR_TX_TYPE_1, WSPR_TX_TYPE_2, WSPR_TX_TYPE_3 }
						wspr_slot_tx[WSPR_SLOTS_MAX];				// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
static	uint32_t		wspr_slot_band[WSPR_SLOTS_MAX];				// Band freqency, 0 .. 200 Hz
static	uint32_t		wspr_slot_freq[WSPR_SLOTS_MAX][3];			// TX frequency for every CLK output (0..2)

const	int32_t      	wspr_sym_freq[4] =
{	( 0.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	( 1.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	( 2.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	( 3.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
};

static	struct {	int 	ChipId;
					int 	FreqCorrection;
					int 	RandomSeed;
					int		DisplayAutoOff;
					String	Hostname;
					float	TempCorrection;			// Change at 18/08/2022
				} ESPChipInfo[] =
{	{ 0x7b06f7, 13126UL,  0x1502, 4*60000, "wsprtx-tst", -2.5 }	// Arduino shield
,	{ 0x62df37, 116000UL, 0x0257, 1*60000, "wsprtx", 0.0 }		// Breadboard
,	{ -1, 0Ul, 0X5555, 1*60000, "wspr-esp", 0.0 }				// Default
};

static		int		CHIP_FREQ_CORRECTION;
static		int		CHIP_RANDOM_SEED;
static		int		CHIP_DISPLAY_AUTO_OFF;;
static		String	CHIP_HOSTNAME;
static		float	CHIP_TEMP_CORRECTION;

void ssd1306_text(uint8_t delay_ms, const char* txt1, const char* txt2=NULL);

//
// Make a plan to TX in one of the 30 slots (2min inteval in a hour).
//
void make_slot_plan(bool setup)
{
    // Clean the old slot plan.
	wspr_slot_band[0] = random(4, 196);	// All TX in the hour on the same band
	for (int i = 0; i < WSPR_SLOTS_MAX; i++)
	{
		wspr_slot_tx  [i]		= WSPR_TX_NONE;
		wspr_slot_band[i]		= wspr_slot_band[0];
		wspr_slot_freq[i][0]	= 0;
		wspr_slot_freq[i][1]	= 0; //WSPR_TX_FREQ_0;
		wspr_slot_freq[i][2]	= 0; //WSPR_TX_FREQ_1;

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

#elif 1

	// Even slot 40m, odd slot 20m
	for (int i = 0; i < WSPR_SLOTS_MAX; i += 2)
	{
		wspr_slot_tx  [i+0]    = WSPR_TX_TYPE_2;
		wspr_slot_freq[i+0][0] = WSPR_TX_FREQ_40m;
		wspr_slot_tx  [i+1]    = WSPR_TX_TYPE_2;
		wspr_slot_freq[i+1][0] = WSPR_TX_FREQ_20m;
	}

#elif 0

	// Every even slot a TX until the first hour.
	for (int i = 0; i < WSPR_SLOTS_MAX; i += 1)
	{
		wspr_slot_tx  [i]    = WSPR_TX_TYPE_2;
// Kan niet alleen CLK0 op freq 0 zetten!!!!!!!!!!!!!
		wspr_slot_freq[i][0] = i % 3 == 0 ? WSPR_TX_FREQ_2m : 0;
		wspr_slot_freq[i][1] = WSPR_TX_FREQ_0;
		wspr_slot_freq[i][2] = WSPR_TX_FREQ_1;
	}

#elif 1
	{
		int   s0,s1,s2,s3,t;
		float tf;

		ReadTemperature();
//		sensors.requestTemperatures();
//		tf = sensors.getTempCByIndex(0) + CHIP_TEMP_CORRECTION;
//		PRINTF_P("Slot Temperature %.1fºC\n", tf);

		//Convert temperature to integer value
		// >-20 ... <50 ==> 0 ... 70 ==> *10 = 0 ... 700
		tf += 20.0;								//== Negative start offset
		if (tf < 0.0) tf = 0.0;
		if (tf > (70.0-0.1)) tf = 70.0-0.1;
		tf *= 10;								//== Decimal steps
		t = (int)(tf + 0.5);					// Rounding

		s1 =  0 + t / 100;	t %= 100;	// 0-6 :  0 .. 12 min
		s2 = 10 + t /  10;	t %= 10;	// 0-9 : 20 .. 38 min
		s3 = 20 + t /   1;				// 0-9 : 40 .. 58 min
		s0 = s1 + 1;					// After first digit tx

		PRINTF_P("TX Slots min: (%d), [%d, %d, %d]\n", s0 * 2, s1 * 2, s2 * 2, s3 * 2);

		wspr_slot_tx[s0]	= WSPR_TX_TYPE_3;	// TX locator 6
		wspr_slot_tx[s1]	= WSPR_TX_TYPE_2;	// Comp, no locator
		wspr_slot_tx[s2]	= WSPR_TX_TYPE_2;	// Comp, no locator
		wspr_slot_tx[s3]	= WSPR_TX_TYPE_2;	// Comp, no locator
	}
#endif

#ifdef FEATURE_PRINT_TIMESLOT
	PRINT_P("Time Slot:\n");
	for(uint8_t i=0; i < WSPR_SLOTS_MAX; ++i) {
		if (wspr_slot_tx[i] != WSPR_TX_NONE)
			PRINTF_P("\t%02d:wspr%d-%d+%d\n", i, wspr_slot_tx[i], wspr_slot_freq[i], wspr_slot_band[i]);
	}
	PRINT_P("\n");
#endif
}

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 ()
{
    return sntp_update_delay;
}

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup()
{
//	WiFi.mode(WIFI_OFF);				// Start WiFi later
	// Always check the near AP with WiFiMulti
	WiFi.persistent(false);				// Don't save WiFi configuration in flash
//	WiFi.persistent(true);				// Save WiFi configuration in flash
// true: WiFi mode configuration will be retained through power cycle. (Default)
// false: WiFi mode configuration will not be retained through power cycle.
//	WiFi.setmode(WIFI_OFF, false);				// Start WiFi later
//	WiFii.mode(WIFI_OFF, false);				// Start WiFi later
//	yield();

	// Find the ESP chip-id specific data.
	int i = 0, chipid = ESP.getChipId();
	while(true)
	{
		CHIP_FREQ_CORRECTION 	= ESPChipInfo[i].FreqCorrection;
		CHIP_RANDOM_SEED     	= ESPChipInfo[i].RandomSeed;
		CHIP_DISPLAY_AUTO_OFF	= ESPChipInfo[i].DisplayAutoOff;
		CHIP_HOSTNAME			= ESPChipInfo[i].Hostname;
		CHIP_TEMP_CORRECTION	= ESPChipInfo[i].TempCorrection;
		if (ESPChipInfo[i].ChipId == chipid || ESPChipInfo[i].ChipId == -1) break;
		i++;
	}
	randomSeed(CHIP_RANDOM_SEED);
	value_display_auto_off = CHIP_DISPLAY_AUTO_OFF;
	HostName = CHIP_HOSTNAME;

	Serial.begin(115200);				// 115200
	Serial.setTimeout(2000);
	while(!Serial) yield();
#ifdef DEBUG_ESP_PORT
//	Serial.setDebugOutput(true);
#endif
	delay(300);

	PRINT_P("\n=== PE0FKO, TX WSPR temperature coded\n");
	PRINTF_P ("=== Version: " VERSION ", Build at: %s %s\n", __TIME__, __DATE__);
	PRINTF_P ("=== Config: frequency %.6fMHz/%.6fMHz - " HAM_PREFIX HAM_CALL HAM_SUFFIX " - " HAM_LOCATOR " - %ddBm\n", 
			WSPR_TX_FREQ_0/1000000.0, WSPR_TX_FREQ_1/1000000.0, HAM_POWER);

	PRINTF_P("SSD1306: %dx%d addr:0x%02x\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH, 0x3C);
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//	display.setRotation(2);					// Display upside down...

	pinMode(BUTTON_INPUT, INPUT_PULLUP);	// Button for display on/off
	timer_display_auto_off = millis();		// Start the display ON timer
	display_status  = DISPLAY_ON;

	pinMode(LED_BUILTIN, OUTPUT);			// BuildIn LED

    // Set Hostname.
	WiFi.hostname(HostName);
	ssd1306_text(1000, "Hostname", HostName.c_str());

	WiFi.mode(WIFI_STA);				// Set WiFi to station mode
//	WiFi.setAutoReconnect(true);		// Keep WiFi connected
	// Register FiFi event handlers
	wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
	wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

#ifdef FEATURE_WIFIMULTI
	// Try to startup the WiFi Multi connection with the strongest AP found.
	for(i = 0; i < WifiApListNumber; i++)
		wifiMulti.addAP(WifiApList[i].ssid, WifiApList[i].passwd);
#else
	WiFi.begin(WIFI_SSID_01);
#endif

#ifdef FEATURE_mDNS
	init_mdns();						// Init the broadcast DNS server (.local)
#endif

#ifdef FEATURE_OTA
	// Start OTA server.
	ArduinoOTA.setHostname((const char *)HostName.c_str());
	ArduinoOTA.onStart([]() { ssd1306_text(200, "Running", "OTA update"); });
	ArduinoOTA.begin();
#endif

	init_sntp_now();					// Init the sNTP client to get the real time

	sensors.begin();					// Init the onewire for the DS18B20 temp sensor
	ReadTemperature();					// Read the Dallas temperature one-wire sensor

	init_si5351();						// Init the frequency generator SI5351

	make_slot_plan(true);				// The first hour slot plan definition

	time_t now = time(nullptr);			// get UNIX timestamp
	PRINT(ctime(&now));					// convert timestamp and display

	pinMode(LED_BUILTIN, OUTPUT);		// LED on the ESP
	digitalWrite(LED_BUILTIN, HIGH);	// High is off!

	timer_20ms_loop = millis();

#ifdef FEATURE_CARRIER

	si5351.set_freq( SI5351_FREQ_MULT * wspr_slot_freq[0][0], SI5351_CLK0 );
	wspr_tx_enable(SI5351_CLK0);
	si5351.set_freq( SI5351_FREQ_MULT * wspr_slot_freq[0][1], SI5351_CLK1 );
	wspr_tx_enable(SI5351_CLK1);
	si5351.set_freq( SI5351_FREQ_MULT * wspr_slot_freq[0][2], SI5351_CLK2 );
	wspr_tx_enable(SI5351_CLK2);

	PRINTF_P("CW Carrier on: %fMHz\n", (float)(WSPR_TX_FREQ + 100)/1e6);
#endif

	ssd1306_text(200, "Start", "looping");
	PRINT_P("=== Start looping...\n");
}



void onWifiConnect(const WiFiEventStationModeGotIP& ipInfo)
{
	PRINTF_P("WiFi connected: IP:%s/%s GW:%s\n",
		ipInfo.ip.toString().c_str(),
		ipInfo.mask.toString().c_str(),
		ipInfo.gw.toString().c_str()
		);
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo)
{
//	Crash by multiple display update!
//	ssd1306_text(200, "WiFi", "Disconnect");

	PRINTF_P("WiFi disconnected from SSID: %s, Reason: %d\n",
  		disconnectInfo.ssid.c_str(),
		disconnectInfo.reason
		);
}

void ReadTemperature()
{
	sensors.requestTemperatures();
	temperature_now = sensors.getTempCByIndex(0) + CHIP_TEMP_CORRECTION;

	PRINTF_P("Sensor DS18B20 temperature %.1fºC\n", temperature_now);
}

#ifdef FEATURE_mDNS
static void init_mdns()
{
	ssd1306_text(200, "mDNS", "Setup");
	if (MDNS.begin(HostName.c_str()))
		MDNS.addService("http", "tcp", 80);
	else
		PRINT_P("mDNS ERROR!\n");
}
#endif

void init_sntp_now()
{
	ntp_time_sync = false;
//	sntp_stop();
	settimeofday_cb(time_is_set);			// Call-back function
	// Set the TZ and NTP servers to check.
	configTime(MYTZ, "time.google.com","nl.pool.ntp.org");
	sntp_init();

	timer_ntp_faild_reboot = millis();
}

// callback routine - arrive here whenever a successful NTP update has occurred
void time_is_set (bool from_sntp)
{
	time_t now = time(nullptr);          // get UNIX timestamp
	PRINT_P("NTP update done at: ");
	PRINT(ctime(&now));
	ntp_time_sync = true;
//	timer_ntp_faild_reboot = millis();
}


//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------
void loop()
{
#if 0
	// Send the WSPR bits into the air if active TX!
	// When started it will wait for the bit time and start a next bit.
	if (wspr_symbol_index != 0 && (millis() - timer_wspr_bit_ms) >= value_wspr_bit_ms)
	{
		value_wspr_bit_ms_float += value_wspr_bit_ms_incr;	// Increment the float ms counter (exact counting)
		value_wspr_bit_ms = value_wspr_bit_ms_float + 0.5;	//   copy and round to uint32_t

		wspr_tx_bit();										// Ok, transmit the net tone bit
	}
#else
	// Send the WSPR bits into the air if active TX!
	// When started it will wait for the bit time and start a next bit.
	if (wspr_symbol_index != 0)
	{
		int32_t ms = (millis() - timer_wspr_bit_ms) - value_wspr_bit_ms;
		if (ms >= 0)
		{
			wspr_tx_bit();				// Ok, transmit the next tone bit

			value_wspr_bit_ms_float += value_wspr_bit_ms_incr;	// Increment the float ms counter (exact counting)
			value_wspr_bit_ms = value_wspr_bit_ms_float + 0.5;	//   copy and round to uint32_t
	
			if (ms > 3)
				PRINTF_P("Overrun bit %3d: %3d ms\n", wspr_symbol_index, ms);
		}
	}
#endif

	#ifdef FEATURE_OTA
	ArduinoOTA.handle();
	#endif

	#ifdef FEATURE_mDNS
	MDNS.update();
	#endif

	#ifdef FEATURE_WIFIMULTI
	wifiMulti.run(4000);
	#endif

	if (display_status == DISPLAY_ON && ((millis() - timer_display_auto_off) >= value_display_auto_off))
			ssd1306_display_off();

	// Show WiFi status change on the oled display.
	if (wifi_status_previous != WiFi.status())
	{
		wifi_status_previous = WiFi.status();

		switch(wifi_status_previous)
		{
			case WL_CONNECTED:		ssd1306_wifi_page();						break;
			case WL_CONNECT_FAILED:	ssd1306_text(200, "WiFi", "FAILED");		break;
			case WL_WRONG_PASSWORD:	ssd1306_text(200, "WiFi", "Error PASSWD");	break;
			case WL_DISCONNECTED:	ssd1306_text(200, "WiFi", "Disconnected");	break;
			case WL_NO_SSID_AVAIL:	ssd1306_text(200, "WiFi", "No SSID avail");	break;
			case WL_IDLE_STATUS:	ssd1306_text(200, "WiFi", "Idle status");	break;
			default:				ssd1306_text(200, "WiFi", "ERROR");
									PRINTF_P("WiFi status unknow = %d\n", wifi_status_previous);
									break;
		}
	}

	if (wifi_status_previous != WL_CONNECTED)
		return;		// Return the loop!!

	// Wait for the NTP time service is known!
	if (!ntp_time_sync)
	{
		if ((millis() - timer_ntp_faild_reboot) >= value_ntp_faild_reboot)
		{
			ssd1306_text(2000, "REBOOT", "NTP sync...");
			PRINT_P("REBOOT: No NTP time received.\n");
			Serial.flush();
			ESP.restart();
		}

// Maak langere loops met timer

		ssd1306_text(500, "Waiting", "NTP sync");
		return;		// Return the loop!!
	}

	// ----------------------------------------
	// Now we have WiFi running,
	//  and a valid time from the NTP servers!

#if 0
	// No need for WiFi until the next NTP request.
	PRINT_P("\nSwitch WiFi off!!!!\n");
	WiFi.setAutoReconnect(false);
	WiFi.mode(WIFI_OFF);
	PRINTF_P("WiFi.status=%d\n", WiFi.status());
	PRINT_P("WiFi is switched off!!!!\n\n");
return;

#endif

	//++ 50Hz code lus
	// Used for slower processing, timing from the cpu xtal
	if ((millis() - timer_20ms_loop) >= value_20ms_loop)    		// Every 20ms...
	{
		timer_20ms_loop += value_20ms_loop;

		time_t		now			= time(nullptr);
		uint16_t	sec			= now % 60;
static 	int8_t 		last_sec	= -1;

		// Check if button is pressed to lightup the display
		int switchStatus = digitalRead(BUTTON_INPUT);				// read status of switch
		if (switchStatus != switchStatusLast)						// if status of button has changed
		{
			  switchStatusLast = switchStatus;
			  if (switchStatus == LOW)
			  {
				display_status = display_status == DISPLAY_ON ? DISPLAY_OFF : DISPLAY_ON;

				if (display_status == DISPLAY_ON)
				{
					timer_display_auto_off = millis();  	// Start the display ON timer
					digitalWrite(LED_BUILTIN, HIGH);		// Switch the ESP LED off
				}
				else
				{
					ssd1306_display_off();
				}
				last_sec = -1;								// Fast update of the display
				PRINTF_P("Button pressed, display_status=%d\n", display_status);
			  }
		}

		// Blink the ESP LED every 4s if display is off
		if (display_status == DISPLAY_OFF)
		{
			static uint8_t led_pwm;
			led_pwm %= 200;
			digitalWrite(LED_BUILTIN, led_pwm++ == 0 ? LOW : HIGH);
			delay(3);
			digitalWrite(LED_BUILTIN, HIGH);
		}


		//++ Updates for every second
		//
		if (sec != last_sec)
		{
			last_sec = sec;

			uint32_t s = millis();

			//
			//++ At every 2 minute interval start a WSPR message, if slot is richt.
			//
			if (now % 120 == 0)										// First second of the 2 minute interval clock
			{
				char const* call = NULL;
				wspr_slot = (now / 120) % WSPR_SLOTS_MAX;			// Slot in the hour, 0..29

				if (wspr_slot == 0)
					make_slot_plan(false);

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_1)		// Type 1 message: CALL, LOC4, dBm
					call = HAM_CALL;

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_2)		// Type 2 message: pre/CALL/suff, dBm
					call = HAM_PREFIX HAM_CALL HAM_SUFFIX;

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_3)		// Type 3 message: hash <pre/CALL/suff>, LOC6, dBm
					call = "<" HAM_PREFIX HAM_CALL HAM_SUFFIX ">";

				if (call != NULL)
					wspr_tx_init(call);
			}

			//
			//++ Set the random seed ones every day.
			//   Posible track of (semi) random numbers!
			if (now % 3600*24 == 114)
			{
				PRINTF_P("Set the const ramdom seed number 0x%08x\n", CHIP_RANDOM_SEED);
				randomSeed(CHIP_RANDOM_SEED);
			}

			//
			//++ At avery 8 minute interval, at second 16, print the temperature
			// No wspr TX at > 51+60sec 
//			if (now % 120*4 == 113)
			if (now % 120 == 113)
			{
				ReadTemperature();
				PRINT_P("Time now: ");	PRINT(ctime(&now));
			}

			if (display_status == DISPLAY_ON)
				ssd1306_main_window(now);

			s = millis() - s;
			if (s > 150)	// 147ms
				PRINTF_P("Long-time: %d\n", s);

		}   	// One second lus
	}   		// 50Hz code lus
}


static void init_si5351()
{
	ssd1306_text(200, "Si5351", "Setup");

	if ( si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, CHIP_FREQ_CORRECTION) )
	{	// Disable the clock initially...
		wspr_tx_disable(SI5351_CLK0);
		wspr_tx_disable(SI5351_CLK1);
		wspr_tx_disable(SI5351_CLK2);
		ssd1306_text(200, "Si5351", "OK");
	}
	else
		ssd1306_text(200, "Si5351", "Not found");
}

void wspr_tx_init(const char* call)
{
	PRINTF_P("WSPR TX with Call: %s, Loc:%s, Power:%ddBm\n", call, HAM_LOCATOR, HAM_POWER);

	wspr.wspr_encode(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);
#ifdef FEATURE_PRINT_WSPR_SIMBOLS
	print_wspr_symbols(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);
#endif
	timer_wspr_bit_ms = millis();
	value_wspr_bit_ms_float = value_wspr_bit_ms_incr;	// First ms value
	value_wspr_bit_ms = value_wspr_bit_ms_float + 0.5;	//   copy and round to uint32_t

	wspr_tx_bit();
}

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2
void print_wspr_symbols(const char* call, const char* loc, uint8_t power, uint8_t symbols[])
{
	PRINTF_P("%s %s %ddBm:\n  ", call, loc, power);
	for (uint8_t i = 0; i < WSPR_SYMBOL_COUNT; )
	{
		PRINTF_P("%d,", symbols[i++]);
		if (i % 41 == 0) PRINT_P("\n  ");
	}
	PRINT_P("\n");
}
#endif

void wspr_tx_bit()
{
	if (wspr_symbol_index != WSPR_SYMBOL_COUNT)
	{
		if (wspr_symbol_index == 0)   // On first bit enable the tx output.
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

		PRINTF_P("TX WSPR %d Ended.\n", wspr_tx_counter);
	}
}

void wspr_tx_freq(si5351_clock clk)
{
	if (wspr_slot_freq[wspr_slot][clk] != 0)
	{
		uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[wspr_slot][clk] + wspr_slot_band[wspr_slot]);
		if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) )
			PRINTF_P("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
	}
}

void wspr_tx_enable(si5351_clock clk)
{
	if (wspr_slot_freq[wspr_slot][clk] != 0)
	{
		PRINTF_P("TX WSPR start %d: slot %d, freq %fMHz + %dHz\n", 
				clk, wspr_slot, 
				wspr_slot_freq[wspr_slot][clk] / 1000000.0, 
				wspr_slot_band[wspr_slot]);
		si5351.drive_strength(clk, SI5351_DRIVE_8MA); // 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
		si5351.set_clock_pwr(clk, 1);
		si5351.output_enable(clk, 1);
	}
}

void wspr_tx_disable(si5351_clock clk)
{
	si5351.set_clock_pwr(clk, 0);
	si5351.output_enable(clk, 0);
}


void ssd1306_main_window(time_t now)
{
	char		buffer[30];
	struct tm*	timeinfo;
	int16_t   	x,y;
	uint16_t  	w,h;
	int			ns = wspr_slot;	// next slot

	ssd1306_background();

	timeinfo = localtime (&now);
	strftime (buffer ,sizeof buffer ,"%H:%M:%S", timeinfo);
	display.setTextSize(2);
	w = getFontStringWidth(buffer);
	display.setCursor((display.width() - w) / 2 , 12);
	display.print(buffer);

	strftime (buffer ,sizeof buffer ,"- %d/%m/%Y -", timeinfo);
	display.setTextSize(1);
	w = getFontStringWidth(buffer);
	display.setCursor((display.width() - w) / 2 , 32);
	display.print(buffer);

	sprintf(buffer, "%s/%s/%ddBm", HAM_CALL, HAM_LOCATOR, HAM_POWER);
	w = getFontStringWidth(buffer);
	display.setCursor((display.width()- w)/2, display.height()-10);
	display.print(buffer);

	if (wspr_symbol_index != 0)
	{
		uint16_t B,E;
		B = 4;
		E = B + now % 120;
		display.drawLine(B, display.height()-14, E, display.height()-14, WHITE);
		display.drawLine(B, display.height()-16, E, display.height()-16, WHITE);

		// At TX invert the display collors
		display.invertDisplay(true);
	}
	else
	{
		// Calculate the next tx time and slot
		uint16_t w = 120 - (now % 120);
		while (wspr_slot_tx[ns++] == WSPR_TX_NONE) {
			if (ns >= WSPR_SLOTS_MAX) ns = 0;
			w += 120;
		}
		if (w > 3600) return;		// Bailout if no next tx

		// Display time next tx
		sprintf(buffer, "TX %02d:%02d", w/60, w%60);
		display.setCursor(14, display.height()-20);
		display.print(buffer);

		// The actual temperature used
		sprintf(buffer, "%.1f", temperature_now);
		w = getFontStringWidth(buffer);
		display.setCursor(display.width() - 14 - w, display.height()-20);
		display.print(buffer);

		// Non TX normal display
		display.invertDisplay(false);
	}

	// Display the used bands
	sprintf(buffer, "%s/%s/%s", 
			wspr_slot_freq[ns][0] == 0 ? "x" : String(300/(wspr_slot_freq[ns][0]/1000000)).c_str(),
			wspr_slot_freq[ns][1] == 0 ? "x" : String(300/(wspr_slot_freq[ns][1]/1000000)).c_str(),
			wspr_slot_freq[ns][2] == 0 ? "x" : String(300/(wspr_slot_freq[ns][2]/1000000)).c_str()
			);

	display.getTextBounds(buffer, 0, 0, &x, &y, &w, &h);
	x = display.width() * 1 / 4;
	x -= w / 2; if (x < 10) x = 10;
	display.fillRect(x, y, w, h, BLACK);
	display.setCursor(x, y);
	display.print(buffer);

	display.display();
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

void ssd1306_display_off()
{
	PRINTF_P("Display auto Off at %d sec.\n", value_display_auto_off/1000);
	display.clearDisplay();
	display.invertDisplay(false);
	display.display();
	display_status = DISPLAY_OFF;
}

uint16_t getFontStringWidth(const String& str)
{
	int16_t   x, y;
	uint16_t  w, h;
	display.getTextBounds(str, 0, 0, &x, &y, &w, &h);
	return w;
}

void ssd1306_text(uint8_t delay_ms, const char* txt1, const char* txt2)
{
	ssd1306_background();

	display.setTextSize(1);
	if (txt1 != NULL)
	{
		uint16_t  w = getFontStringWidth(txt1);
		display.setCursor((display.width() - w) / 2 , 16);
		display.print(txt1);
	}
	if (txt2 != NULL)
	{
		uint16_t  w = getFontStringWidth(txt2);
		display.setCursor((display.width() - w) / 2 , 16+16);
		display.print(txt2);
	}

	if (WiFi.SSID().length() > 0)
	{
		String ssid("SSID:"); ssid += WiFi.SSID();
		uint16_t  w = getFontStringWidth(ssid);
		display.setCursor((display.width() - w) / 2 , 52);
		display.print(ssid);
	}

	display.display();

//	PRINTF_P("ssd1306_text: %s / %s\n", txt1, txt2);
	PRINTF_P("ssd1306_text[%d]: ", delay_ms);
	if (txt1 != NULL)	PRINT(txt1);
	if (txt2 != NULL) {	PRINT_P(" / "); PRINT(txt2); }
	PRINT_P("\n");

	timer_display_auto_off = millis();					// Start the display ON timer
	display_status = DISPLAY_ON;

	if (delay_ms >= 0)
		delay(delay_ms);
}

void ssd1306_wifi_page()
{
	PRINTF_P("Connected to %s (IP:%s/%s, GW:%s, RSSI %d)\n"
		, WiFi.SSID().c_str()
		, WiFi.localIP().toString().c_str()				// Send the IP address of the ESP8266 to the computer
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

	display.display();

	timer_display_auto_off = millis();					// Start the display ON timer
	display_status = DISPLAY_ON;

	delay(2000);
}
