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

#include "header.h"
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20

/* ==== Example ====
{       "chipid":6479671
,       "call":"PE0FKO"
,       "prefix":""
,       "suffix":""
,       "locator":"JO32CD"
,       "power":"10"
,       "hostname":"wsprtx"
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
,		"wspr_tx_enabled":true
,       "temp_tx_enabled":true
}
*/

// The defailt startup config
config_t config	=
{	.call="PE0xxx",
	.prefix="",
	.suffix="",
	.qth="JO32",
	.power=10,
	.freq_cal_factor=0,
	.randomSeed=1234567,
	.hostname=HOSTNAME,
	.displayOff=300,
	.temp_cor=0.0,
	.wspr_enabled=false,
	.temp_enabled=false,
};

			QTHLocator			QTH;										// Get the QTH locator from the internet
			JsonDocument		jsonDoc;									// Allocate the JSON document

static		uint32_t			timer_ms_20ms_loop		= 0;
			uint32_t			timer_us_one_second		= 0;				// micros()
static		uint32_t			timer_no_network		= 0;

uint8_t		hour_now;														// Time now in hour (0..23)
uint8_t		slot_now;														// Time now in slot (0..29)
uint8_t		slot_sec;														// Second in the slot (0..119)

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup()
{
	// pinMode(LED_BUILTIN, OUTPUT);				// LED on the ESP
	// digitalWrite(LED_BUILTIN, HIGH);			// High is off!
	// pinMode(BUTTON_INPUT, INPUT_PULLUP);		// Button for display on/off

	Serial.begin(115200);
	Serial.setTimeout(2000);
	while(!Serial);
#ifdef DEBUG_ESP_PORT
	Serial.setDebugOutput(true);
#endif
	delay(1000);

	LOG_I("=== PE0FKO, TX WSPR temperature coded\n");
	LOG_I("=== Version: " VERSION ", Build at: " __DATE__ " " __TIME__ "\n");
	LOG_I("=== Config: %s - %s - %ddBm\n", config.call.c_str(), config.qth.c_str(), config.power);
	LOG_I("=== ChipId: %d, Hostname: %s.local\n", ESP.getChipId(), config.hostname.c_str() );
	LOG_I("=== SSD1306: Display %dx%d address:0x%02x\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH, SCREEN_ADDRESS);

	setup_display();						// Setup the SSD1306 display 128x32
	setup_ds18b20();						// Setup the temperature sensor
	setup_si5351();
	setup_wifi();

	LOG_I("=== Start looping...\n");
}

bool loadWebConfigData()
{
	String URL("https://pe0fko.nl/wspr/id/client_");
	URL += String(ESP.getChipId(), 10);
	URL += "_NL.json";

	String page;
	if (QTH.fetchWebPage(page, URL) == HTTP_CODE_OK) 
	{
		jsonSetConfig(page);
	} else 
	{
		LOG_E("Error loading web config, from: %s\n", URL.c_str());
		ssd1306_printf_P(5*60*1000, PSTR("JSON\nERROR\nWebLoad"));	// 5min wait
		// ESP.restart();		//<</////////////////////////////////////////////////////////////
		return false;
	}
	return true;
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

	config.call				= jsonDoc["call"]				| "PE0xxx";
	config.prefix			= jsonDoc["prefix"]				| "";
	config.suffix			= jsonDoc["suffix"]				| "";
	config.hostname			= jsonDoc["hostname"]			| HOSTNAME;
	config.power			= jsonDoc["power"]				| 10;
	config.freq_cal_factor	= jsonDoc["freq_correction"]	| 0UL;
	config.randomSeed		= jsonDoc["rndseed"]			| 19570215;
	config.displayOff		= jsonDoc["displayoff"]			| 120;
	config.temp_cor			= jsonDoc["temp_correction"]	| 0.0;
	config.wspr_enabled		= jsonDoc["wspr_tx_enabled"]	| true;
	config.temp_enabled		= jsonDoc["temp_tx_enabled"]	| true;


	// Add the slash char if needed in prefix & suffix
	if (config.prefix.length() != 0 && config.prefix.indexOf('/') == -1) config.prefix += '/';
	if (config.suffix.length() != 0 && config.suffix.indexOf('/') == -1) config.suffix = "/" + config.suffix;

	// LOG_I("*********  prefix/siffix: -%s- -%s-\n", config.prefix.c_str(), config.suffix.c_str());

	if (jsonDoc["locator"].is<const char*>()) 
	{
		config.qth = jsonDoc["locator"]	| "JO32";
		LOG_I("QTH locator: by config file, %s\n", config.qth.c_str());
	}
	else
	{
		config.qth = QTH.getQthLoc();
		LOG_I("QTH locator: by IP address config, %s\n", config.qth.c_str());
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
	LOG_I("INIT: Temp sensor corre	: %.1f\n",			config.temp_cor);
	LOG_I("INIT: WSPR TX Enabled	: %d\n",			config.wspr_enabled);
	LOG_I("INIT: Temp TX Enabled	: %d\n",			config.temp_enabled);

	// Set the frequency correction value
	si5351.set_correction(config.freq_cal_factor, si5351_pll_input::SI5351_PLL_INPUT_XO);

	if ( ! config.hostname.isEmpty() )
	{
		// LOG_I("INIT: hostname=%s\n", config.hostname.c_str());
		WiFi.setHostname(config.hostname.c_str());					// Set WiFi Hostname.
		MDNS.setHostname(config.hostname.c_str());					// Set mDNS hostname (for next setting)
	}
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

// #define	TW	10
#define	TW	1000

void loop()
{
	enum	state_t {	sIdle, sWaitWifiConnect, sWifiConnect, sWifiIpAddress, sWifiNtpTimeSet,
						sGetQTHLocator, sLoadConfig, sWifiDefaultLoop, sWifiDisconnect };
	static	state_t	state = sIdle;

	switch (state) 
	{
		case sIdle:
			init_wifi();								// Initialize the Wifi for the NTP,OTA service 
			ssd1306_printf_P(TW, PSTR("WiFi\nStarting"));
			timer_no_network = millis();
			state = sWaitWifiConnect;
			break;

		case sWaitWifiConnect:
		{
			uint32_t delay = millis() - timer_no_network;

			// Still no network, then reboot after some time
			if (delay >= value_no_network) {
				LOG_F("REBOOT: No NTP time received.\n");
				ssd1306_printf_P(10 * 1000, PSTR("**REBOOT**\nNo NTP sync"));
				ESP.restart();
			}

			if (semaphore_wifi_connected) {
				ssd1306_printf_P(TW, PSTR("WiFi\nConnected"));
				state = sWifiConnect;
			}

			// Niet met WiFiMulti, die wacht in run()
			// if ((delay % 16) == 0) {
			// 	ssd1306_printf_P(TW, PSTR("WiFi\nStarting\n%d"), delay);
			// }
		}	break;

		case sWifiConnect:
			if (semaphore_wifi_ip_address) {
				// ssd1306_wifi_page();
				ssd1306_printf_P(4000, PSTR("IP:%s\nGW:%s\nRSSI:%d"),
					WiFi.localIP().toString().c_str(),
					WiFi.gatewayIP().toString().c_str(),
					WiFi.RSSI() );
				state = sWifiIpAddress;
			}
			break;

		case sWifiIpAddress:
			if (semaphore_wifi_ntp_received) {
				ssd1306_printf_P(TW, PSTR("WiFi\nNTP Time"));		//TODO:
				state = sWifiNtpTimeSet;
			}
			break;

		case sWifiNtpTimeSet:
			if (!semaphore_wifi_connected) { state = sWifiDisconnect; break; }

			setSlotTime();						// Get the current time and slot
			ssd1306_printf_P(TW, PSTR("Web API\nLoad\nSun rise/set"));

			state = sGetQTHLocator;
			if (!QTH.begin())					// Get the locator and sunrise/sunset API
			{
				LOG_E("Error loading QTH locator\n");
				ssd1306_printf_P(10*1000, PSTR("Error loading QTH"));
				state = sWifiDefaultLoop;		//TODO: Error state!
			}
			break;

		case sGetQTHLocator:
			// ssd1306_printf_P(4000, PSTR("QTH locator\n%s\n%s"), QTH.getQthLoc().c_str(), QTH.getCity().c_str() );

			if (QTH.getQthLoc() != config.qth)
			{
				ssd1306_printf_P(4000, PSTR("QTH locator\n%s\n%s"), QTH.getQthLoc().c_str(), QTH.getCity().c_str() );
				// config.qth = QTH.getQthLoc();	//TODO: Check
				LOG_I("New QTH locator: %s (@%s)\n", config.qth.c_str(), QTH.getCity().c_str());
			}
			state = sLoadConfig;
			break;

		case sLoadConfig:
			if (!semaphore_wifi_connected) { state = sWifiDisconnect; break; }

			ssd1306_printf_P(TW, PSTR("Web API\nLoad\nID:%d"), ESP.getChipId() );
			if (loadWebConfigData())				// Load the config data
				makeSlotPlan();						// Make a plan for the this TX hour

			state = sWifiDefaultLoop;
			break;

		case sWifiDefaultLoop:
			if (!semaphore_wifi_connected) { state = sWifiDisconnect; break; }

			loop_1s_tick();
			loop_20ms_tick();
			// loop_ds18b20();								// Empty

			break;

		case sWifiDisconnect:
			ssd1306_printf_P(1000, PSTR("WiFi disconnect\nRestart WiFi"));
			state = sIdle;
			break;

		default:
			break;
	}

	loop_wspr_tx();
	loop_display();
	loop_wifi();
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





	// LOG_I("timer_us_one_second=%d\n", timer_us_one_second)





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
			LOG_I("WSPR-Time: Hour:%u Slot:%u, CALL=%s, QTH=%s, Freq=%d/%d/%d(+%d)Hz\n", 
				hour_now, slot_now, 
				Call.c_str(), 
				config.qth.c_str(),
				wspr_slot_freq[slot_now][0],
				wspr_slot_freq[slot_now][1],
				wspr_slot_freq[slot_now][2],
				wspr_slot_band[slot_now] );
		}
	}

	// Actions needed in the free time after a wspr tx (> 111sec, 9sec time!)
	if (slot_sec == wspr_free_second) 
	{
		if (slot_now == 29)		// Last slot of the hour
		{
			if (hour_now == 23)	// Last slot of the day
			{
				// Get the sunrise/sunset times for tomorrow ()
				QTH.api_sunrise("tomorrow");	// Get the sunrise/sunset data from the web

				LOG_I("Set the const ramdom seed number 0x%08x\n", config.randomSeed);
				randomSeed(config.randomSeed);
			}

			// First load the Json config data from webserver
			if (loadWebConfigData())
				makeSlotPlan();			// Make a plan for the next TX hour
		}

		// Only read the temp once every 2min
		if (display_status == DISPLAY_ON) 
			readTemperature();
	}

	ssd1306_main_window();
}


// Used for slower processing, timing from the cpu xtal
void loop_20ms_tick() 
{
	static uint8_t switchStatusLast = HIGH;				// last status hardware switch

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
				readTemperature();							// Get temperature
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

/*
https://pe0fko.nl/wspr/id/client_6479671_NL.json
{
  "chipid": 6479671,
  "call": "PE0FKO",
  "prefix": "",
  "suffix": "F",
  "locator": "JN13IW",
  "power": "10",
  "wspr_tx_enable": true,
  "hostname": "wsprtx",
  "displayoff": 240,
  "freq_correction": -195,
  "timezones": [
    {
      "start": "0:0",
      "sunrise": "-1:0",
      "clk": 0,
      "list": "clk0N",
      "name": "Nigth light zone"
    },
    {
      "sunrise": "-1:00",
      "sunset": "+2:00",
      "clk": 0,
      "list": "clk0D",
      "name": "Day light zone"
    },
    {
      "sunset": "2:0",
      "end": "24:00",
      "clk": 0,
      "list": "clk0N",
      "name": "Day light zone"
    },
    {
      "start": "00:00",
      "end": "24:00",
      "clk": 1,
      "list": "clk1",
      "name": "Freq list for clk1"
    },
    {
      "start": "00:00",
      "end": "24:00",
      "clk": 2,
      "list": "clk2",
      "name": "Freq list for clk2"
    }
  ],
  "clk0D": [7, 28, 14],
  "clk0N": [7, 3, 14],
  "clk1": [0],
  "clk2": [0],
  "wsprtype": [2, 2, 2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
  "temp_correction": -2
   "temperature": {
    "band": 21,
    "clk": 0,
    "enable": true
  }
}
*/