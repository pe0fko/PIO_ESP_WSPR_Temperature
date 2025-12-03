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

/* ==== Example ====
{       "chipid":6479671
,       "call":"PE0FKO"
,       "prefix":""
,       "suffix":""
,       "locator":"JO32CD"
,       "power":"10"
,		"drive_strength":8
,       "hostname":"wsprtx"
,       "rndseed":19561113
,       "display_off":300
,       "freq_cal_factor":762
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
}

////////// New example //////////

{
  "system": {
	"version": "1.0.0",
  	"chipid": 8065906,
  	"hostname": "wsprtx",
  	"display_off": 60
  },

  "user": {
	"call": "PE0FKO",
	"prefix": "",
	"suffix": "",
	"locator": "JO32CD",
	"locator_FR": "JN13IW",
	"power": "10",
  },

  "wspr": {
    "enabled": true,
    "band_freq": 0,
    "tone_mul": 1,
    "drive_strength": 8
  }

  "si5351": {
  	"enable": true,
    "drive_strength": 8,
  	"calibration": -900,
  },

  "temperature": {
    "enable": true,
    "band": 10,
    "clk": 0,
  	"temp_correction": -4,
    "timezones": [
      {
        "start": "00:00",
        "end": "24:00",
        "clk": 0,
        "band": 10
      }
    ]
  },

  "timezones": [
    {
      "start": "0:0",
      "sunrise": "-1:0",
      "clk": 0,
      "list": "clk0",
      "name": "Nigth light zone"
    },
    {
      "sunrise": "-1:00",
      "sunset": "+1:00",
      "clk": 0,
      "list": "clk0",
      "name": "Day light zone"
    },
    {
      "sunset": "+1:0",
      "end": "24:00",
      "clk": 0,
      "list": "clk0",
      "name": "Night light zone"
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
  "clk0": [
    7
  ],
  "clk0D": [
    7,
    28,
    14,
    7,
    21,
    14
  ],
  "clk0N": [
    7,
    3,
    14
  ],
  "clk1": [
    0
  ],
  "clk2": [
    0
  ],

  "wsprtype": [
    { "slot": 3, "type": 3 },
    { "slot": 4, "type": 3 },
	{ "slot": 5, "type": 3 },
  ],
}

*/

// The defailt startup config
config_t config	=
{
	.user_call				= "PE0xxx",
	.user_prefix			= "",
	.user_suffix			= "",
	.user_locator			= "",
	.user_power				= 10,

	.system_version			= VERSION,
	.system_chipid			= ESP.getChipId(),
	.system_hostname		= String(HOSTNAME),
	.system_display_off		= 300,
	.system_randomSeed		= 1234567,

	.si5351_enable			= true,
	.si5351_xtal_freq		= 25000000,
	.si5351_calibration		= 0,
	.si5351_drive_strength	= 8,

	.wspr_enable			= false,
	.wspr_band_freq			= 0,
	.wspr_tone_mul			= 1.0,

	.temp_enable			= false,
	.temp_band				= 10,
	.temp_offset			= 0.0,
	.temp_clk				= SI5351_CLK0,

	.loc_lat_lon			= ""
};

			QTHLocator			QTH;										// Get the QTH locator from the internet
			JsonDocument		jsonDoc;									// Allocate the JSON document

			uint32_t			timer_us_one_second			= 0;			// micros()
static		uint32_t			timer_no_network			= 0;
static		uint32_t			timer_retry_config_load		= 0;
static		uint32_t			timer_ms_reboot				= 0;			// Timer for reboot

static		bool				sunrise_loaded				= false;		// Sunrise/Sunset loaded from the internet

uint8_t		hour_now;														// Time now in hour (0..23)
uint8_t		slot_now;														// Time now in slot (0..29)
uint8_t		slot_sec;														// Second in the slot (0..119)

static bool loadWebConfig(JsonDocument& doc);
static void jsonParseConfig(JsonDocument& jsonDoc);



//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup()
{
	Serial.begin(115200);
	Serial.setTimeout(2000);
	while(!Serial);
#ifdef DEBUG_ESP_PORT
	Serial.setDebugOutput(true);
#endif
	delay(1000);

	LOG_I("\n\n=== PE0FKO, TX WSPR temperature coded\n");
	LOG_I("=== Version: " VERSION ", Build at: " __DATE__ " " __TIME__ "\n");
	LOG_I("=== Config: %s - %s - %ddBm\n", config.user_call.c_str(), config.user_locator.c_str(), config.user_power);
	LOG_I("=== ChipId: %d, Hostname: %s.local\n", ESP.getChipId(), config.system_hostname.c_str() );
	LOG_I("=== SSD1306: Display %dx%d address:0x%02x\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH, SCREEN_ADDRESS);

	// Fast Fix OTA hostname setup
	config.system_hostname = String(HOSTNAME);
	// config.wspr_tone_mul = 1.0;		// Tone spacing normal

	setup_button();
	setup_display();						// Setup the SSD1306 display 128x32
	setup_ds18b20();						// Setup the temperature sensor
	setup_si5351();
	setup_wifi();
	setup_wspr_tx();

	LOG_I("=== Start looping...\n");
}

#if 0

bool loadWebConfig(JsonDocument& doc) 
{
#if 1
	String URL("https://pe0fko.nl/wspr/id/client_");	// HTTPS gives problems with certs
	URL += String(ESP.getChipId(), 10);
	URL += "_NL.json";
#else
	String URL("https://pe0fko.nl/wspr/id/prod_NL.json");	// HTTPS gives problems with certs
#endif

	Serial.printf("Free heap before HTTPS: %d\n", ESP.getFreeHeap());	

	HTTPClient http;
	WiFiClientSecure client;

	http.begin(client, URL);

	http.setReuse(false);	// Do not reuse the connection
	http.setTimeout(10000);	// Set the timeout to 10 seconds
	// http.setConnectTimeout(10000);	// Set the connect timeout to 10 seconds
	http.collectHeaders(nullptr, 0);  // Negeer alle headers
	http.setUserAgent("PE0FKO Lib");
	// http.setFollowRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS);	// Do not follow redirects
	// http.setRedirectLimit(0);	// Do not follow redirects
	// http.setUseHTTP10(true);	// Use HTTP/1.0 instead of HTTP/1.1
	// http.setAuthorization("username", "password");	// Set the username and password for basic authentication
	http.addHeader("Accept", "application/json");	// Set the Accept header to application/json
	// http.addHeader("Content-Type", "application/json");	// Set the Content-Type header to application/json
	// http.addHeader("User-Agent", "PE0FKO Lib QTHLocator");	// Set the User-Agent header to PE0FKO Lib QTHLocator

  	client.setInsecure();	// Skip certificate verificatie


	int httpCode = http.GET();
	if (httpCode == HTTP_CODE_OK) 
	{
		WiFiClient* stream = http.getStreamPtr();
		DeserializationError error = deserializeJson(doc, *stream);
		http.end();

		if (error) 
		{
			LOG_E("Error Json deserialize: %s\n", error.f_str());
			ssd1306_printf_P(5000, PSTR("JSON\nDeserialize\nError"));		// 5sec wait
			return false;
		}

		Serial.printf("Free heap after HTTPS: %d\n", ESP.getFreeHeap());

		return true;
	} else 
	{
		LOG_E("Error https GET, from: %s, HTTP code: %d\n", URL.c_str(), httpCode);
		ssd1306_printf_P(5000, PSTR("JSON\nERROR\nWebpage-Get"));	// 5sec wait
	}
  
	http.end();

	Serial.printf("Free heap after HTTPS error: %d\n", ESP.getFreeHeap());
	return false;
}

#elif 1

bool loadWebConfig(JsonDocument& doc) 
{
	// page.clear();
	// page.reserve(4096);		// Increase buffer size to 4kB.

	String URL("https://pe0fko.nl/wspr/id/client_");
	URL += String(ESP.getChipId(), 10);
	URL += "_NL.json";

	if (QTH.fetchWebPage(doc, URL) != HTTP_CODE_OK) 
	{
		LOG_E("Error loading web config, from: %s\n", URL.c_str());
		ssd1306_printf_P(1000, PSTR("JSON\nERROR\nWebpage-Load"));	// 1sec wait
		// ESP.restart();	// Restart ESP, no return
		// return false; --- IGNORE ---
		if (timer_ms_reboot == 0)
			timer_ms_reboot = millis() + 1000*60*5;		// Reboot in 5 min
		return false;
	}
	return true;
}
#else

bool loadJsonConfig(String& page)
{
	// page.clear();
	// page.reserve(4096);		// Increase buffer size to 4kB.

	String URL("https://pe0fko.nl/wspr/id/client_");
	URL += String(ESP.getChipId(), 10);
	URL += "_NL.json";

	if (QTH.fetchWebPage(page, URL) != HTTP_CODE_OK) 
	{
		LOG_E("Error loading web config, from: %s\n", URL.c_str());
		ssd1306_printf_P(1000, PSTR("JSON\nERROR\nWebpage-Load"));	// 1sec wait
		// ESP.restart();	// Restart ESP, no return
		// return false; --- IGNORE ---
		if (timer_ms_reboot == 0)
			timer_ms_reboot = millis() + 1000*60*5;		// Reboot in 5 min
		return false;
	}
	return true;
}
#endif

//---------------------------------------------------------------------------------
//---- JSON SET CONFIG....  JSON SET CONFIG....  JSON SET CONFIG....  JSON SET CONFIG....
//---------------------------------------------------------------------------------

#if 1

void jsonParseConfig(JsonDocument& jsonDoc)
{
	// LOG_I("JSON:"); serializeJson(jsonDoc, Serial);	LOG_I("\n");

	// Parse System configuratie
	JsonObject system = jsonDoc["system"];
	config.system_version			= system["version"]			| String("1.0.0");
	config.system_chipid			= system["chipid"]			| ESP.getChipId();
	config.system_hostname			= system["hostname"]		| String(HOSTNAME);
	config.system_display_off		= system["display_off"]		| 120;
	config.system_randomSeed		= system["random_seed"]		| 19570215;

	// Parse SI5351 configuratie
	JsonObject si5351 = jsonDoc["si5351"];
	config.si5351_enable			= si5351["enable"]			| true;
	config.si5351_drive_strength	= si5351["drive_strength"]	| 8;		// Drive strength: 2,4,6 or 8 (mA)   SI5351_CRYSTAL_LOAD_8PF
	config.si5351_calibration		= si5351["calibration"]		| 0.0;
	config.si5351_xtal_freq			= si5351["xtal_freq"]		| SI5351_XTAL_FREQ;

	// Parse User configuratie
	JsonObject user = jsonDoc["user"];
	config.user_call				= user["call"]				| String("PE0xxx");
	config.user_prefix				= user["prefix"]			| String("");
	config.user_suffix				= user["suffix"]			| String("");
	config.user_locator				= user["locator"]			| String("");
	config.user_power				= user["power"]				| 10;

	// Parse WSPR configuratie
	JsonObject wspr = jsonDoc["wspr"];
	config.wspr_enable				= wspr["enable"]			| true;
	config.wspr_band_freq			= wspr["band_freq"]			| 0.00;	// 0.00 = random
	config.wspr_tone_mul			= wspr["test_tone_mul"]		| 1.0;	// 1.0 = normal

	// Parse Temperature configuratie
	JsonObject temp = jsonDoc["temperature"];
	config.temp_enable				= temp["enable"]			| false;
	config.temp_band				= temp["band"]				| WSPR_TX_NONE;
	config.temp_clk					= temp["clk"]				| SI5351_CLK0;
	config.temp_offset				= temp["offset"]			| 0.0;

	if (config.si5351_drive_strength < 2 || config.si5351_drive_strength > 8)
		config.si5351_drive_strength = 8;
	if (config.si5351_xtal_freq < 10000000 || config.si5351_xtal_freq > 40000000)
		config.si5351_xtal_freq = SI5351_XTAL_FREQ;
	if (config.user_locator.length() != 6 && config.user_locator.length() != 4)
		config.user_locator = "JO32";
	if (config.user_power < 0 || config.user_power > 50)
		config.user_power = 10;
	if (config.wspr_band_freq < 0.0 || config.wspr_band_freq > 200.0)
		config.wspr_band_freq = 0.0;
	if (config.wspr_tone_mul < 1.0 || config.wspr_tone_mul > 4.0)
		config.wspr_tone_mul = 1.0;

	// Add the slash char if needed in prefix & suffix
	if (config.user_prefix.length() != 0 && config.user_prefix.indexOf('/') == -1) 
		config.user_prefix += '/';
	if (config.user_suffix.length() != 0 && config.user_suffix.indexOf('/') == -1) 
		config.user_suffix = "/" + config.user_suffix;

	if (config.wspr_band_freq < 20.0 || config.wspr_band_freq > 180.0)
		config.wspr_band_freq = 0.0;
	if (config.wspr_tone_mul < 1.0 || config.wspr_tone_mul > 4.0)
		config.wspr_tone_mul = 1.0;

	// Display the config data
	//=========================================================
	LOG_I("INIT: Wspr call sign   		: %s\n",			config.user_call.c_str());
	LOG_I("INIT: Wspr call prefix 		: %s\n",			config.user_prefix.c_str());
	LOG_I("INIT: Wspr call suffix 		: %s\n",			config.user_suffix.c_str());
	LOG_I("INIT: Wspr qth locator 		: %s\n",			config.user_locator.c_str());
	LOG_I("INIT: Wspr TX power    		: %ddBm\n",			config.user_power);

	LOG_I("INIT: Wspr TX hostname		: %s.local\n",		config.system_hostname.c_str());
	LOG_I("INIT: Display auto off 		: %d sec\n",		config.system_display_off);

	LOG_I("INIT: Si5351 enable  		: %d\n",			config.si5351_enable);
	LOG_I("INIT: Si5351 xtal freq		: %d Hz\n",			config.si5351_xtal_freq);
	LOG_I("INIT: Freq call factor		: %ld ppm\n", 			config.si5351_calibration);
	LOG_I("INIT: SI5351 drive strength	: %d mA\n",			config.si5351_drive_strength);	// mA

	LOG_I("INIT: WSPR TX Enable			: %d\n",			config.wspr_enable);
	LOG_I("INIT: WSPR TX Band Freq		: %.2f Hz\n",		config.wspr_band_freq);
	LOG_I("INIT: WSPR TX Tone Multiply	: %f\n",			config.wspr_tone_mul);

	LOG_I("INIT: Temp TX Enable			: %d\n",			config.temp_enable);
	LOG_I("INIT: Temp sensor offset		: %.1f °C\n",		config.temp_offset);

	// Set the frequency correction value
	si5351_clockgen.set_correction(config.si5351_calibration, si5351_pll_input::SI5351_PLL_INPUT_XO);

	if ( ! config.system_hostname.isEmpty() )
	{
		// LOG_I("INIT: hostname=%s\n", config.hostname.c_str());
		WiFi.setHostname(config.system_hostname.c_str());					// Set WiFi Hostname.
		MDNS.setHostname(config.system_hostname.c_str());					// Set mDNS hostname (for next setting)
	}

	setup_wspr_tx();	// Setup the WSPR TX parameters for the wspr_tone_mul
}

#else

bool jsonSetConfig(String jsonString)
{
	// JsonDocument		jsonDoc;									// Allocate the JSON document
	jsonDoc.clear();

	DeserializationError error =  deserializeJson(jsonDoc, jsonString);
	if (error != DeserializationError::Ok) {
		LOG_E("Error Json Seserialize: %s\n", error.f_str());
		ssd1306_printf_P(1000, PSTR("JSON\nFormat\nERROR"));	// 1sec wait
		if (timer_ms_reboot == 0)
			timer_ms_reboot = millis() + 1000*60*5;					// Reboot in 5 min
		return false;
	}

	// LOG_I("JSON:"); serializeJson(jsonDoc, Serial);	LOG_I("\n");

	// Parse System configuratie
	JsonObject system = jsonDoc["system"];
	config.system_version			= system["version"]			| String("1.0.0");
	config.system_chipid			= system["chipid"]			| ESP.getChipId();
	config.system_hostname			= system["hostname"]		| String(HOSTNAME);
	config.system_display_off		= system["display_off"]		| 120;
	config.system_randomSeed		= system["random_seed"]		| 19570215;

	// Parse SI5351 configuratie
	JsonObject si5351 = jsonDoc["si5351"];
	config.si5351_enable			= si5351["enable"]			| true;
	config.si5351_drive_strength	= si5351["drive_strength"]	| 8;		// Drive strength: 2,4,6 or 8 (mA)   SI5351_CRYSTAL_LOAD_8PF
	config.si5351_calibration		= si5351["calibration"]		| 0.0;
	config.si5351_xtal_freq			= si5351["xtal_freq"]		| SI5351_XTAL_FREQ;

	// Parse User configuratie
	JsonObject user = jsonDoc["user"];
	config.user_call				= user["call"]				| String("PE0xxx");
	config.user_prefix				= user["prefix"]			| String("");
	config.user_suffix				= user["suffix"]			| String("");
	config.user_locator				= user["locator"]			| String("JO32");
	config.user_power				= user["power"]				| 10;

	// Parse WSPR configuratie
	JsonObject wspr = jsonDoc["wspr"];
	config.wspr_enable				= wspr["enable"]			| true;
	config.wspr_band_freq			= wspr["band_freq"]			| 0.00;	// 0.00 = random
	config.wspr_tone_mul			= wspr["test_tone_mul"]		| 1.0;	// 1.0 = normal

	// Parse Temperature configuratie
	JsonObject temp = jsonDoc["temperature"];
	config.temp_enable				= temp["enable"]			| false;
	config.temp_band				= temp["band"]				| WSPR_TX_NONE;
	config.temp_clk					= temp["clk"]				| SI5351_CLK0;
	config.temp_offset				= temp["offset"]			| 0.0;

	// Add the slash char if needed in prefix & suffix
	if (config.user_prefix.length() != 0 && config.user_prefix.indexOf('/') == -1) 
		config.user_prefix += '/';
	if (config.user_suffix.length() != 0 && config.user_suffix.indexOf('/') == -1) 
		config.user_suffix = "/" + config.user_suffix;
// power limits
	if (config.wspr_band_freq < 20.0 || config.wspr_band_freq > 180.0)
		config.wspr_band_freq = 0.0;
	if (config.wspr_tone_mul < 1.0 || config.wspr_tone_mul > 4.0)
		config.wspr_tone_mul = 1.0;

	// Display the config data
	//=========================================================
	LOG_I("INIT: Wspr call sign   		: %s\n",			config.user_call.c_str());
	LOG_I("INIT: Wspr call prefix 		: %s\n",			config.user_prefix.c_str());
	LOG_I("INIT: Wspr call suffix 		: %s\n",			config.user_suffix.c_str());
	LOG_I("INIT: Wspr qth locator 		: %s\n",			config.user_locator.c_str());
	LOG_I("INIT: Wspr TX power    		: %ddBm\n",			config.user_power);

	LOG_I("INIT: Wspr TX hostname		: %s.local\n",		config.system_hostname.c_str());
	LOG_I("INIT: Display auto off 		: %d sec\n",		config.system_display_off);

	LOG_I("INIT: Si5351 enable  		: %d\n",			config.si5351_enable);
	LOG_I("INIT: Si5351 xtal freq		: %d Hz\n",			config.si5351_xtal_freq);
	LOG_I("INIT: Freq call factor		: %ld ppm\n", 			config.si5351_calibration);
	LOG_I("INIT: SI5351 drive strength	: %d mA\n",			config.si5351_drive_strength);	// mA

	LOG_I("INIT: WSPR TX Enable			: %d\n",			config.wspr_enable);
	LOG_I("INIT: WSPR TX Band Freq		: %.2f Hz\n",		config.wspr_band_freq);
	LOG_I("INIT: WSPR TX Tone Multiply	: %f\n",			config.wspr_tone_mul);

	LOG_I("INIT: Temp TX Enable			: %d\n",			config.temp_enable);
	LOG_I("INIT: Temp sensor offset		: %.1f °C\n",		config.temp_offset);

	// Set the frequency correction value
	si5351_clockgen.set_correction(config.si5351_calibration, si5351_pll_input::SI5351_PLL_INPUT_XO);

	if ( ! config.system_hostname.isEmpty() )
	{
		// LOG_I("INIT: hostname=%s\n", config.hostname.c_str());
		WiFi.setHostname(config.system_hostname.c_str());					// Set WiFi Hostname.
		MDNS.setHostname(config.system_hostname.c_str());					// Set mDNS hostname (for next setting)
	}

	setup_wspr_tx();	// Setup the WSPR TX parameters for the wspr_tone_mul

	return true;
}
#endif

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

#define	TW	200

void loop()
{
	enum	state_t {	sIdle, sWaitWifiConnect, sWifiConnect, sWaitOnTimeReceived,
						sLoadSunRise, sLoadLocation, sLoadConfigJSon, sWaitConfigReload, 
						sLoadIpLocation, sMakeSlotPlan, sSetOneSecondTick, sDefaultLoop, 
						sSlotSecond_0, sSlotSecond_111, sWifiDisconnect };
	static	state_t	state = sIdle;

	if (state >= sWifiConnect && !semaphore_wifi_connected) { 
		state = sWifiDisconnect;
	}

 	switch (state) 
	{
		case sIdle:
			init_wifi();								// Initialize the Wifi for the NTP,OTA service 
			ssd1306_printf_P(TW, PSTR("WiFi\n\nStarting"));
			timer_no_network = millis();
			state = sWaitWifiConnect;
			break;


		case sWaitWifiConnect:
		{
			// uint32_t delay = millis() - timer_no_network;

			// Still no network, then reboot after some time
			// if (delay >= value_no_network) {
			if ((millis() - timer_no_network) >= value_no_network) {
				LOG_F("REBOOT: No NTP time received.\n");
				ssd1306_printf_P(10 * 1000, PSTR("**REBOOT**\nNo NTP sync"));
				ESP.restart();
			}

			if (semaphore_wifi_connected) {
				ssd1306_printf_P(TW, PSTR("WiFi\nConnected\n%s"), WiFi.SSID().c_str());
				LOG_I("WiFi Connected SSID %s, RSSI %d)\n"
					, WiFi.SSID().c_str()
					, WiFi.RSSI());
				state = sWifiConnect;
			}

			// Niet met WiFiMulti, die wacht in run()
			// if ((delay % 16) == 0) {
			// 	ssd1306_printf_P(TW, PSTR("WiFi\nStarting\n%d"), delay);
			// }
		}	break;


		case sWifiConnect:
			if (semaphore_wifi_ip_address) {
				LOG_I("DHCP IP:%s/%s, GW:%s\n"
					, WiFi.localIP().toString().c_str()
					, WiFi.subnetMask().toString().c_str()
					, WiFi.gatewayIP().toString().c_str() );

				ssd1306_printf_P(1000, PSTR("IP:%s\nGW:%s\nRSSI:%d"),
					WiFi.localIP().toString().c_str(),
					WiFi.gatewayIP().toString().c_str(),
					WiFi.RSSI() );

				state = sWaitOnTimeReceived;
			}
			break;


		case sWaitOnTimeReceived:
			if (semaphore_wifi_ntp_received) {
				ssd1306_printf_P(TW, PSTR("WiFi\nsNTP Time\nReceived"));
				state = sLoadConfigJSon;
			}
			break;


#if 0
		case sLoadConfigJSon:
		{	String page;
			page.reserve(4096);		// Increase buffer size to 4kB.

			setSlotTime();						// Get the current time and slot
			ssd1306_printf_P(TW, PSTR("Web API\nLoad\nID:%d"), ESP.getChipId() );

			if (loadJsonConfig(page)) {				// Load the config data
				// LOG_I("JSON Config: %s\n", page.c_str());
				if (jsonSetConfig(page)) {
					state = sLoadIpLocation;
					break;
				} else {
					ssd1306_printf_P(2000, PSTR("Web API\nJSON\nError") );
				}
			} else {
				ssd1306_printf_P(2000, PSTR("Web API\nLOAD\nError") );
			}

			// Try sometime later to load the config
			timer_retry_config_load = millis();
			state = sWaitConfigReload;

		}	break;
#else
		case sLoadConfigJSon:
			Serial.printf("Free heap before loadConfigJSon: %d\n", ESP.getFreeHeap());	// TEST

			setSlotTime();						// Get the current time and slot
	
			ssd1306_printf_P(TW, PSTR("Web API\nLoad\nID:%d"), ESP.getChipId() );
			if (loadWebConfig(jsonDoc)) 				// Load the config data
			{
				jsonParseConfig(jsonDoc);				// Decode the json config data
				state = sLoadIpLocation;
			} else {
				ssd1306_printf_P(2000, PSTR("Web API\nLOAD\nError") );

				// Try sometime later to load the config again
				timer_retry_config_load = millis();
				state = sWaitConfigReload;
			}	

			Serial.printf("Free heap after loadConfigJSon: %d\n", ESP.getFreeHeap());	// TEST
			break;
#endif

		case sWaitConfigReload:
			if ((millis() - timer_retry_config_load) > 30*1000)	{	// Wait 30 seconds
				state = sLoadConfigJSon;
			}
			// // Add the reboot timer
			// if (timer_ms_reboot != 0 && millis() >= timer_ms_reboot) {
			// 	LOG_F("REBOOT: After JSON config load error.\n");
			// 	ssd1306_printf_P(10 * 1000, PSTR("**REBOOT**\nAfter JSON Error"));
			// 	ESP.restart();
			// }
			break;


		case sLoadIpLocation:
			// If no location in the json config file, then try to get
			// the lat,lon from the internet IP address. That is done 
			// by the API "https://ipinfo.io/json".
			if (config.user_locator.length() < 4) {
				if (QTH.load_api_location())
				{
					config.loc_lat_lon = QTH.getLoc();
					config.user_locator = QTH.latLonToMaidenhead(config.loc_lat_lon);
					LOG_I("Find QTH by IP address: %s, %s\n", config.loc_lat_lon.c_str(), config.user_locator.c_str());
				}
				//TODO: Check on error state qth,loc_lat_lon
			} else {
				config.loc_lat_lon = QTH.MaidenheadTolatLon(config.user_locator);
				LOG_I("Set location by QTH:%s to location:%s\n", config.user_locator.c_str(), config.loc_lat_lon.c_str());
			}
			state = sLoadSunRise;
			break;


		case sLoadSunRise:
			setSlotTime();								// Get the current time and slot
			if (! sunrise_loaded 
			&&	QTH.load_api_sunrise(config.loc_lat_lon, hour_now == 23 ? "tomorrow" : "today" )) 
			{
				sunrise_loaded = true;
			}
			state = sMakeSlotPlan;
			break;


		case sMakeSlotPlan:
			setSlotTime();								// Get the current time and slot
			ssd1306_printf_P(TW, PSTR("Make\nSlot Plan"));
			makeSlotPlan();								// Make a plan for the this TX hour


			// DEBUG: Test the power usage of the WiFi module
			// stop_wifi();							// Stop the WiFi, we don't need it anymore
			// DEBUG:


			state = sSetOneSecondTick;
			break;


		case sSetOneSecondTick:
		// Start the one second timer on the exaxt second tick.
		// This is done only once, so the timer is not reset in the loop.
		{	struct timeval tv;
			gettimeofday(&tv, NULL);					// Get the current time in sec and usec

			// tv_usec should be "This is the rest of the elapsed time (a fraction of a second), 
			// represented as the number of microseconds. It is always less than one million."
			timer_us_one_second = micros();				// Initialize the timer only ones!
			timer_us_one_second -= tv.tv_usec;			// Correct the us to the sec tick

			state = sDefaultLoop;
		}	break;


		case sDefaultLoop:
			// Check the 1 second timer tick.
			if ((micros() - timer_us_one_second) >= value_us_one_second)
			{
				timer_us_one_second += value_us_one_second;

				setSlotTime();							// Get the current time and slot

				if (slot_sec == 0) {					// First second of the 2 minute interval clock
					state = sSlotSecond_0;
				} else
				if (slot_sec == wspr_free_second) {		// Actions needed in the free time after a wspr tx (> 111sec, 9sec time available!)
					state = sSlotSecond_111;
				} else
				{
					ssd1306_main_window();				// Show the (second) clock on display
				}
			}
			break;


		case sSlotSecond_0:
		{
			//++ At every 2 minute interval start a WSPR message, if slot is richt.
			switch (wspr_slot_type[slot_now]) {
			default:
			case WSPR_TX_NONE:							// No WSPR TX now
				break;
			case WSPR_TX_TYPE_1:						// Type 1 message: CALL, LOC4, dBm
				wspr_tx_init(config.user_call.c_str());
				break;
			case WSPR_TX_TYPE_2:						// Type 2 message: pre/CALL/suff, dBm
				wspr_tx_init((config.user_prefix + config.user_call + config.user_suffix).c_str());
				break;
			case WSPR_TX_TYPE_3:						// Type 3 message: *hash* <pre/CALL/suff>, LOC6, dBm
				wspr_tx_init(('<' + config.user_prefix + config.user_call + config.user_suffix + '>').c_str());
				break;
			}

			ssd1306_main_window();						// Show the (second) clock on display
			state = sDefaultLoop;
		}	break;


		case sSlotSecond_111:							// wspr_free_second	= 8192.0 / 12000.0 * WSPR_SYMBOL_COUNT + 1.0
		{
			state = sDefaultLoop;

			if (slot_now == 29)							// Last slot in the hour (0...29)
			{
				if (!WiFi.isConnected()) 
				{
					LOG_W("WiFi not connected at hour change, restart WiFi\n");
					ssd1306_printf_P(2000, PSTR("WiFi\nNot\nConnected"));
					stop_wifi();
					init_wifi();
					state = sWaitWifiConnect;
				} else {
					LOG_I("WiFi connected at hour change\n");
					state = sLoadConfigJSon;				// Reload the config (.json) from the webserver every hour
				}

				if (hour_now == 23)						// Last slot of the day to do some houskeeping
				{

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


					sunrise_loaded = false;
					state = sLoadSunRise;				// Reload the sunrise and sunset time from a webserver

					LOG_I("Set the const ramdom seed number 0x%08x\n", config.system_randomSeed);
					randomSeed(config.system_randomSeed);		// Set the random seed for the next day
				}
			}

			// Only read the temp once every 2min
			if (display_status == DISPLAY_ON) 
				readTemperature();

			ssd1306_main_window();						// Show the (second) clock on display
		}	break;

		case sWifiDisconnect:
			ssd1306_printf_P(2000, PSTR("WiFi disconnect\nRestart WiFi"));

			//TODO: Clear the user config to force a new load from the webserver
			// config.user_locator.clear();
			config.loc_lat_lon.clear();

			WiFi.disconnect(true);						// Disconnect from WiFi and reset the WiFi module

			state = sIdle;
			break;

		default:
			break;
	}

	loop_wspr_tx();
	loop_wifi();
	loop_display();
	loop_button();
	loop_ds18b20();

	// Check if reboot is requested
	if (timer_ms_reboot != 0) {
		if (millis() > timer_ms_reboot) {
			LOG_I("REBOOT: Reboot requested by user/software.\n");
			ssd1306_printf_P(10 * 1000, PSTR("**REBOOT**\nBy User"));
			ESP.restart();
		}
	}
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