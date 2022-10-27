// *********************************************
// WSPR Clock TX ESP8266.
// 01/05/2021 Fred Krom, pe0fko
//
// Board: WeMos D1 R1 ESP8266
//
// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2	
// "PE0FKO JN13 10"
// SYMBOL: 3,1,2,2,2,2,0,2,3,2,2,0,3,3,3,0,0,0,1,0,0,3,0,1,1,3,3,2,0,0,0,2,2,0,1,0,0,3,2,3,2,0,0,2,2,0,3,0,1,1,2,2,1,3,0,3,0,0,2,3,1,0,1,0,0,0,0,3,1,2,1,0,3,0,3,2,1,0,0,1,0,2,1,0,3,3,0,0,2,1,1,2,3,2,1,2,0,2,1,0,0,0,0,2,1,0,2,3,2,0,1,3,1,2,3,1,2,2,1,3,2,3,0,0,0,1,1,1,2,2,2,2,2,3,0,1,2,0,3,3,0,0,0,2,2,0,2,3,1,2,3,0,1,3,2,0,0,3,3,2,2,0,


#define   LOC_PE0FKO
//#define   LOC_PA_PE0FKO
//#define   LOC_LA_MOTHE

#define   DEBUGGING
#define   FEATURE_OTA
#define   FEATURE_mDNS
//#define   FEATURE_CARRIER
#define	  EEPROM_VERSION  1
#define	  EEPROM_INDEX	  0


#if defined LOC_PE0FKO
#define   HAM_CALL        "PE0FKO"        	// Ham radio call sign
#define   HAM_PREFIX      ""				// Prefix of the ham call
#define   HAM_SUFFIX      ""				// Prefix of the ham call
#define   HAM_LOCATOR     "JO32cd"			// JO32CD40OJ
#define   HAM_POWER       7					// Power TX in dBm
#elif defined LOC_PA_PE0FKO
#define   HAM_CALL        "PE0FKO"			// Ham radio call sign
#define   HAM_PREFIX      "PA/"				// Prefix of the ham call
#define   HAM_SUFFIX      ""				// Prefix of the ham call
#define   HAM_LOCATOR     "JO32cd"			// JO32CD40OJ
#define   HAM_POWER       7					// Power TX in dBm
#elif defined LOC_LA_MOTHE
#define   HAM_CALL        "PE0FKO"			// Ham radio call sign
#define   HAM_PREFIX      "F/"				// Prefix of the ham call
#define   HAM_SUFFIX      ""				// Prefix of the ham call
#define   HAM_LOCATOR     "JN13"			// JN13IW08UG
#define   HAM_POWER       7					// Power TX in dBm
#else
#error    "Specify the Location..."
#endif

//#define	WSPR_TX_FREQ		1838000UL	// 160m  1.838000 -  1.838200
//#define	WSPR_TX_FREQ		3570000UL	// 80m   3.570000 -  3.570200
//#define	WSPR_TX_FREQ		5288600UL	// 60m   5.288600 -  5.288800
#define	WSPR_TX_FREQ		7040000UL	// 40m   7.040000 -  7.040200
//#define	WSPR_TX_FREQ		10140100UL	// 30m  10.140100 - 10.140300
//#define	WSPR_TX_FREQ		14097000UL	// 20m  14.097000 - 14.097200
//#define	WSPR_TX_FREQ		18106000UL	// 17m  18.106000 - 18.106200
//#define	WSPR_TX_FREQ		21096000UL	// 15m  21.096000 - 21.096200
//#define	WSPR_TX_FREQ		24926000UL	// 12m  24.926000 - 24.926200
//#define	WSPR_TX_FREQ		28126000UL	// 10m  28.126000 - 28.126200
//#define	WSPR_TX_FREQ		50294400UL	// 6m   50.294400 - 50.294600
//#define	WSPR_TX_FREQ		144489900UL	// 2m  144.489900 - 144.490100


#define   SI5351_FREQ_CORRECTION_01   (13126UL)
#define   SI5351_FREQ_CORRECTION_02   (116000UL)

#define		HOSTNAME          	"WSPR-TX"
#define		TEMP_CORRECTION   	0.0           // Change at 18/08/2022
#define		WSPR_DELAY        	683           // Delay value for WSPR

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
#include <WiFiManager.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <si5351.h>           // Etherkit
#include <JTEncode.h>         // Etherkit
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
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#ifdef DEBUGGING
#define PRINTF(...)		{ Serial.printf(__VA_ARGS__); }
#define PRINTVAR(...)	{ Serial.printf(#__VA_ARGS__"="); Serial.print (__VA_ARGS__); Serial.print("\n"); }
#else
#define PRINTF(...)		{}
#define PRINTVAR(...)	{}
#endif


ESP8266WiFiMulti		wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
boolean connectioWasAlive = true;		// ESP8266WiFiMulti
WiFiManager				  wifiManager;
Adafruit_SSD1306		display(-1);
Si5351					    si5351;
JTEncode				    wspr;

OneWire				      oneWire(ONE_WIRE_BUS);
DallasTemperature	  sensors(&oneWire);

// Variables and constants for timeouts
const   uint32_t    value_20ms_loop			 = 20;   // 20ms
static  uint32_t    timer_20ms_loop;
const   uint32_t    value_wspr_bit_ms		= WSPR_DELAY;
static  uint32_t    timer_wspr_bit_ms;
const   uint32_t    value_display_auto_off	= (600UL * 1000UL);    // 10 min
static  uint32_t    timer_display_auto_off;

volatile bool		ntp_time_sync			= false;
char 				PASSWORD[32]			= "12345678";	// Will use the CPU ID for the password!
float				temperature_now			= 0.0;

uint8_t				wspr_symbols[WSPR_SYMBOL_COUNT];

uint8_t				wspr_symbol_index		= 0;
uint16_t			wspr_tx_counter        	= 0;

enum {        		WSPR_TX_NONE, WSPR_TX_CALL, WSPR_TX_HASH_CALL   };
uint8_t				wspr_slot_tx[WSPR_SLOTS_MAX];     // 0=None, 1=CALL, 2=F/CALL, 3=<F/CALL>
uint8_t				wspr_slot_band[WSPR_SLOTS_MAX];
uint64_t			wspr_frequency;

#define TONE_SPACING(N)           ((uint16_t)(12000.0/8192.0 * N * SI5351_FREQ_MULT + 0.5))
const uint16_t      wspr_sym_freq[4] = { TONE_SPACING(0), TONE_SPACING(1), TONE_SPACING(2), TONE_SPACING(3) };


static  uint8_t   switchStatusLast  = HIGH;  // last status hardware switch
enum {		DISPLAY_OFF,	DISPLAY_ON,	DISPLAY_NOTHING	};
static  uint8_t   display_switch_status   = DISPLAY_ON;


//
// Make a plan to TX in one of the 30 slots (2min inteval in a hour).
//
void make_slot_plan(bool setup)
{
    // Clean the old slot plan.
    memset(wspr_slot_tx, WSPR_TX_NONE, WSPR_SLOTS_MAX);
    memset(wspr_slot_band, 100, WSPR_SLOTS_MAX);
    
#if 1
	if (setup)
	{
		//
		// Every odd slot a TX until the first hour.
		//
		for (int i = 0; i < WSPR_SLOTS_MAX; i += 3)
		{
		  wspr_slot_tx[i+0] = WSPR_TX_CALL;
		  wspr_slot_tx[i+1] = WSPR_TX_HASH_CALL;
		  wspr_slot_tx[i+2] = WSPR_TX_NONE;

		  wspr_slot_band[i+0] = 100-10;
		  wspr_slot_band[i+1] = 100+10;
		}
	}
//	else
#endif
	{
		int   s0,s1,s2,t;
		float tf;

		sensors.requestTemperatures(); 
		tf = sensors.getTempCByIndex(0) + TEMP_CORRECTION;
		PRINTF("Slot Temperature %.1fºC\n", tf);

		//Convert temperature to integer value
		// >= -10 --- <70 ==> 0 -- 70 = *10 = 700
		tf += 20.0;								//== Negative start offset
		if (tf < 0.0) tf = 0.0;
		if (tf > (70.0-0.1)) tf = 70.0-0.1;
		tf *= 10;								//== Decimal steps
		t = (int)(tf + 0.5);					// Rounding
//		PRINTF("Code message temp: %03d\n", t);

		s0 = 2 + t / 100;       // 4..18 min
		t %= 100;
		s1 = 10 + t / 10;       // 
		t %= 10;
		s2 = 20 + t;
//		PRINTF("TX Slots: %d, %d, %d\n", s0, s1, s2);
//		PRINTF("TX minut: %d, %d, %d\n", s0*2, s1*2, s2*2);

		wspr_slot_tx[s0+0] = WSPR_TX_CALL;
		wspr_slot_tx[s0+1] = WSPR_TX_HASH_CALL;
		wspr_slot_tx[s1]   = WSPR_TX_CALL;
		wspr_slot_tx[s2]   = WSPR_TX_CALL;

		wspr_slot_band[s0+0]  = 50;
		wspr_slot_band[s0+1]  = 50;
		wspr_slot_band[s1]    = random(20, 180);
		wspr_slot_band[s2]    = random(20, 180);
	}

#if 1
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
void setup() 
{
	randomSeed(0x1502);

	Serial.begin(74880);				// 115200 74880
//	Serial.setTimeout(2000);
	while(!Serial) yield();
	delay(2000);
	Serial.setDebugOutput(true);

	WiFi.disconnect();	// Debug!! Testing delete all wifi profiles onboard
	WiFi.softAPdisconnect(true);	// TEST TEST TEST TEST TEST TEST TEST TEST
	WiFi.hostname(HOSTNAME);

  Serial.printf("DEBUG SSD1306: %d x %d\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH);

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//	display.setRotation(2);				// Display upside down...

	pinMode(BUTTON_INPUT, INPUT);		// Button for display on/off
	timer_display_auto_off = millis();	// Start the display ON timer

	pinMode(LED_BUILTIN, OUTPUT);		// BuildIn LED

	PRINTF("\nPE0FKO, WSPR Clock, Build: %s %s\n", __TIME__, __DATE__);
	PRINTF("Config: frequency %fMHz - " HAM_PREFIX HAM_CALL HAM_SUFFIX " - " HAM_LOCATOR " - %ddBm\n", WSPR_TX_FREQ/1000000.0, HAM_POWER);


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

	sntp_stop();
	settimeofday_cb(time_is_set);	// Call-back function
//	sntp_set_time_sync_notification_cb(time_is_set);

	// Google NTP servers
	configTime("CET-1CEST,M3.5.0/2,M10.5.0/3",
				"216.239.35.0", "216.239.35.4", "time1.google.com");

	// Try to startup the WiFi connection
	ssd1306_text(200, "WiFiMulti Init");
	wifiMulti.addAP("pe0fko_guest", "Welkom-114");
	wifiMulti.addAP("pe0fko_ziggo", "NetwerkBeheer114");
	wifiMulti.addAP("pe0fko_4g", "NETWERKBEHEER");
	wifiMulti.addAP("pe0fko", "NETWERKBEHEER");

	PRINTF("Connecting .");
	int i = 0;
	while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
		delay(400);
		PRINTF(".");
	}
	PRINTF("\n");

	// Keep WiFi connected
	WiFi.setAutoReconnect(true);
	WiFi.persistent(true);

	// Tell us what network we're connected to
	PRINTF("Connected to %s (IP:%s, RSSI %d)\n"
		, WiFi.SSID().c_str()              
		, WiFi.localIP().toString().c_str()           // Send the IP address of the ESP8266 to the computer
		, WiFi.RSSI());
  
	ssd1306_WiFi();

	// autoconfiguration portal for wifi settings
	ssd1306_text(200, "WiFi Manager");
	wifiManager.setConfigPortalTimeout(5*60);
	wifiManager.setDebugOutput(true);
	wifiManager.setWiFiAPChannel(9);
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

#ifdef FEATURE_mDNS
	init_mdns();
#endif

#ifdef FEATURE_OTA
	init_ota();
#endif

#if 1
	init_si5351();
#endif

	sensors.begin();
	sensors.requestTemperatures();
	temperature_now = sensors.getTempCByIndex(0) + TEMP_CORRECTION;  
	PRINTF("Sensor temperature %.1fºC\n", temperature_now);

	make_slot_plan(true);

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
}

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
	ArduinoOTA.begin();
}

#endif


#if 1
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
		Serial.println(F("SI5351 Initialized"));
	}
	else
	{
		ssd1306_text(200, "Si5351 ERROR");
		Serial.println(F("SI5351 not found on I2C bus!"));
	}
}
#endif


// callback routine - arrive here whenever a successful NTP update has occurred
void time_is_set (void)
{
	time_t now = time(nullptr);          // get UNIX timestamp 

	Serial.print("NTP update done at: ");
	Serial.print(ctime(&now));

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


//		ESP.restart();

	// If still no WiFi then try again later...
	if (wifiMulti.run() != WL_CONNECTED) 
	{
		PRINTF("WiFi Disconnected.....\n");
//		delay(200);
		return;
	}

	// Wait for the NTP time service is known!
	if (!ntp_time_sync) 
	{
		PRINTF("No NTP sync...\n");
		time(nullptr);
		delay(500);
		sntp_init();
		return;
		//TODO: Reboot after some time
	}

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
				char const *call = NULL;

				if (wspr_slot == 0)
					make_slot_plan(false);

				// Calc the SI5351 frequency setting.
				wspr_frequency  = SI5351_FREQ_MULT * (uint64_t)(WSPR_TX_FREQ);
				wspr_frequency += SI5351_FREQ_MULT * wspr_slot_band[wspr_slot];

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_CALL) 
					call = HAM_PREFIX HAM_CALL HAM_SUFFIX;

				if (wspr_slot_tx[wspr_slot] == WSPR_TX_HASH_CALL) 
					call = "<" HAM_PREFIX HAM_CALL HAM_SUFFIX ">";
				
				if (call != NULL) 
				{
					PRINTF("WSPR Hour slot %d, band %d, type WSPR_TX_CALL: %s\n", wspr_slot, wspr_slot_band[wspr_slot], call);
					wspr.wspr_encode(call, HAM_LOCATOR, HAM_POWER, wspr_symbols);
					timer_wspr_bit_ms = millis();
					wspr_bit_tx();
				}
			}

			//
			//++ At avery 2 minute interval, second 60, print the temperature
			//
			if (sec2min == 60)
			{
				sensors.requestTemperatures(); 
				temperature_now = sensors.getTempCByIndex(0) + TEMP_CORRECTION;  
				PRINTF("Actual Temerature %.1fºC\n", temperature_now);
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
  if (wspr_symbol_index != WSPR_SYMBOL_COUNT) 
  {
    if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], SI5351_CLK0))
    {
      // Error !
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

void ssd1306_text(uint8_t delay_ms, const char* txt)
{
	ssd1306_background();

	display.setTextSize(1);
	uint16_t  w = getFontStringWidth(txt);
	display.setCursor((display.width() - w) / 2 , 16);
	display.print(txt);

//	display.setCursor(8, 52);
//	display.print(WiFi.SSID());              // Tell us what network we're connected to

	display.display();
	if (delay_ms > 0)
		delay(delay_ms);
}

void ssd1306_WiFi()
{
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
