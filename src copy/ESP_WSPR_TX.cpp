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

#include <stdlib.h>
#include <Arduino.h>
#include <coredecls.h>
#include <time.h>
#include <TZ.h>
#include <sntp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>		// Include the Wi-Fi-Multi library
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>			// ArduinoJson
#include <Adafruit_GFX.h>			// Adafruit GFX Library               1.11.5
#include <Adafruit_SSD1306.h>		// Adafruit SSD1306 Wemos Mini OLED
#include <si5351.h>					// Etherkit
#include <JTEncode.h>				// Etherkit
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20
#include <WiFi_SSID.h>				// WiFi SSID's from know networks
#include <QTHLocator.h>				// Get the QTH locator from the internet

#ifdef FEATURE_OTA					// OTA & mDNS (included)
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#endif

#include "header.h"

//#define	URL_BASE	"http://sample.nl/wspr/client_"
//#define	URL_ID		"NL"

// JSon static and default config.
const	char	localConfig[] = R"(
{		"call":"PE0aaaa"
,		"txenabled":false
,		"hostname":"wspr"
}
)";
//,		"locator":"JO00AA"

/* ==== Example ====
{       "chipid":6479671
,       "call":"PE0FKO"
,       "prefix":""
,       "suffix":""
,       "locator":"JO32CD"
,       "power":"10"
,       "hostname":"wsprtx"
,		"txenabled":true
,       "rndseed":19561113
,       "displayoff":300
,       "freq_correction":762
,		"timezones":[
		{
			"start":"11:55",
			"sunrise":-1:30,
			"sunset":"1:00",
			"end":"12:05",
			"clk":0,
			"list":"clk0N"
		},
		{"start":6,"end":18,"clk":0,"list":"clk0D"},
		{"start":18,"end":24,"clk":0,"list":"clk0N"},
		{"start":0,"end":24,"clk":1,"list":"clk1"},
		{"start":0,"end":24,"clk":2,"list":"clk2"}	]
,       "clk0N":[7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14]
,       "clk0D":[7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14,7,3,14]
,       "clk1":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
,       "clk2":[0,144,0,50,0,0,144,0,50,0,0,144,0,50,0,0,144,0,50,0,0,144,0,50,0,0,144,0,50,0]
,       "wsprtype":[3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2]
,       "temp_wspr_band":21
,       "temp_wspr_clk":0
,       "temp_correction":-1.0
}
*/


config_t config	=
{		"PE0xxx",
		"",
		"",
		"JO32xx",
		10,
		0,
		0,
		HOSTNAME,
		300,
		0.0
};


#define	SCREEN_WIDTH	128			// OLED display width, in pixels
#define	SCREEN_HEIGHT	64			// OLED display height, in pixels
#define OLED_RESET		-1			// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS	0x3C		// See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

static		Adafruit_SSD1306	display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static		Si5351				si5351;
static		JTEncode			wspr;
			QTHLocator			QTH;										// Get the QTH locator from the internet
			JsonDocument		jsonDoc;									// Allocate the JSON document

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
const		uint32_t			value_no_network_connection	= 4 * 60 * 1000;	// 4 min (ntp must be init updated in this time)
static		uint32_t			timer_no_network_connection;
const		uint32_t			value_ms_led_blink_on	= 3141UL;			// 4sec interval blink led
static		uint32_t			timer_ms_led_blink_on	= 0;
const		uint32_t			value_ms_led_blink_off	= 3UL;				// 3ms led on
static		uint32_t			timer_ms_led_blink_off	= 0;
const		uint32_t			sntp_update_delay		= 3600 * 1000UL;	// NTP update every 1h
volatile	bool				ntp_time_sync			= false;
			float				temperature_now			= 0.0;
static		uint8_t				switchStatusLast		= HIGH;				// last status hardware switch
static		enum { DISPLAY_OFF,	DISPLAY_ON }	display_status = DISPLAY_ON;

static		uint8_t				wspr_symbols[WSPR_SYMBOL_COUNT];
static		uint32_t			wspr_symbol_index		= 0;
static		uint32_t			wspr_tx_counter        	= 0;

			uint8_t				hour_now;								// Time now in hour (0..23)
			uint8_t				slot_now;								// Time now in slot (0..29)
			uint8_t				slot_sec;								// 

			int					wspr_slot_type[WSPR_SLOTS_HOUR];				// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
			uint32_t			wspr_slot_band[WSPR_SLOTS_HOUR];				// Band freqency, 0 .. 200 Hz
			uint32_t			wspr_slot_freq[WSPR_SLOTS_HOUR][3];			// TX frequency for every CLK output (0..2)


const	int32_t      	wspr_sym_freq[4] =
{	static_cast<uint32_t> ( 0.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 1.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 2.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 3.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
};


//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);				// LED on the ESP
	digitalWrite(LED_BUILTIN, HIGH);			// High is off!
	pinMode(BUTTON_INPUT, INPUT_PULLUP);		// Button for display on/off

	Serial.begin(115200);
	Serial.setTimeout(2000);
	while(!Serial);
#ifdef DEBUG_ESP_PORT
	Serial.setDebugOutput(true);
#endif
	delay(1000);

	// sensors.begin();					// Init the onewire for the DS18B20 temp sensor
	// LOG_I("DS18B20: %d devices found\n", sensors.getDeviceCount());
	// sensors.setResolution(12);			// Set the resolution to 12 bit
	// // sensors.setWaitForConversion(true);	// No blocking wait for the conversion
	// // sensors.setWaitForConversion(true);	// Block until the sensor is read
	// sensors.requestTemperatures();
	// LOG_I("DS18B20: %d devices found\n", sensors.getDeviceCount());

	LOG_I("=== PE0FKO, TX WSPR temperature coded\n");
	LOG_I("=== Version: " VERSION ", Build at: " __DATE__ " " __TIME__ "\n");
	LOG_I("=== Config: %s - %s - %ddBm\n", config.call.c_str(), config.qth.c_str(), config.power);
	LOG_I("=== ChipId: %d, Hostname: %s.local\n", ESP.getChipId(), config.hostname.c_str() );
	LOG_I("=== SSD1306: Display %dx%d address:0x%02x\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH, SCREEN_ADDRESS);
	// LOG_I("value_us_wspr_bit: %ld\n", value_us_wspr_bit);

	// Start the display driver
	// Wire.begin(D2, D1);						// I2C SDA, SCL
	Wire.setClock(400000);						// I2C clock speed 400kHz
	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
//	display.setRotation(2);						// Display upside down...

	// Json decode the first, basic, config
	jsonSetConfig(localConfig);					// Load the default config
	makeSlotPlan();								// Make a plan for the TX slots at 0h

	// ssd1306_display_on();						// Start the display ON timer
	ssd1306_printf_P(2000, PSTR("WSPR TX\nID %d\n%s.local"), ESP.getChipId(), config.hostname.c_str());

	setupWifiStatioMode();						// Setup the Wifi for the NTP service 

	sensors.begin();							// Init the onewire for the DS18B20 temp sensor
	sensors.setResolution(12);					// Set the resolution to 12 bit
	// sensors.setWaitForConversion(true);		// No blocking wait for the conversion
	sensors.setWaitForConversion(true);			// Block until the sensor is read
	LOG_I("DS18B20: %d devices found\n", sensors.getDeviceCount());

	ReadTemperature();							// Read the Dallas temperature one-wire sensor
	init_si5351();								// Init the frequency generator SI5351

#ifdef FEATURE_CARRIER
	LOG_I("CW Carrier on: %fMHz (CLK%d)\n", (float)CARRIER_FREQUENCY/1e6, CARRIER_SI5351_CLK);
	si5351.set_freq( SI5351_FREQ_MULT * CARRIER_FREQUENCY, CARRIER_SI5351_CLK );
	si5351.drive_strength( CARRIER_SI5351_CLK, SI5351_DRIVE_8MA );		// 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
	si5351.set_clock_pwr( CARRIER_SI5351_CLK, 1);
	si5351.output_enable( CARRIER_SI5351_CLK, 1);
#endif

#ifdef FEATURE_OTA
	if ( ! config.hostname.isEmpty())
	{
		LOG_I("Setup OTA: %s\n", config.hostname.c_str());
		ArduinoOTA.setHostname(config.hostname.c_str());

#ifdef OTAPASSWD
		ArduinoOTA.setPassword(OTAPASSWD);
#endif
		// LOG_I("Set OTA: %s\n", config.hostname.c_str());
		// ArduinoOTA.setHostname(config.hostname.c_str());
		ArduinoOTA.onStart([]() 
			{	ssd1306_printf_P(1000, PSTR("OTA update\nRunning")); 
				wspr_tx_disable(SI5351_CLK0);
				wspr_tx_disable(SI5351_CLK1);
				wspr_tx_disable(SI5351_CLK2);
			});
		ArduinoOTA.onEnd([]()   
			{	ssd1306_printf_P(1000, PSTR("OTA update\nReboot"));
				ESP.restart();
			});
		ArduinoOTA.begin();
	}
#endif


	ssd1306_printf_P(300, PSTR("Start\nLooping"));
	ssd1306_main_window();
	LOG_I("=== Start looping...\n");
}

void setupWifiStatioMode()
{
	timer_no_network_connection = millis();						// Keep track of the first received NTP
	ntp_time_sync = false;

	WiFi.disconnect(false);										// Cleanup old info
	WiFi.mode(WIFI_STA);										// Set WiFi to station mode
	WiFi.setHostname(config.hostname.c_str());					// Set Hostname.
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
	LOG_I("WiFi connected: SSID %s, channel %d\n", ssid.ssid.c_str(), ssid.channel);
}

void onWiFiGotIP(const WiFiEventStationModeGotIP& ipInfo)
{
	LOG_I("WiFi IP %s, mask %s, GW:%s\n",
		ipInfo.ip.toString().c_str(),
		ipInfo.mask.toString().c_str(),
		ipInfo.gw.toString().c_str()
	);

	settimeofday_cb(sntp_time_is_set);			// Call-back NTP function
	configTime(MYTZ, "time.google.com", "nl.pool.ntp.org");
}

// callback routine - arrive here whenever a successful NTP update has occurred
void sntp_time_is_set(bool from_sntp)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);					// Get the current time in sec and usec
	timer_us_one_second = micros();				// Initialize the timer only ones!
	timer_us_one_second -= tv.tv_usec;			// Correct the us to the sec tick

	ntp_time_sync = true;						// and the new time is set
	timer_no_network_connection = 0;			// NTP Received ok, no reboot needed!

//	stopWifiStatioMode();						// Stop the WiFi connection

	LOG_I("NTP update at [%dus] [%d] %s", tv.tv_usec, timer_us_one_second, ctime(&tv.tv_sec));
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo)
{
	LOG_I("WiFi disconnected from SSID: %s, Reason: %d\n"
	,	disconnectInfo.ssid.c_str()
	,	disconnectInfo.reason
	);

	sntp_stop();
	ntp_time_sync = false;
}

uint32_t sntp_update_delay_MS_rfc_not_less_than_15000 ()
{
	return sntp_update_delay;
}

bool loadWebConfigData()
{
//	String URL(URL_BASE);
	String URL("https://pe0fko.nl/wspr/id/client_");

	URL.concat(String(ESP.getChipId()));
	URL.concat('_');
	URL.concat(URL_ID);
	URL.concat(".json");

	String page;
	if (QTH.fetchWebPage(page, URL) == HTTP_CODE_OK) {
			jsonSetConfig(page);
	} else {
		LOG_E("Error loading web config, from: %s\n", page.c_str());
		// displayMessage(1000, "E: JSON");
		return false;
	}
	return true;
}


// void printJsonDoc(String txt, JsonObject& jsonDoc)
// void printJsonDoc(String txt, JsonDocument& jsonDoc)
// JsonVariantConst
void printJsonDoc(String txt, JsonDocument& jsonDoc)
{
	if (display_status == DISPLAY_ON)
	{
		LOG_I("JSON %s: ", txt.c_str()); 
		serializeJson(jsonDoc, Serial);	
		LOG_I("\n");
	}
}



void jsonSetConfig(String jsonString)
{
	jsonDoc.clear();

	DeserializationError error =  deserializeJson(jsonDoc, jsonString);
	if (error != DeserializationError::Ok) {
		LOG_E("Error Json Seserialize: %s\n", error.f_str());
		// displayMessage(1000, "E: JSON");
		return;
	}

	// LOG_I("JSON:"); serializeJson(jsonDoc, Serial);	LOG_I("\n");
	printJsonDoc("Config", jsonDoc);

	config.call				= jsonDoc["call"]				| "PA0xxx";
	config.prefix			= jsonDoc["prefix"]				| "";
	config.suffix			= jsonDoc["suffix"]				| "";
	config.hostname			= jsonDoc["hostname"]			| "";
	config.power			= jsonDoc["power"]				| 10;
	config.freq_cal_factor	= jsonDoc["freq_correction"]	| 0UL;
	config.randomSeed		= jsonDoc["rndseed"]			| 19570215;
	config.displayOff		= jsonDoc["displayoff"]			| 120;
	config.temp_cor			= jsonDoc["temp_correction"]	| 0.0;

	if (jsonDoc["locator"].is<const char*>()) 
	{
		config.qth = jsonDoc["locator"]	| "JO32xx";
		Serial.printf("locator: by config, %s\n", config.qth.c_str());
	}
	else 
	{
		config.qth = QTH.getQthLoc();
		Serial.printf("locator: by QTH.getQthLoc(), %s\n", config.qth.c_str());
	}

	// Display the config data
	//=========================================================
	LOG_I("INIT: Wspr call sign   	: %s\n",			config.call.c_str());
	LOG_I("INIT: Wspr call prefix 	: %s\n",			config.prefix.c_str());
	LOG_I("INIT: Wspr call suffix 	: %s\n",			config.suffix.c_str());
	LOG_I("INIT: Wspr qth locator 	: %s\n",			config.qth.c_str());
	LOG_I("INIT: Wspr TX power    	: %ddBm\n",			config.power);
	LOG_I("INIT: Wspr TX hostname	: %s.local\n",		config.hostname.c_str());
	LOG_I("INIT: Display auto off 	: %d seccond\n",	config.displayOff);
	LOG_I("INIT: Freq callibration	: %ld\n", 			config.freq_cal_factor);
	LOG_I("INIT: Temperature sensor correction %.1f\n",	config.temp_cor);

	// Set the frequency correction value
	si5351.set_correction(config.freq_cal_factor, si5351_pll_input::SI5351_PLL_INPUT_XO);

	//++ Set the random seed ones every day.
	//   Posible track of (semi) random numbers!
	if (hour_now == 23)	// && slot_now == 29)
	{
		LOG_I("Set the const ramdom seed number 0x%08x\n", config.randomSeed);
		randomSeed(config.randomSeed);
	}

	if ( config.hostname.isEmpty() == false )
	{
		// LOG_I("INIT: hostname=%s\n", config.hostname.c_str());
		WiFi.setHostname(config.hostname.c_str());					// Set WiFi Hostname.
		MDNS.setHostname(config.hostname.c_str());					// Set mDNS hostname (for next setting)
	}
}



uint32_t
getWsprBandFreq(int band)
{
	switch(band) {
		case 0:		break;						// MF      138000 ?		Now it is No Band
		case 1:		return	WSPR_TX_FREQ_160m;	// 160m  1.838000 -  1.838200
		case 3:		return	WSPR_TX_FREQ_80m;	// 80m   3.570000 -  3.570200
		case 5:		return	WSPR_TX_FREQ_60m;	// 60m   5.288600 -  5.288800
		case 7:		return	WSPR_TX_FREQ_40m;	// 40m   7.040000 -  7.040200
		case 10:	return	WSPR_TX_FREQ_30m;	// 30m  10.140100 - 10.140300
		case 14:	return	WSPR_TX_FREQ_20m;	// 20m  14.097000 - 14.097200
		case 18:	return	WSPR_TX_FREQ_17m;	// 17m  18.106000 - 18.106200
		case 21:	return	WSPR_TX_FREQ_15m;	// 15m  21.096000 - 21.096200
		case 24:	return	WSPR_TX_FREQ_12m;	// 12m  24.926000 - 24.926200
		case 28:	return	WSPR_TX_FREQ_10m;	// 10m  28.126000 - 28.126200
		case 50:	return	WSPR_TX_FREQ_6m;	// 6m   50.294400 - 50.294600
		case 70:	return	WSPR_TX_FREQ_4m;	// 4m   70.092400 - 70.092600?
		case 144:	return	WSPR_TX_FREQ_2m;	// 2m  144.489900 - 144.490100
		case -1:								// LF  ?
		case 433:								// 70cm 433. - 433.
		case 1296:								// 23cm  1296. - 1296.
					LOG_E("Not implemented WSPR bands\n");
					break;
		default:	LOG_E("Wrong WSPR band %d selected.\n", band);
					break;						// No TX mode
	}
	return WSPR_TX_FREQ_NONE;
}


int
getWsprSlotType(int type)
{
	if (type < WSPR_TX_NONE || type > WSPR_TX_TYPE_3) 
	{
		LOG_E("Wrong WSPR type %d selected.\n", type);
		type = WSPR_TX_NONE;
	}
	return type;
}


void ReadTemperature()
{
	sensors.setWaitForConversion(true);	// Block until the sensor is read
	sensors.requestTemperatures();

	int devices = sensors.getDeviceCount();

	temperature_now = sensors.getTempCByIndex(0) + config.temp_cor;

	LOG_I("Sensor DS18B20 (#%d) temperature %.1fºC\n", devices, temperature_now);
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------
void loop()
{
	// Wait for the WiFi and NTP-time is known!
	if (timer_no_network_connection != 0)
	{
		// Still no network, then reboot after some time
		if ((millis() - timer_no_network_connection) >= value_no_network_connection)
		{
			ssd1306_printf_P(1000, PSTR("REBOOT\nNTP sync"));
			LOG_F("REBOOT: No NTP time received.\n");
		}
	}
	else
	{
		static bool firstBoot = true;
		if (firstBoot) 
		{
			firstBoot = false;

			setSlotTime();				// Get the current time and slot
			if (!QTH.begin())			// Get the QTH locator from the internet
			{
				LOG_E("Error loading QTH locator\n");
				ssd1306_printf_P(1000, PSTR("Error loading QTH"));
			}

			if (loadWebConfigData())	// Load the config data
				makeSlotPlan();			// Make a plan for the this TX hour

#if 0
			// Get the sunrise/sunset data from the web
			if (QTH.getDayligth())
			{
				LOG_I("Daylight: Rise=%s, Set=%s\n", QTH.getSunRise().c_str(), QTH.getSunSet().c_str());
				// ssd1306_printf_P(1000, PSTR("Daylight: %s"), QTH.getSunRise().c_str());
			}
			else
			{
				LOG_E("Error loading daylight data\n");
				// ssd1306_printf_P(1000, PSTR("Error loading daylight data"));
			}
#endif	
		}

		loop_wspr_tx();
		loop_1s_tick();
		loop_20ms_tick();
	}

	yield();

	// Some default householding
	loop_wifi_tick();
	loop_led_tick();

#ifdef FEATURE_OTA
	ArduinoOTA.handle();
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

void setSlotTime()
{
	time_t now = time(NULL);
	struct tm* timeinfo = localtime(&now);

	int m = timeinfo->tm_min;
	int s = timeinfo->tm_sec;
	hour_now	= timeinfo->tm_hour;
	slot_now	= m / 2;
	slot_sec	= s + (m % 2 ? 60 : 0);
}

// Secure one second function call
void loop_1s_tick() 
{
	if ((micros() - timer_us_one_second) < value_us_one_second)
		return;

	timer_us_one_second +=	value_us_one_second;

	setSlotTime();						// Get the current time and slot

	//++ At every 2 minute interval start a WSPR message, if slot is richt.
	if (slot_sec == 0)												// First second of the 2 minute interval clock
	{
		String Call;

		switch (wspr_slot_type[slot_now]) {
		case WSPR_TX_NONE:							// No WSPR TX slot
			LOG_I("No WSPR TX slot %d\n", slot_now);
			break;
		case WSPR_TX_TYPE_1:						// Type 1 message: CALL, LOC4, dBm
			Call = config.call;
			break;
		case WSPR_TX_TYPE_2:						// Type 2 message: pre/CALL/suff, dBm
			Call  = config.prefix;
			Call += config.call;
			Call += config.suffix;
			break;
		case WSPR_TX_TYPE_3:						// Type 3 message: *hash* <pre/CALL/suff>, LOC6, dBm
			Call  = '<';
			Call += config.prefix;
			Call += config.call;
			Call += config.suffix;
			Call += '>';
		} 

		if (!Call.isEmpty()) 
		{
			wspr_tx_init(Call);
			Serial.printf("WSPR-Time: Hour:%u Slot:%u, CALL=%s, Freq=%d/%d/%d\n", 
				hour_now, slot_now, 
				Call.c_str(), 
				wspr_slot_freq[slot_now][0], wspr_slot_freq[slot_now][1], wspr_slot_freq[slot_now][2]);
		}
	}

	// Actions needed in the free time after a wspr tx (> 112sec)
	if (slot_sec == wspr_free_second) 
	{
		if (slot_now == 29)		// Last slot of the hour
		{
			// First load the Json config data from webserver
			loadWebConfigData();
			makeSlotPlan();			// Make a plan for the next TX hour
		}

		// Only read the temp once every 2min
		if (display_status == DISPLAY_ON) 
			ReadTemperature();
	}

	ssd1306_main_window();
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

			LOG_I("Button pressed, display_status=%d\n", display_status);
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

void init_si5351()
{
	LOG_I("SI5351 init: xtal:%d, correction:%d\n", SI5351_XTAL_FREQ, config.freq_cal_factor);

	ssd1306_printf_P(1000, PSTR("SI5351\nSetup"));

	// TCXO input to xtal pin 1?
	if ( si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, config.freq_cal_factor) )
	{
		// Disable the clock initially...
		wspr_tx_disable(SI5351_CLK0);
		wspr_tx_disable(SI5351_CLK1);
		wspr_tx_disable(SI5351_CLK2);

		ssd1306_printf_P(1000, PSTR("SI5351\nOk"));
	}
	else
	{
		LOG_E("SI5351 Initialize error\n");
		ssd1306_printf_P(1000, PSTR("SI5351\nERROR"));
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
		LOG_E("SI5351 I2C connect error 0x%04X\n", reg_val);
		ssd1306_printf_P(10*1000, PSTR("NO SI5351\n0x%04X"), reg_val);
		return false;
	}

	reg_val = si5351.si5351_read(SI5351_DEVICE_STATUS);
	reg_val &= 0b11100000;	// SYS_INIT | LOL_A | LOL_B
	if (reg_val)
	{
		LOG_E("WSPR TX, the SI5351 is not ready, 0x%02x\n", reg_val);
		ssd1306_printf_P(5*1000, PSTR("SI5351 Error\n0x%02x"), reg_val);
		return false;
	}

	return true;
}


void wspr_tx_init(String& call)
{
	if (si5351_ready())
	{
		LOG_I("WSPR TX with Call: %s, Loc:%s, Power:%ddBm\n", call.c_str(), config.qth.c_str(), config.power);
		wspr.wspr_encode(call.c_str(), config.qth.c_str(), config.power, wspr_symbols);

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
		print_wspr_symbols(call.c_str(), config.qth.c_str(), config.power, wspr_symbols);
#endif

		timer_us_wspr_bit = micros();
		wspr_tx_bit();
	}
	else
	{
		LOG_E("WSPR TX not started, SI5351 not ready.\n");
	}
}

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2
void print_wspr_symbols(const char* call, const char* loc, uint8_t power, uint8_t symbols[])
{
	LOG_I("%s %s %ddBm:\n  ", call, loc, power);
	for (uint8_t i = 0; i < WSPR_SYMBOL_COUNT; )
	{
		LOG_I("%d,", symbols[i++]);
		if (i % 41 == 0) LOG_I("\n  ");
	}
	LOG_I("\n");
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

		LOG_I("TX WSPR #%d Ended.\n", wspr_tx_counter);
	}
}

void wspr_tx_freq(si5351_clock clk)
{
	if (wspr_slot_type[slot_now] != WSPR_TX_NONE && 
		wspr_slot_freq[slot_now][clk] != 0)
	{
		uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[slot_now][clk] + wspr_slot_band[slot_now]);

		if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) ) {
			LOG_E("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
		}
	}
}

void wspr_tx_enable(si5351_clock clk)
{
	if (wspr_slot_type[slot_now] != WSPR_TX_NONE && 
		wspr_slot_freq[slot_now][clk] != 0)
	{
		LOG_I("TX WSPR start CLK%d: slot %d, freq %.6fMHz + %dHz\n", 
				clk, slot_now, 
				wspr_slot_freq[slot_now][clk] / 1000000.0, 
				wspr_slot_band[slot_now]);

//		si5351.drive_strength(clk, SI5351_DRIVE_8MA); 			// 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
		si5351.set_clock_pwr(clk, 1);
		si5351.output_enable(clk, 1);
		si5351.drive_strength(clk, SI5351_DRIVE_8MA);
	}
 }

void wspr_tx_disable(si5351_clock clk)
{
	si5351.output_enable(clk, 0);
	si5351.set_clock_pwr(clk, 0);
}

void ssd1306_main_window()
{
	char	buffer[40];
	int		next_tx_slot = 0;

	if (display_status == DISPLAY_OFF)
		return;

	if ((millis() - timer_ms_display_auto_off) >= 1000UL * config.displayOff)
	{
		ssd1306_display_off();
		return;
	}

	ssd1306_background();

	if (ntp_time_sync)
	{
		// Get local time
		time_t t = time(NULL);
		struct tm* timeinfo = localtime(&t);

		// Disply the actual time
		strftime (buffer ,sizeof buffer, "%H:%M:%S", timeinfo);
		ssd1306_center_string(buffer, 12, 2);

		// Display the actual date and temperature
		char date[20];
		strftime (date ,sizeof date, "%d/%m/%Y", timeinfo);
		sprintf(buffer, "%s - %.1f", date, temperature_now);
		ssd1306_center_string(buffer, 32-2, 1);
	}
	else
	{
		if (WiFi.status() != WL_CONNECTED)
			ssd1306_center_string("WiFi disconnect", 16);
		else
			ssd1306_center_string("sNTP Waiting", 16);
		ssd1306_center_string("- Syncing -", 32);
	}

	// Display the WSPR Call, Locator and Power in dBm
	sprintf(buffer, "%s/%s/%ddBm", config.call.c_str(), config.qth.c_str(), config.power);
	ssd1306_center_string(buffer, SCREEN_HEIGHT-12);

	if (wspr_symbol_index != 0)
	{
		auto y1 = display.height()-20-0;
		auto y2 = display.height()-20-2;
		display.drawLine(4, y1, slot_sec+4, y1, WHITE);
		display.drawLine(4, y2, slot_sec+4, y2, WHITE);
		display.invertDisplay(true);		// At TX invert the display collors
		next_tx_slot = slot_now;
	}
	else
	{
		// Calculate the next tx time and slot
		int	wait_sec = 120 - slot_sec;			// time in this slot
		for (auto x = 0; x < WSPR_SLOTS_HOUR; ++x)
		{
			next_tx_slot = (slot_now + x) % WSPR_SLOTS_HOUR;
			if (wspr_slot_type[next_tx_slot] != WSPR_TX_NONE) 
				break;
			wait_sec += 120;
		}
		if (wait_sec < 3600)
		{
			// Display time next tx
			sprintf(buffer, "TX at %02d:%02d", wait_sec / 60, wait_sec % 60);
			ssd1306_center_string(buffer, display.height()-20-4);
		}

		// Non TX normal display
		display.invertDisplay(false);
	}

	// Display the used freq bands
	sprintf(buffer, "%s/%s/%s"
			, wspr_slot_freq[next_tx_slot][0] == 0 ? "a" : String(wspr_slot_freq[next_tx_slot][0]/1000000).c_str()
			, wspr_slot_freq[next_tx_slot][1] == 0 ? "b" : String(wspr_slot_freq[next_tx_slot][1]/1000000).c_str()
			, wspr_slot_freq[next_tx_slot][2] == 0 ? "c" : String(wspr_slot_freq[next_tx_slot][2]/1000000).c_str()
			);

	int16_t   		x,y;
	uint16_t  		w,h;
//	display.getTextBounds(buffer, 0, 0, &x, &y, &w, &h);
	display.getTextBounds(buffer, 0, 1, &x, &y, &w, &h);
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
	display.getTextBounds(buf_count, 0, 1, &x, &y, &w, &h);

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
	LOG_I("Connected to %s (IP:%s/%s, GW:%s, RSSI %d)\n"
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

	delay(3000);
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

	if (txt1 != NULL)	ssd1306_center_string(txt1, 16);
	if (txt2 != NULL)	ssd1306_center_string(txt2, 16+12);
	if (txt3 != NULL)	ssd1306_center_string(txt3, 16+12+12);

	if (WiFi.SSID().length() > 0)
	{
		String ssid("SSID:"); ssid += WiFi.SSID();
		ssd1306_center_string(ssid.c_str(), 16+12+12+12);
	}

	ssd1306_display_on();
	display.display();

// #ifdef DEBUG_ESP_PORT
// 	LOG_I("SSD1306[[ ");
// 	if (txt1 != NULL)	LOG_I("%s",txt1);
// 	if (txt2 != NULL) {	LOG_I(" / "); LOG_I("%s",txt2); }
// 	if (txt3 != NULL) {	LOG_I(" / "); LOG_I("%s",txt3); }
// 	LOG_I(" ]](%dms)\n", wait);
// #endif

	if (wait >= 0)
		delay(wait);
}

void ssd1306_display_on()
{
	LOG_I("Display On for %dsec.\n", config.displayOff);

	timer_ms_display_auto_off = millis();		// Start the display ON timer
	display_status  = DISPLAY_ON;
}

void ssd1306_display_off()
{
	LOG_I("Display auto Off now.\n");

	display.clearDisplay();
	display.invertDisplay(false);
	display.display();
	display_status = DISPLAY_OFF;
}


/*
curl -s http://pe0fko.nl/wspr/id/client_6479671_NL.json
{	"chipid":8065906
,	"call":"PE0FKO"
,	"prefix":""
,	"suffix":""
,	"locatorXX":"JO32CD"
,	"power":"10"
,	"txenable":true
,	"hostname":"wsprtx"

,	"timezones":[
	{"start":0,"end":6,"clk":0,"list":"clk0N"},
	{"start":6,"end":18,"clk":0,"list":"clk0D"},
	{"start":18,"end":24,"clk":0,"list":"clk0N"},
	{"start":0,"end":24,"clk":1,"list":"clk1"},
	{"start":0,"end":24,"clk":2,"list":"clk2"}	]

,	"zones": [	[ 0,  6, 0, "clk0N"],
			[ 6, 18, 0, "clk0D"], 
			[18, 24, 0, "clk0N"],
			[ 0, 24, 1, "clk1"], 
			[ 0, 24, 2, "clk2"]
		 ]
,	"clk0D":[7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14]
,	"clk0N":[7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14]
,	"clk1":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
,	"clk2":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]

,	"wsprtype":[2,2,2, 2,2,2, 3,3,3, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2]
,	"rndseed":19561113
,	"displayoff":240
,	"freq_correction":-195
,	"temp_wspr_band":21
,	"temp_wspr_clk":0
,	"temp_correction":0.0
}
*/

/*
curl -s http://ipinfo.io/json|jq
{
  "ip": "89.99.108.64",
  "hostname": "89-99-108-64.cable.dynamic.v4.ziggo.nl",
  "city": "Zutphen",
  "region": "Gelderland",
  "country": "NL",
  "loc": "52.1383,6.2014",
  "org": "AS33915 Vodafone Libertel B.V.",
  "postal": "7201",
  "timezone": "Europe/Amsterdam",
  "readme": "https://ipinfo.io/missingauth"
}
*/

/*
curl -s http://api.sunrise-sunset.org/json?lat=52.1383\&lng=6.2014\&formatted=1\&tzid=Europe/Amsterdam|jq
{
  "results": {
    "sunrise": "6:39:54 AM",
    "sunset": "8:31:24 PM",
    "solar_noon": "1:35:39 PM",
    "day_length": "13:51:30",
    "civil_twilight_begin": "6:05:53 AM",
    "civil_twilight_end": "9:05:25 PM",
    "nautical_twilight_begin": "5:21:32 AM",
    "nautical_twilight_end": "9:49:47 PM",
    "astronomical_twilight_begin": "4:31:40 AM",
    "astronomical_twilight_end": "10:39:38 PM"
  },
  "status": "OK",
  "tzid": "Europe/Amsterdam"
}
*/

/*
curl -s http://pe0fko.nl/wspr/id/client_6479671_NL.json
{	"chipid":8065906
,	"call":"PE0FKO"
,	"prefix":""
,	"suffix":""
,	"locatorXX":"JO32CD"
,	"power":"10"
,	"txenable":true
,	"hostname":"wsprtx"

,	"zones": [	[ 0,  6, 0, "clk0N"],
			[ 6, 18, 0, "clk0D"], 
			[18, 24, 0, "clk0N"],
			[ 0, 24, 1, "clk1"], 
			[ 0, 24, 2, "clk2"]
		 ]
,	"clk0D":[7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14, 7,28,14]
,	"clk0N":[7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14, 7,3,14]
,	"clk1":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]
,	"clk2":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]

,	"wsprtype":[2,2,2, 2,2,2, 3,3,3, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2, 2,2,2]
,	"rndseed":19561113
,	"displayoff":240
,	"freq_correction":-195
,	"temp_wspr_band":21
,	"temp_wspr_clk":0
,	"temp_correction":0.0
}
*/
