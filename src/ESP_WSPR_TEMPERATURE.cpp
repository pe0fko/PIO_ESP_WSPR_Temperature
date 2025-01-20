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

#define		VERSION		"V3.1"

//#define	LOC_PE0FKO_40m
//#define	LOC_PE0FKO_40m_1
//#define	LOC_PA_PE0FKO
//#define	LOC_PE0FKO_NR
//#define	LOC_LA_MOTHE_40m
//#define	LOC_LA_MOTHE_30m
//#define	LOC_LA_MOTHE_20m

// Defined in PlatformIO .ini file
//#define		FEATURE_OTA
//#define		FEATURE_mDNS
//#define		FEATURE_CARRIER
//#define		FEATURE_1H_FAST_TX
//#define		FEATURE_PRINT_TIMESLOT
//#define		FEATURE_PRINT_WSPR_SIMBOLS
//#define		FEATURE_CHECK_TIMING

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
 #define PRINTF_P(F,...)	{ DEBUG_ESP_PORT.printf_P(PSTR(F), ##__VA_ARGS__); }
#else
 #define PRINT(S)			{ }
 #define PRINT_P(...)		{ }
 #define PRINTF_P(...)		{ }
#endif

#define	SCREEN_WIDTH	128			// OLED display width, in pixels
#define	SCREEN_HEIGHT	64			// OLED display height, in pixels

#define OLED_RESET     -1 	// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C	///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306			display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Si5351						si5351;
JTEncode					wspr;

static	ESP8266WiFiMulti	wifiMulti;
static 	WiFiEventHandler	wifiConnectHandler;						// WiFi connect event handler
static	WiFiEventHandler	wifiDisconnectHandler;					// WiFi disconnect event handler
static	String				HostName;
static	OneWire				oneWire(ONE_WIRE_BUS);					// Temp sensor
static	DallasTemperature	sensors(&oneWire);						// Dallas DS18B20

//static	uint8_t				wifi_status_previous	= WL_DISCONNECTED;

//static esp8266::polledTimeout::periodicMs showTimeNow(60000);  	// Checkout

const   uint32_t		value_ms_20ms_loop		= 20;				// 20ms interval check time ntp sec
static  uint32_t		timer_ms_20ms_loop		= 0;
const	uint32_t		value_us_wspr_bit		= 8192.0 / 12000.0 * 1000000.0;	 // Delay value for WSPR
static	uint32_t		timer_us_wspr_bit		= 0;
const	uint32_t		value_us_one_second		= 1000000UL;		// micro second (us)
static	uint32_t		timer_us_one_second		= 0;				// micros()
static	uint32_t		value_ms_display_auto_off;					// Display on time, load per chip
static	uint32_t		timer_ms_display_auto_off;
const	uint32_t		value_ms_ntp_faild_reboot= 30 * 60 * 1000;	// 1/2 hour (ntp must be init updated in this time)
static	uint32_t		timer_ms_ntp_faild_reboot;
const   uint32_t		value_ms_led_blink_on	= 3141UL;			// 4sec interval blink led
static  uint32_t		timer_ms_led_blink_on	= 0;
const   uint32_t		value_ms_led_blink_off	= 3UL;				// 3ms led on
static  uint32_t		timer_ms_led_blink_off	= 0;

const	uint32_t		sntp_update_delay		= 3600 * 1000UL;	// NTP update every 1h
volatile bool			ntp_time_sync_first		= false;			// First time, no Unix Epoch
volatile bool			ntp_time_sync			= false;
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

const	uint32_t		wspr_free_second		= 8192.0 / 12000.0 * WSPR_SYMBOL_COUNT + 1.0;

const	int32_t      	wspr_sym_freq[4] =
{	static_cast<uint32_t> ( 0.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 1.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 2.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 3.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
};

static	struct {	int 	ChipId;					// ESP Chip ID
					int 	FreqCorrection;			// Si5351 frequency correction, PPB
					int 	RandomSeed;				// Daily pseudo random number, freq
					int		DisplayAutoOff;			// Switch display off timeout
					String	Hostname;				// mDNS & OTA hostname
					float	TempCorrection;			// DS18B20 temp correction, at 18/08/2022
				} ESPChipInfo[] 
=
//{	{ 0x7b06f7, 0,			0x19570215,	1*60000, "WsprTX", 	-3.7 }	// Arduino shield, TCXO, Old D1 board
{	{ 0x7b1372, -195,		0x19570215,	1*60000, "WsprTX", 	-3.7 }	// Arduino shield, TCXO
,	{ 0x62df37, 620+142,	0x19561113, 5*60000, "WsprTST",	-1.0 }	// Breadboard, TCXO
,	{ -1, 		0,			0X5555,		1*60000, "WsprESP",  0.0 }	// Default
};

static		int		CHIP_FREQ_CORRECTION;
static		int		CHIP_RANDOM_SEED;
static		int		CHIP_DISPLAY_AUTO_OFF;;
static		String	CHIP_HOSTNAME;
static		float	CHIP_TEMP_CORRECTION;

// Forward reference
void		ReadTemperature();
void		ssd1306_display_on();
void		ssd1306_display_off();
void		ssd1306_text(uint16_t delay_ms, const char* txt1, const char* txt2=NULL);
void		ssd1306_printf_P(int wait, PGM_P formatP, ...);
void		ssd1306_wifi_page();
void		onWifiConnect(const WiFiEventStationModeGotIP& ipInfo);
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
void		loop_1s_tick_wspr(uint8_t hour, uint8_t slot, uint8_t slot_sec);
void		loop_1s_tick_clock(uint8_t slot_sec);
void		wspr_tx_init(const char* call);
void		wspr_tx_disable(si5351_clock clk);
void		wspr_tx_freq(si5351_clock clk);
void		wspr_tx_enable(si5351_clock clk);
void		ssd1306_background();
uint16_t	getFontStringWidth(const String& str);

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

//		PRINTF_P("TX Slots: loc[%d], temp[%d, %d, %d]\n", s0, s1, s2, s3);
		PRINTF_P("TX Slots: temperature code [%d, %d, %d]\n", s1, s2, s3);

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
			PRINTF_P("\t%02d:WSPR%d-[%d,%d,%d]+%d\n", i, wspr_slot_tx[i], wspr_slot_freq[i][0], wspr_slot_freq[i][1], wspr_slot_freq[i][2], wspr_slot_band[i]);
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
	value_ms_display_auto_off = CHIP_DISPLAY_AUTO_OFF;
	HostName = CHIP_HOSTNAME;

	ntp_time_sync_first = false;
	ntp_time_sync = false;

	Serial.begin(115200);				// 115200
	Serial.setTimeout(2000);
	while(!Serial) yield();
#ifdef DEBUG_ESP_PORT
	Serial.setDebugOutput(true);
#endif
	delay(1000);

	PRINT_P ("\n=== PE0FKO, TX WSPR temperature coded\n");
	PRINT_P ("=== Version: " VERSION ", Build at: " __DATE__ " " __TIME__ "\n");
	PRINTF_P("=== Config: " HAM_PREFIX HAM_CALL HAM_SUFFIX " - " HAM_LOCATOR " - %ddBm\n", HAM_POWER);
	PRINTF_P("=== ChipId: 0x%x, Host:%s, TempCor=%f\n", chipid, CHIP_HOSTNAME.c_str(), CHIP_TEMP_CORRECTION);

	PRINTF_P("Sizeof int      : %d\n", sizeof(int));
	PRINTF_P("Sizeof long     : %d\n", sizeof(long));
	PRINTF_P("Sizeof long long: %d\n", sizeof(long long));
	PRINTF_P("value_us_wspr_bit: %ld\n", value_us_wspr_bit);

	PRINTF_P("SSD1306: %dx%d addr:0x%02x\n", SSD1306_LCDHEIGHT, SSD1306_LCDWIDTH, SCREEN_ADDRESS);
	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);

//	display.setRotation(2);						// Display upside down...

	pinMode(BUTTON_INPUT, INPUT_PULLUP);		// Button for display on/off
	ssd1306_display_on();						// Start the display ON timer

	pinMode(LED_BUILTIN, OUTPUT);				// BuildIn LED

	ssd1306_printf_P(800, PSTR("Hostname\n%s\n.local"), HostName.c_str());

	// WiFi settings
	WiFi.mode(WIFI_STA);						// Set WiFi to station mode
	WiFi.setHostname(HostName.c_str());			// Set Hostname.
//	wifi_station_set_hostname(HostName.c_str());
	WiFi.setAutoReconnect(true);				// Keep WiFi connected

	// Register WiFi event handlers
//	WiFi.onEvent(onWifiEvent);
	wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
	wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

	// Try to startup the WiFi Multi connection with the strongest AP found.
	for(i = 0; i < WifiApListNumber; i++)
		wifiMulti.addAP(WifiApList[i].ssid, WifiApList[i].passwd);

#ifdef FEATURE_mDNS
	init_mdns();						// Init the broadcast DNS server (.local)
#endif

#ifdef FEATURE_OTA
	// Start OTA server.
	ArduinoOTA.setHostname((const char *)HostName.c_str());
	ArduinoOTA.onStart([]() { ssd1306_printf_P(100, PSTR("OTA update\nRunning")); });
	ArduinoOTA.onEnd([]()   { ssd1306_printf_P(100, PSTR("OTA update\nReboot")); ESP.restart(); });
	ArduinoOTA.setPassword(OtaPassword);
	ArduinoOTA.begin();
#endif

	sensors.begin();					// Init the onewire for the DS18B20 temp sensor
	ReadTemperature();					// Read the Dallas temperature one-wire sensor

	init_si5351();						// Init the frequency generator SI5351

	make_slot_plan(true);				// The first hour slot plan definition

	struct timeval	tv;
	gettimeofday(&tv, NULL);			// Get the current time in usec
	PRINT(ctime(&tv.tv_sec));			// convert timestamp and display

	pinMode(LED_BUILTIN, OUTPUT);		// LED on the ESP
	digitalWrite(LED_BUILTIN, HIGH);	// High is off!

#ifdef FEATURE_CARRIER
  PRINTF_P("CW Carrier on: %fMHz (CLK%d)\n", (float)CARRIER_FREQUENCY/1e6, CARRIER_SI5351_CLK);
  si5351.set_freq( SI5351_FREQ_MULT * CARRIER_FREQUENCY, CARRIER_SI5351_CLK );
  wspr_tx_enable(CARRIER_SI5351_CLK);
#endif

	ssd1306_printf_P(300, PSTR("Start\nLooping"));
	PRINT_P("=== Start looping...\n");

	ssd1306_main_window();
}


//void onWifiConnect(const WiFiEventStationModeConnected& ssid)
//{
//	PRINTF_P("WiFi connected: SSID %s\n", ssid.ssid.c_str());
//}

void onWifiConnect(const WiFiEventStationModeGotIP& ipInfo)
{
	ntp_time_sync = false;
	settimeofday_cb(sntp_time_is_set);			// Call-back function
	configTime(MYTZ, "time.google.com","nl.pool.ntp.org");
	sntp_init();

//	timer_ms_ntp_faild_reboot = millis();

	PRINTF_P("WiFi connected: IP:%s/%s GW:%s\n",
		ipInfo.ip.toString().c_str(),
		ipInfo.mask.toString().c_str(),
		ipInfo.gw.toString().c_str()
		);

}

// callback routine - arrive here whenever a successful NTP update has occurred
void sntp_time_is_set(bool from_sntp)
{
	ntp_time_sync_first = true;		// There is some time, not the Unix Epoch.
	ntp_time_sync = true;			// and the new time is set

	time_t now = time(nullptr);          // get UNIX timestamp
	PRINT_P("NTP update done at: ");
	PRINT(ctime(&now));
}


void onWifiDisconnect(const WiFiEventStationModeDisconnected& disconnectInfo)
{
	sntp_stop();
//	timer_ms_ntp_faild_reboot = millis();

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
	ssd1306_printf_P(200, PSTR("mDNS\n%s"), HostName.c_str());

	if (MDNS.begin(HostName.c_str()))
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
	loop_wspr_tx();
	loop_1s_tick();
	loop_20ms_tick();
	loop_wifi_tick();
	loop_led_tick();

	#ifdef FEATURE_OTA
	ArduinoOTA.handle();
	#endif

	#ifdef FEATURE_mDNS
	MDNS.update();
	#endif

	// Wait for the NTP time service is known!
	if (!ntp_time_sync_first)
	{
		if ((millis() - timer_ms_ntp_faild_reboot) >= value_ms_ntp_faild_reboot)
		{
			ssd1306_printf_P(200, PSTR("REBOOT\nNTP sync"));
			PRINT_P("REBOOT: No NTP time received.\n");
			Serial.flush();
			ESP.restart();
		}

		// Maak langere loops met timer
		ssd1306_printf_P(200, PSTR("Waiting\nNTP first sync"));
	}
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
	if ((micros() - timer_us_one_second) >= value_us_one_second)
	{
		if (timer_us_one_second == 0)							// Initialize the timer only ones!
			timer_us_one_second = micros();

		struct timeval tv;
		gettimeofday(&tv, NULL);								// Get the current time in usec
		if (tv.tv_usec > 500000L)								// Over the half second, wait for second end
		{
			timer_us_one_second +=								// Set the (usec=1000000) timer for one second clock
				1000000UL - tv.tv_usec;							// Still need to wait the last part of us
		}
		else
		{	// On time or to late, both run the sec loop.

			timer_us_one_second +=	value_us_one_second - tv.tv_usec;
//			Serial.printf("Time: %6u: %s", tv.tv_usec, ctime(&tv.tv_sec));

			uint8_t hour		= tv.tv_sec % (3600 * 24)	/ 3600;
			uint8_t slot		= tv.tv_sec % (3600) 		/ 120;
			uint8_t slot_sec	= tv.tv_sec % (120);

			if (ntp_time_sync_first)
				loop_1s_tick_wspr(hour, slot, slot_sec);

			loop_1s_tick_clock(slot_sec);

			//++ Set the random seed ones every day.
			//   Posible track of (semi) random numbers!
			if (hour == 23 && slot == 29 && slot_sec == wspr_free_second)
			{
				PRINTF_P("Set the const ramdom seed number 0x%08x\n", CHIP_RANDOM_SEED);
				randomSeed(CHIP_RANDOM_SEED);
			}

		}
	}
}


// Used for slower processing, timing from the cpu xtal
void loop_20ms_tick() 
{
	if ((millis() - timer_ms_20ms_loop) >= value_ms_20ms_loop)	// Every 20ms...
	{
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

				PRINTF_P("Button pressed, display_status=%d\n", display_status);
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
	}   		// 50Hz code lus
}

void loop_wifi_tick() 
{
	if (wspr_symbol_index == 0)
	{
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

void loop_1s_tick_wspr(uint8_t hour, uint8_t slot, uint8_t slot_sec)
{
	//
	//++ At every 2 minute interval start a WSPR message, if slot is richt.
	//
	if (slot_sec == 0)											// First second of the 2 minute interval clock
	{
		wspr_slot = slot;

		if (wspr_slot == 0)
			make_slot_plan(false);

		Serial.printf("WSPR-Time: %2u:%02u:%03u\n", hour, wspr_slot, slot_sec);

		if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_1)			// Type 1 message: CALL, LOC4, dBm
			wspr_tx_init(HAM_CALL);

		else if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_2)		// Type 2 message: pre/CALL/suff, dBm
			wspr_tx_init(HAM_PREFIX HAM_CALL HAM_SUFFIX);

		else if (wspr_slot_tx[wspr_slot] == WSPR_TX_TYPE_3)		// Type 3 message: hash <pre/CALL/suff>, LOC6, dBm
			wspr_tx_init("<" HAM_PREFIX HAM_CALL HAM_SUFFIX ">");
	}
}

void loop_1s_tick_clock(uint8_t slot_sec)
{
	// Only read the temp once every 2min
	if (display_status == DISPLAY_ON && slot_sec == wspr_free_second) 
		ReadTemperature();

	ssd1306_main_window();
}

static void init_si5351()
{
	PRINTF_P("SI5351 init: xtal:%d, correction:%d\n", SI5351_XTAL_FREQ, CHIP_FREQ_CORRECTION);

	ssd1306_printf_P(200, PSTR("SI5351\nSetup"));

	// TCXO input to xtal pin 1?
	if ( si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, CHIP_FREQ_CORRECTION) )
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
		PRINTF_P("WSPR TX, the SI5351 is not ready, 0x%02x\n", reg_val);
		return false;
	}

	return true;
}


void wspr_tx_init(const char* call)
{
	if (si5351_ready())
	{
		PRINTF_P("WSPR TX with Call: %s, Loc:%s, Power:%ddBm\n", call, HAM_LOCATOR, HAM_POWER);
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

		PRINTF_P("TX WSPR %d Ended.\n", wspr_tx_counter);
	}
}

void wspr_tx_freq(si5351_clock clk)
{
	if (wspr_slot_tx[wspr_slot] != WSPR_TX_NONE && 
		wspr_slot_freq[wspr_slot][clk] != 0)
	{
		uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[wspr_slot][clk] + wspr_slot_band[wspr_slot]);
		if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) )
			PRINTF_P("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
	}
}

void wspr_tx_enable(si5351_clock clk)
{
	if (wspr_slot_tx[wspr_slot] != WSPR_TX_NONE && 
		wspr_slot_freq[wspr_slot][clk] != 0)
	{
		PRINTF_P("TX WSPR start %d: slot %d, freq %.6fMHz + %dHz\n", 
				clk, wspr_slot, 
				wspr_slot_freq[wspr_slot][clk] / 1000000.0, 
				wspr_slot_band[wspr_slot]);

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
	struct tm*		timeinfo;
	int16_t   		x,y;
	uint16_t  		w,h;
	int				ns = wspr_slot;	// next slot


	if (display_status == DISPLAY_OFF)
		return;

	if ((millis() - timer_ms_display_auto_off) >= value_ms_display_auto_off)
	{
		ssd1306_display_off();
		return;
	}

	gettimeofday(&tv, NULL);									// Get the current time in usec

	ssd1306_background();

	if (ntp_time_sync)
	{
		timeinfo = localtime (&tv.tv_sec);
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
	}
	else
	{
		if (WiFi.status() != WL_CONNECTED)
			strcpy(buffer, "WiFi disconnect");
		else
			strcpy(buffer, "sNTP Waiting");

		display.setTextSize(1);
		w = getFontStringWidth(buffer);
		display.setCursor((display.width() - w) / 2 , 16+2);
		display.print(buffer);

		strcpy(buffer, "- Syncing -");

		display.setTextSize(1);
		w = getFontStringWidth(buffer);
		display.setCursor((display.width() - w) / 2 , 32-2);
		display.print(buffer);
	}

	sprintf(buffer, "%s/%s/%ddBm", HAM_CALL, HAM_LOCATOR, HAM_POWER);
	w = getFontStringWidth(buffer);
	display.setCursor((display.width()- w)/2, display.height()-10);
	display.print(buffer);

	if (wspr_symbol_index != 0)
	{
		uint16_t B,E;
		B = 4;
		E = B + tv.tv_sec % 120;
		display.drawLine(B, display.height()-14, E, display.height()-14, WHITE);
		display.drawLine(B, display.height()-16, E, display.height()-16, WHITE);

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
			sprintf(buffer, "TX %02d:%02d", w/60, w%60);
			display.setCursor(14, display.height()-20);
			display.print(buffer);
		}

		// The actual temperature used
		sprintf(buffer, "%.1f", temperature_now);
		w = getFontStringWidth(buffer);
		display.setCursor(display.width() - 14 - w, display.height()-20);
		display.print(buffer);

		// Non TX normal display
		display.invertDisplay(false);
	}

	// Display the used freq bands
	sprintf(buffer, "%s/%s/%s"
			, wspr_slot_freq[ns][0] == 0 ? "x" : String(wspr_slot_freq[ns][0]/1000000).c_str()
			, wspr_slot_freq[ns][1] == 0 ? "x" : String(wspr_slot_freq[ns][1]/1000000).c_str()
			, wspr_slot_freq[ns][2] == 0 ? "x" : String(wspr_slot_freq[ns][2]/1000000).c_str()

//			, wspr_slot_freq[ns][0] == 0 ? "x" : String(300/(wspr_slot_freq[ns][0]/1000000)).c_str()
//			, wspr_slot_freq[ns][1] == 0 ? "x" : String(300/(wspr_slot_freq[ns][1]/1000000)).c_str()
//			, wspr_slot_freq[ns][2] == 0 ? "x" : String(300/(wspr_slot_freq[ns][2]/1000000)).c_str()
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

void ssd1306_wifi_page()
{
	PRINTF_P("Connected to %s (IP:%s/%s, GW:%s, RSSI %d)\n"
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
		display.setCursor((display.width() - w) / 2 , 16+12);	// 16+16
		display.print(txt2);
	}
	if (txt3 != NULL)
	{
		uint16_t  w = getFontStringWidth(txt3);
		display.setCursor((display.width() - w) / 2 , 16+12+12);
		display.print(txt3);
	}

	if (WiFi.SSID().length() > 0)
	{
		String ssid("SSID:"); ssid += WiFi.SSID();
		uint16_t  w = getFontStringWidth(ssid);
		display.setCursor((display.width() - w) / 2 , 52);
		display.print(ssid);
	}

	ssd1306_display_on();
	display.display();

#ifdef DEBUG_ESP_PORT
	PRINTF_P("ssd1306_text[%d]: ", wait);
	if (txt1 != NULL)	PRINT(txt1);
	if (txt2 != NULL) {	PRINT_P(" / "); PRINT(txt2); }
	if (txt3 != NULL) {	PRINT_P(" / "); PRINT(txt3); }
	PRINT_P("\n");
#endif

	if (wait >= 0)
		delay(wait);
}

void ssd1306_display_on()
{
	PRINTF_P("Display On for %dsec.\n", value_ms_display_auto_off/1000);

	timer_ms_display_auto_off = millis();		// Start the display ON timer
	display_status  = DISPLAY_ON;
}

void ssd1306_display_off()
{
	PRINTF_P("Display auto Off at %d sec.\n", value_ms_display_auto_off/1000);

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
