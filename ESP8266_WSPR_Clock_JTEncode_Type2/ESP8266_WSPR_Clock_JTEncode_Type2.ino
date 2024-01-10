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
//	Using library EEPROM at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/EEPROM 
//	Using library ESP8266WiFi at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ESP8266WiFi 
//	Using library DNSServer at version 1.1.1 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/DNSServer 
//	Using library WiFiManager at version 2.0.15-rc.1 in folder: /home/fred/Arduino/libraries/WiFiManager 
//	Using library ESP8266WebServer at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ESP8266WebServer 
//	Using library SPI at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/SPI 
//	Using library Wire at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/Wire 
//	Using library Adafruit_GFX_Library at version 1.11.5 in folder: /home/fred/Arduino/libraries/Adafruit_GFX_Library 
//	Using library Adafruit_BusIO at version 1.14.1 in folder: /home/fred/Arduino/libraries/Adafruit_BusIO 
//	Using library Adafruit_SSD1306_Wemos_Mini_OLED at version 1.1.2 in folder: /home/fred/Arduino/libraries/Adafruit_SSD1306_Wemos_Mini_OLED 
//	Using library Etherkit_Si5351 at version 2.1.4 in folder: /home/fred/Arduino/libraries/Etherkit_Si5351 
//	Using library Etherkit_JTEncode at version 1.3.1 in folder: /home/fred/Arduino/libraries/Etherkit_JTEncode 
//	Using library OneWire at version 2.3.7 in folder: /home/fred/Arduino/libraries/OneWire 
//	Using library DallasTemperature at version 3.9.0 in folder: /home/fred/Arduino/libraries/DallasTemperature 
//	Using library ESP8266mDNS at version 1.2 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ESP8266mDNS 
//	Using library ArduinoOTA at version 1.0 in folder: /home/fred/.arduino15/packages/esp8266/hardware/esp8266/3.1.2/libraries/ArduinoOTA 
/*/


#define	LOC_PE0FKO
//#define	LOC_PA_PE0FKO
//#define	LOC_PE0FKO_NR
//#define	LOC_LA_MOTHE_40m
//#define	LOC_LA_MOTHE_30m
//#define	LOC_LA_MOTHE_20m

//#define		FEATURE_OTA
#define		FEATURE_mDNS
//#define		FEATURE_CARRIER
//#define		FEATURE_1H_FAST_TX
//#define		FEATURE_PRINT_TIMESLOT

#define	  EEPROM_VERSION  1
#define	  EEPROM_INDEX	  0

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

// Update JTEncode.cpp Line 1000

#if defined LOC_PE0FKO
  #define	WSPR_TX_FREQ	7040000UL		// 40m   7.040000 -  7.040200
  #define   HAM_PREFIX      ""				// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"        // Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JO32cd"		// JO32CD 40OJ
  #define   HAM_POWER       8				// Power TX in dBm
//  #define	WIFI_SSID_01	"pe0fko_ziggo",	"NetwerkBeheer114"
#elif defined LOC_PA_PE0FKO
  #define	WSPR_TX_FREQ	7040000UL		// 40m   7.040000 -  7.040200
  #define   HAM_PREFIX      "PA/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JO32cd"		// JO32CD 40OJ
  #define   HAM_POWER       8				// Power TX in dBm
#elif defined LOC_LA_MOTHE_40m
  #define	WSPR_TX_FREQ	7040000UL		// 40m   7.040000 -  7.040200
  #define   HAM_PREFIX      "F/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
  #define   HAM_POWER       8				// Power TX in dBm
#elif defined LOC_LA_MOTHE_30m
  #define	WSPR_TX_FREQ	10140100UL		// 30m  10.140100 - 10.140300
  #define   HAM_PREFIX      "F/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
  #define   HAM_POWER       5				// Power TX in dBm
#elif defined LOC_LA_MOTHE_20m
  #define	WSPR_TX_FREQ	14097000UL		// 20m  14.097000 - 14.097200
  #define   HAM_PREFIX      "F/"			// Prefix of the ham call
  #define   HAM_CALL        "PE0FKO"		// Ham radio call sign
  #define   HAM_SUFFIX      ""				// Suffix of the ham call
  #define   HAM_LOCATOR     "JN13IW"		// JN13IW 08UG
  #define   HAM_POWER       4				// Power TX in dBm
#else
  #error    "Specify the Location..."
#endif

#ifndef WSPR_TX_FREQ
//#define	WSPR_TX_FREQ		1838000UL	// 160m  1.838000 -  1.838200
//#define	WSPR_TX_FREQ		3570000UL	// 80m   3.570000 -  3.570200
//#define	WSPR_TX_FREQ		5288600UL	// 60m   5.288600 -  5.288800
#define	WSPR_TX_FREQ			7040000UL	// 40m   7.040000 -  7.040200
//#define	WSPR_TX_FREQ		10140100UL	// 30m  10.140100 - 10.140300
//#define	WSPR_TX_FREQ		14097000UL	// 20m  14.097000 - 14.097200
//#define	WSPR_TX_FREQ		18106000UL	// 17m  18.106000 - 18.106200
//#define	WSPR_TX_FREQ		21096000UL	// 15m  21.096000 - 21.096200
//#define	WSPR_TX_FREQ		24926000UL	// 12m  24.926000 - 24.926200
//#define	WSPR_TX_FREQ		28126000UL	// 10m  28.126000 - 28.126200
//#define	WSPR_TX_FREQ		50294400UL	// 6m   50.294400 - 50.294600
//#define	WSPR_TX_FREQ		144489900UL	// 2m  144.489900 - 144.490100
#endif

#ifndef WIFI_SSID_01
#define		WIFI_SSID_01		"pe0fko_guest",	"Welkom-114"
#define		WIFI_SSID_02		"pe0fko_ziggo",	"NetwerkBeheer114"
//#define		WIFI_SSID_03		"pe0fko-4g",	"NETWERKBEHEER"
#define		WIFI_SSID_03		"pe0fko_deco",	"NetwerkBeheer114"
#endif

#define		SI5351_FREQ_CORRECTION_01   (13126UL)
#define		SI5351_FREQ_CORRECTION_02   (116000UL)

#define		HOSTNAME          	"WsprTx"
#define		TEMP_CORRECTION   	0.0			// Change at 18/08/2022
#define		WSPR_DELAY        	683			// Delay value for WSPR
#define		WSPR_SLOTS_MAX		30			// 30 times 2min slots in a hour
#define		BUTTON_INPUT		D6			// GPIO 12
#define		ONE_WIRE_BUS		D7			// GPIO 13

//
// *********************************************
//
#include <stdlib.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <DNSServer.h>
//#include <WiFiManager.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>         // Adafruit GFX Library               1.11.5
#include <Adafruit_SSD1306.h>     // Adafruit SSD1306 Wemos Mini OLED   
#include <si5351.h>               // Etherkit
#include <JTEncode.h>             // Etherkit
#include "coredecls.h"
#include "sntp.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#ifdef FEATURE_mDNS
#include <ESP8266mDNS.h>
#endif
#ifdef FEATURE_OTA
#include <ArduinoOTA.h>
#endif

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix the file Adafruit_SSD1306.h, enable the SSD1306_128_64!");
#endif

#ifdef DEBUG_ESP_PORT
#define PRINTF(...)		{ DEBUG_ESP_PORT.printf(__VA_ARGS__); }
#define PRINTVAR(...)	{ DEBUG_ESP_PORT.printf(#__VA_ARGS__"="); DEBUG_ESP_PORT.print (__VA_ARGS__); DEBUG_ESP_PORT.print("\n"); }
#else
#error "WARNING: Set Tools/Debug port:Serial to see some output"
#define PRINTF(...)		{ }
#define PRINTVAR(...)	{ }
#endif


ESP8266WiFiMulti		wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
boolean					connectioWasAlive = true;		// ESP8266WiFiMulti
//WiFiManager				wifiManager;
Adafruit_SSD1306		display(-1);
Si5351					si5351;
JTEncode				wspr;

OneWire					oneWire(ONE_WIRE_BUS);
DallasTemperature		sensors(&oneWire);

// Variables and constants for timeouts
const   uint32_t		value_20ms_loop			= 20;   // 20ms
static  uint32_t		timer_20ms_loop;
const   uint32_t		value_wspr_bit_ms		= WSPR_DELAY;
static  uint32_t		timer_wspr_bit_ms;
const   uint32_t		value_display_auto_off	= (1 * 60UL * 1000UL);		// 1 min
static  uint32_t		timer_display_auto_off;
const   uint32_t		value_ntp_faild_reboot	= (5 * 60UL * 1000UL);		// 5 min
static  uint32_t		timer_ntp_faild_reboot;
volatile bool			ntp_time_sync;
static	bool			wifi_connected_msg		= false;

char					PASSWORD[32]			= "12345678";	// Will use the CPU ID for the password!
float					temperature_now			= 0.0;

uint8_t					wspr_symbols[WSPR_SYMBOL_COUNT];
uint8_t					wspr_symbol_index		= 0;
uint16_t				wspr_tx_counter        	= 0;

enum {        			WSPR_TX_NONE, WSPR_TX_TYPE_1, WSPR_TX_TYPE_2, WSPR_TX_TYPE_3 };
uint8_t					wspr_slot_tx[WSPR_SLOTS_MAX];     // 0=None, 1=CALL, 2=F/CALL, 3=<F/CALL>
uint8_t					wspr_slot_band[WSPR_SLOTS_MAX];
uint64_t				wspr_frequency;

#define TONE_SPACING(N)	((uint16_t)(12000.0/8192.0 * N * SI5351_FREQ_MULT + 0.5))
const int16_t      		wspr_sym_freq[4] = { TONE_SPACING(0), TONE_SPACING(1), TONE_SPACING(2), TONE_SPACING(3) };

static  uint8_t			switchStatusLast		= HIGH;  // last status hardware switch
enum {		DISPLAY_OFF,	DISPLAY_ON,	DISPLAY_NOTHING	};
static  uint8_t			display_switch_status	= DISPLAY_ON;

void ssd1306_text(uint8_t delay_ms, const char* txt1, const char* txt2=NULL);

//
// Make a plan to TX in one of the 30 slots (2min inteval in a hour).
//
void make_slot_plan(bool setup)
{
    // Clean the old slot plan.
    memset(wspr_slot_tx, WSPR_TX_NONE, WSPR_SLOTS_MAX);
	memset(wspr_slot_band, random(10, 190), WSPR_SLOTS_MAX);

#ifdef FEATURE_1H_FAST_TX
	if (setup)
	{
		// Every odd slot a TX until the first hour.
		for (int i = 0; i < WSPR_SLOTS_MAX; i += 6)
		{
			wspr_slot_tx[i+0] = WSPR_TX_TYPE_1;	// 0 - Min:  0, 12, 24, 36, 48
//			wspr_slot_tx[i+1] = WSPR_TX_NONE;	// 1 - Min:  2, 14, 26, 38, 50
			wspr_slot_tx[i+2] = WSPR_TX_TYPE_2;	// 2 - Min:  4, 16, 28, 40, 52
			wspr_slot_tx[i+3] = WSPR_TX_TYPE_3;	// 3 - Min:  6, 18, 30, 42, 54
//			wspr_slot_tx[i+4] = WSPR_TX_NONE;	// 4 - Min:  8, 20, 32, 44, 56
//			wspr_slot_tx[i+5] = WSPR_TX_NONE;	// 5 - Min: 10, 22, 34, 46, 58
		}
	}
	else
#endif
	{
		int   s0,s1,s2,s3,t;
		float tf;

		sensors.requestTemperatures(); 
		tf = sensors.getTempCByIndex(0) + TEMP_CORRECTION;
		PRINTF("Slot Temperature %.1fºC\n", tf);

		//Convert temperature to integer value
		// >-20 ... <50 ==> 0 ... 70 ==> *10 = 0 ... 700
		tf += 20.0;								//== Negative start offset
		if (tf < 0.0) tf = 0.0;
		if (tf > (70.0-0.1)) tf = 70.0-0.1;
		tf *= 10;								//== Decimal steps
		t = (int)(tf + 0.5);					// Rounding
//		PRINTF("Code message temp: %03d\n", t);

// 	4	6	8	10	12	14	16	18
// 	-20	-10	0	10	20	30	40	50
//	-11	-1	9	19	29	39	49	59

		s1 = 1 + t / 100;       // 0-6 :  4 .. 16 min
		t %= 100;
		s2 = 10 + t / 10;       // 0-9 : 20 .. 38 min
		t %= 10;
		s3 = 20 + t;			// 0-9 : 40 .. 58 min
		s0 = s1 + 1;			// After first digit tx

		PRINTF("TX Slots: %d, %d, %d, %d\n", s0, s1, s2, s3);
#if 0
		wspr_slot_tx[s0]   = WSPR_TX_TYPE_1;
		wspr_slot_tx[s1]   = WSPR_TX_TYPE_1;
		wspr_slot_tx[s2]   = WSPR_TX_TYPE_1;
#else
		wspr_slot_tx[s0]	= WSPR_TX_TYPE_3;	// TX locator 6
		wspr_slot_tx[s1]	= WSPR_TX_TYPE_2;	// Comp, no locator
		wspr_slot_tx[s2]	= WSPR_TX_TYPE_2;	// Comp, no locator
		wspr_slot_tx[s3]	= WSPR_TX_TYPE_2;	// Comp, no locator
#endif
	}

#ifdef FEATURE_PRINT_TIMESLOT
	PRINTF("Time Slot: ");
	for(uint8_t i=0; i < WSPR_SLOTS_MAX; ++i) {
		if (wspr_slot_tx[i] != WSPR_TX_NONE)
			PRINTF("%d:%ds-%03db, ", i, wspr_slot_tx[i], wspr_slot_band[i]); 
	}
	PRINTF("\n");
#endif
}

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....    
//---------------------------------------------------------------------------------

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void setup() 
{
	randomSeed(0x1502);

	Serial.begin(115200);				// 115200
	Serial.setTimeout(2000);
	while(!Serial) yield();
	delay(500);

#ifdef DEBUG_ESP_PORT
	Serial.setDebugOutput(true);
#endif

	PRINTF("DEBUG SSD1306: %d x %d\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH);
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//	display.setRotation(2);				// Display upside down...

	pinMode(BUTTON_INPUT, INPUT);		// Button for display on/off
	timer_display_auto_off = millis();	// Start the display ON timer

	pinMode(LED_BUILTIN, OUTPUT);		// BuildIn LED

	PRINTF("\n\nPE0FKO, WSPR Clock, Build: %s %s\n", __TIME__, __DATE__);
	PRINTF("Config: frequency %fMHz - " HAM_PREFIX HAM_CALL HAM_SUFFIX " - " HAM_LOCATOR " - %ddBm\n\n", WSPR_TX_FREQ/1000000.0, HAM_POWER);

/*	EEPROM.begin(8);
	if (EEPROM.read(0) != (0x50+(EEPROM_VERSION))) 
	{
		EEPROM.write(0, 0x50+(EEPROM_VERSION));
		EEPROM.write(1, EEPROM_INDEX);
		EEPROM.commit();
	}
	uint8_t i;
	if ((i = EEPROM.read(1)) > (sizeof(BTCExchange) / sizeof(BTCExchange[0]))) 
	{
		PRINTF("EEPROM Exchange out of range: %d\n", i);
		i = 0;
		EEPROM.write(1, i);
		EEPROM.commit();
	}
	EEPROM.end();
*/

//	WiFi.disconnect();				// Debug!! Testing delete all wifi profiles onboard
//	WiFi.softAPdisconnect(true);	// TEST TEST TEST TEST TEST TEST TEST TEST
	WiFi.hostname(HOSTNAME);

	// Try to startup the WiFi Multi connection with the strongest AP found.
	ssd1306_text(200, "WiFiMulti Init");
#ifdef WIFI_SSID_01
	wifiMulti.addAP(WIFI_SSID_01);
#endif
#ifdef WIFI_SSID_02
	wifiMulti.addAP(WIFI_SSID_02);
#endif
#ifdef WIFI_SSID_03
	wifiMulti.addAP(WIFI_SSID_03);
#endif

#if 0
	PRINTF("Connecting .");
	ssd1306_text(200, "WM: Connect");
	int i = 0;
	while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
		delay(400);
		PRINTF(".");
	}
	PRINTF("\n");
	ssd1306_text(200, "WM: Connected");
#endif

	// Don't save WiFi configuration in flash - optional
	WiFi.persistent(false);
	// Set WiFi to station mode
	WiFi.mode(WIFI_STA);
	// Keep WiFi connected
	WiFi.setAutoReconnect(true);

#if 0
	// autoconfiguration portal for wifi settings
	ssd1306_text(200, "WiFi Manager");
	wifiManager.setConfigPortalTimeout(5*60);
#ifdef DEBUG_ESP_PORT
	wifiManager.setDebugOutput(true);
#endif
//	wifiManager.setWiFiAPChannel(9);
	wifiManager.setMinimumSignalQuality(10);  // 10%
	wifiManager.setAPCallback(configModeCallback);
//	sprintf(PASSWORD, "%08X", ESP.getChipId());		// Comment, use default

#if 1
//	wifiManager.autoConnect(HOSTNAME, PASSWORD);
	bool success = wifiManager.autoConnect(HOSTNAME, PASSWORD);
	if (success) {
		PRINTF("WiFiManager connected!\n");
	} else {
		PRINTF("WiFiManager Failed to connected.\n");
	}
#else
	wifiManager.startConfigPortal(HOSTNAME, PASSWORD);
#endif

#endif

	init_sntp_now();					// Init the sNTP client to get the real time

#ifdef FEATURE_mDNS
	init_mdns();						// Init the broadcast DNS server (.local)
#endif

#ifdef FEATURE_OTA
	init_ota();
#endif

	init_si5351();						// Init the frequency generator SI5351
	init_oneWire();						// Init the Dallas temperature one-wire sensor

	make_slot_plan(true);				// The first hour slot plan definition

	time_t now = time(nullptr);			// get UNIX timestamp 
	Serial.print(ctime(&now));			// convert timestamp and display

	pinMode(LED_BUILTIN, OUTPUT);		// LED on the ESP
	digitalWrite(LED_BUILTIN, HIGH);	// High is off!

	timer_20ms_loop = millis();

#ifdef FEATURE_CARRIER
	// Start with default CW carrier to calibrate the signal
	si5351.set_freq(  (WSPR_TX_FREQ + 100) * SI5351_FREQ_MULT, SI5351_CLK0);

	si5351.set_clock_pwr(SI5351_CLK0, 1);
	si5351.output_enable(SI5351_CLK0, 1);
	PRINTF("CW Carrier on: %fMHz\n", (float)(WSPR_TX_FREQ + 100)/1e6);
#endif

	//Register event handlers
	wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
	wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

	ssd1306_text(200, "Start looping");
	Serial.println("==== Start looping ====");
}

#if 0
void configModeCallback (WiFiManager *myWiFiManager) 
{
	ssd1306_background();
	display.setCursor(0, 16);
	display.printf(" SSID: %s\n PASS: %s\n   IP: ", 
			myWiFiManager->getConfigPortalSSID().c_str(), PASSWORD);
	display.print(WiFi.softAPIP());
	display.display();
/*
	PRINTF("WiFi portal config mode\nSSID: %s\nIP  : %s\n", 
			myWiFiManager->getConfigPortalSSID().c_str(),
			WiFi.softAPIP());
*/
}
#endif

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
	wifi_connected_msg = true;
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
}

void init_oneWire()
{
	sensors.begin();
	sensors.requestTemperatures();
	temperature_now = sensors.getTempCByIndex(0) + TEMP_CORRECTION;  
	PRINTF("Sensor init temperature %.1fºC\n", temperature_now);
}

#ifdef FEATURE_mDNS
static void init_mdns()
{
	ssd1306_text(200, "mDNS Setup");
	if (MDNS.begin(HOSTNAME))
		MDNS.addService("http", "tcp", 80);
	else
		PRINTF("mDNS ERROR!\n");
}
#endif

#ifdef FEATURE_OTA
// No authentication by default
// ArduinoOTA.setPassword("admin");
// Password can be set with it's md5 value as well
// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

void ota_start()
{
	PRINTF("OTA Start\n");
	ssd1306_text(50, "OTA-START");
}

void ota_stop()
{
	PRINTF("OTA Stop\n");
	ssd1306_text(100, "OTA-STOP");
	delay(200);
	Serial.flush();
	ESP.restart();
}

static int report_perc = 0;

void ota_progress(unsigned int progress, unsigned int total)
{
//	PRINTF("OTA Progress %d/%d\n", progress, total);

	int perc = (uint32_t)progress * 100UL / total;
	if (perc >= report_perc) 
	{
		PRINTF("OTA Progress %d%%\n", perc);
		report_perc += 5;

		ssd1306_background();
		display.setTextSize(1);
		display.setCursor(16 , 16);
		display.printf("Progress: %u%%\r", perc );
		display.display();
	}
}

static void ota_error(ota_error_t error) 
{
    PRINTF("OTA Error[%u]: ", error);

    if (error == OTA_AUTH_ERROR) 			PRINTF("Auth Failed")
    else if (error == OTA_BEGIN_ERROR) 		PRINTF("Begin Failed")
    else if (error == OTA_CONNECT_ERROR) 	PRINTF("Connect Failed")
    else if (error == OTA_RECEIVE_ERROR) 	PRINTF("Receive Failed")
    else if (error == OTA_END_ERROR) 		PRINTF("End Failed")
}

static void init_ota()
{
	PRINTF("OTA Initialize\n");
	ssd1306_text(200, "OTA setup");
	ArduinoOTA.onStart(ota_start);
	ArduinoOTA.onProgress(ota_progress);
	ArduinoOTA.onEnd(ota_stop);
	ArduinoOTA.onError(ota_error);
	ArduinoOTA.setHostname(HOSTNAME);
//	ArduinoOTA.setPassword("pe0fko");
	ArduinoOTA.begin();
}
#endif

static void init_si5351()
{
	ssd1306_text(200, "Si5351 init");

	if ( si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, SI5351_FREQ_CORRECTION_02) )
	{
		si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // 4mA=3dBm, 8mA=10dBm
		si5351.output_enable(SI5351_CLK0, 0);                 // Disable the clock initially
		si5351.output_enable(SI5351_CLK1, 0);
		si5351.output_enable(SI5351_CLK2, 0);
		si5351.set_clock_pwr(SI5351_CLK0, 0);                 // Set power down
		si5351.set_clock_pwr(SI5351_CLK1, 0);
		si5351.set_clock_pwr(SI5351_CLK2, 0);
		ssd1306_text(200, "Si5351 OK");
		PRINTF("SI5351 Initialized\n");
	}
	else
	{
		ssd1306_text(200, "Si5351 ERROR");
		PRINTF("ERROR: SI5351 not found on I2C bus!\n");
	}
}

void init_sntp_now()
{
	sntp_stop();
//	sntp_init();

	settimeofday_cb(time_is_set);	// Call-back function
//	sntp_set_time_sync_notification_cb(time_is_set);

	// Google NTP servers
	configTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "time.google.com");

//	timer_ntp_faild_reboot = value_ntp_faild_reboot;
	timer_ntp_faild_reboot = millis();
	ntp_time_sync = false;
}

// callback routine - arrive here whenever a successful NTP update has occurred
void time_is_set (void)
{
#if 0
	time_t now = time(nullptr);          // get UNIX timestamp 
	Serial.print("NTP update done at: ");
	Serial.print(ctime(&now));
#endif
	ntp_time_sync = true;
}


//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....  
//---------------------------------------------------------------------------------
void loop() 
{
#ifdef FEATURE_OTA
	ArduinoOTA.handle();
#endif

#ifdef FEATURE_mDNS
	MDNS.update();
#endif

	// If still no WiFi then try again later...
	switch(wifiMulti.run()) 
	{
		case WL_CONNECTED:
//		PRINTF("WiFi connected!\n");
		break;

		case WL_CONNECT_FAILED:
		PRINTF("WiFi connect failed.\n");
		ssd1306_text(200, "WiFi connect", "FAILED");
		timer_display_auto_off = millis();	// Start the display ON timer
		display_switch_status = DISPLAY_ON;
		return;		// Return the loop!!

		case WL_WRONG_PASSWORD:
		ssd1306_text(200, "WiFi connect", "Error PASSWD");
		return;		// Return the loop!!

//		case WL_IDLE_STATUS:
//		case WL_NO_SSID_AVAIL:
//		case WL_DISCONNECTED:
		default:
		ssd1306_text(200, "WiFi connect", "ERROR");
		return;		// Return the loop!!
	}

	// Tell us what network we're connected to
	if (wifi_connected_msg) {
		wifi_connected_msg = false;
		ssd1306_wifi_page();
	}

	// Wait for the NTP time service is known!
	if (!ntp_time_sync) 
	{
		ssd1306_text(200, "Waiting", "NTP sync");
		time_t now = time(nullptr);			// get UNIX timestamp 
		Serial.print("Waiting NTP sync, time: ");
		Serial.print(ctime(&now));
		time(nullptr);

		if ((millis() - timer_ntp_faild_reboot) >= value_ntp_faild_reboot)
		{
			Serial.println("System Restart: No NTP time received.");
			Serial.flush();
			ESP.restart();
		}

		sntp_init();
		return;		// Return the loop!!
	}

	// ----------------------------------------
	// Now we have WiFi running,
	// and a valid time from the NTP servers!

	// Send the WSPR bits into the air if active TX!
	// When started it will wait for the bit time and start a next bit.
	if (wspr_symbol_index != 0 && (millis() - timer_wspr_bit_ms) >= value_wspr_bit_ms)
	{
		timer_wspr_bit_ms += value_wspr_bit_ms;
		wspr_bit_tx();
	}

	//
	//++ 50Hz code lus
	// Used for slower processing
	//
	if ((millis() - timer_20ms_loop) >= value_20ms_loop)    // Every 20ms...
	{
		timer_20ms_loop += value_20ms_loop;

		time_t		now			= time(nullptr);
		uint16_t	sec			= now % 60;
		uint16_t	sec2min		= now % 120;
		uint8_t		wspr_slot	= (now / 120) % WSPR_SLOTS_MAX;		// Slot in the hour, 0..29
static 	int8_t 		last_sec	= -1;

		// Check if button is pressed to lightup the display
		int switchStatus = digitalRead(BUTTON_INPUT);			// read status of switch
		if (switchStatus != switchStatusLast)  // if status of button has changed
		{
			  switchStatusLast = switchStatus;
			  if (switchStatus == LOW) {
				display_switch_status = display_switch_status == DISPLAY_ON ? DISPLAY_OFF : DISPLAY_ON;
				if (display_switch_status == DISPLAY_ON) {
					timer_display_auto_off = millis();  	// Start the display ON timer
					digitalWrite(LED_BUILTIN, HIGH);		// Switch the ESP LED off
				}
				last_sec = -1;								// Fast update of the display
				PRINTF("Button pressed, display_switch_status=%d\n", display_switch_status);
			  }
		}

		// Blink the ESP LED every 4s if display is off
		if (display_switch_status == DISPLAY_NOTHING)
		{
			static uint8_t led_pwm;
			led_pwm %= 200;
			digitalWrite(LED_BUILTIN, led_pwm++ == 0 ? LOW : HIGH);
			delay(3);
			digitalWrite(LED_BUILTIN, HIGH);
		}
		
		//
		//++ Updates for every second
		//
		if (sec != last_sec)
		{
			last_sec = sec;
      
			//
			//++ At every 2 minute interval start a WSPR message, if slot is richt.
			//
			if (sec2min == 0)	// First second of the 2 minute interval clock
			{
				char const* call = NULL;

				if (wspr_slot == 0)
					make_slot_plan(false);

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_1)		// Type 1 message: CALL, LOC4, dBm
					call = HAM_CALL;

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_2)		// Type 2 message: pre/CALL/suff, dBm
					call = HAM_PREFIX HAM_CALL HAM_SUFFIX;

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_3)		// Type 3 message: hash <pre/CALL/suff>, LOC6, dBm
					call = "<" HAM_PREFIX HAM_CALL HAM_SUFFIX ">";

				if (call != NULL) 
				{
					PRINTF("WSPR TX slot %d, band %d, TX call: %s\n", 
							wspr_slot, wspr_slot_band[wspr_slot], call);

					wspr.wspr_encode(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);

					// Calc the SI5351 frequency setting.
					wspr_frequency  = 
							SI5351_FREQ_MULT * (uint64_t)(WSPR_TX_FREQ)
						+ 	SI5351_FREQ_MULT * wspr_slot_band[wspr_slot];

					timer_wspr_bit_ms = millis();
					wspr_bit_tx();

//					extern void print_wspr_symbols(const char* call, const char* loc, uint8_t power, uint8_t symbols[]);
//					print_wspr_symbols(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);
				}
			}

			//
			//++ At avery 2 minute interval, second 60, print the temperature
			//
			if (sec2min == 60)
			{
				sensors.requestTemperatures(); 
				temperature_now = sensors.getTempCByIndex(0) + TEMP_CORRECTION;  
				Serial.printf("Actual Temerature %.1fºC\n", temperature_now);

				Serial.print("Time now: ");
				Serial.print(ctime(&now));
			}

			if (display_switch_status == DISPLAY_OFF)
			{
				display.clearDisplay();
				display.invertDisplay(false);
				display.display();
				display_switch_status = DISPLAY_NOTHING;
			}
			
			if (display_switch_status == DISPLAY_ON)
			{
				if ((millis() - timer_display_auto_off) >= value_display_auto_off)
				{
					display_switch_status = DISPLAY_OFF;
					PRINTF("Display auto Off at %d sec.\n", value_display_auto_off/1000);
				}
				else
				{
					ssd1306_background();

					uint16_t w;
					char buffer[30];
					struct tm *timeinfo;
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
						E = B + sec2min;
						display.drawLine(B, display.height()-14, E, display.height()-14, WHITE);
						display.drawLine(B, display.height()-16, E, display.height()-16, WHITE);
					}
					else
					{
						int s = wspr_slot, w = -sec2min;
						while (wspr_slot_tx[s++] == WSPR_TX_NONE) {
							if (s >= WSPR_SLOTS_MAX) s = 0;
							w += 120;
						}
						if (w >= 0) {
							sprintf(buffer, "TX %02d:%02d", w/60, w%60);
							display.setCursor(14, display.height()-20);
							display.print(buffer);
						}				

						sprintf(buffer, "%.1f", temperature_now);
						w = getFontStringWidth(buffer);
						display.setCursor(display.width() - 14 - w, display.height()-20);
						display.print(buffer);
					}
	  
					display.invertDisplay(wspr_symbol_index != 0);
					display.display();
				}
			}	// DISPLAY_ON
		}   	// One second lus
	}   		// 50Hz code lus
}

void wspr_bit_tx()
{
	if (wspr_symbol_index == 0)
	{
		PRINTF("WSPR TX Start transmission\n");
	}

	if (wspr_symbol_index != WSPR_SYMBOL_COUNT) 
	{
		if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], SI5351_CLK0 ) )
		{
	  		PRINTF("WSPR TX SI5351::set_freq() error.\n");
		}

		if (wspr_symbol_index == 0)   // On first bit enable the tx output.
		{
			si5351.set_clock_pwr(SI5351_CLK0, 1);
			si5351.output_enable(SI5351_CLK0, 1);
		}

		wspr_symbol_index += 1;
	}
	else
	{
		si5351.output_enable(SI5351_CLK0, 0);
		si5351.set_clock_pwr(SI5351_CLK0, 0);
		wspr_symbol_index = 0;
		wspr_tx_counter += 1;
		PRINTF("WSPR TX %d Ended.\n", wspr_tx_counter);
	}
}

// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2	
void print_wspr_symbols(const char* call, const char* loc, uint8_t power, uint8_t symbols[])
{
	PRINTF("%s %s %ddBm\n", call, loc, power);
	for (uint8_t i = 0; i < WSPR_SYMBOL_COUNT; ++i)
	{
		PRINTF("%d,", symbols[i]);
	}
	PRINTF("\n");
}

void ssd1306_background()
{
	char buffer[20];
	int16_t   x, y;
	uint16_t  w, h;

	sprintf(buffer, " %d ", wspr_tx_counter);

	display.getTextBounds(buffer, 0, 0, &x, &y, &w, &h);
	display.clearDisplay();
	display.drawRoundRect(x, h/2-1, display.width(), display.height() - h/2-1, 8, WHITE);
	display.setTextSize(1);
	display.setTextColor(WHITE);

	x = (display.width()- w) / 2;
	display.fillRect(x, y, w, h, BLACK);
	display.setCursor(x, y);
	display.print(buffer);
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

//	display.setCursor(8, 52);
//	display.print(WiFi.SSID());              // Tell us what network we're connected to

	display.display();
	if (delay_ms >= 0)
		delay(delay_ms);
}

void ssd1306_wifi_page()
{
	PRINTF("Connected to %s (IP:%s, RSSI %d)\n"
		, WiFi.SSID().c_str()              
		, WiFi.localIP().toString().c_str()           // Send the IP address of the ESP8266 to the computer
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
	delay(5000);
}
