#include "header.h"
#include <Adafruit_GFX.h>			// Adafruit GFX Library
#include <Adafruit_SSD1306.h>		// Adafruit SSD1306 Wemos Mini OLED
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20

Adafruit_SSD1306	display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static		uint32_t			timer_ms_display_auto_off	= 0;
static		uint32_t			timer_ms_led_blink_on		= 0;
static		uint32_t			timer_ms_led_blink_off		= 0;

uint32_t			wspr_tx_counter        	= 0;
display_status_t	display_status			= DISPLAY_ON;		// Display status on/off

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_display()
{
	pinMode(LED_BUILTIN, OUTPUT);				// LED on the ESP
	digitalWrite(LED_BUILTIN, HIGH);			// High is off!
	pinMode(BUTTON_INPUT, INPUT_PULLUP);		// Button for display on/off

	// Start the display driver
	// Wire.begin(D2, D1);						// I2C SDA, SCL
	Wire.begin();								// I2C SDA, SCL
	Wire.setClock(400000);						// I2C clock speed 400kHz fast
	display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
	display.setRotation(0);						// Display normal wide screen

	ssd1306_printf_P(4000, PSTR("PE0FKO WSPR TX\nID %d\n%s.local"), ESP.getChipId(), config.hostname.c_str());
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------
void loop_display()
{
	if (wspr_symbol_index != 0)				// At TX WSPR message blink LED
	// if (semaphore_wifi_connected)
	// if (semaphore_wifi_ip_address)
	// if (semaphore_wifi_ntp_received)
	{
		if (timer_ms_led_blink_on == 0)
		{
			timer_ms_led_blink_on = millis();
		}
	}
	else
	{
		timer_ms_led_blink_on = 0;
		// digitalWrite(LED_BUILTIN, HIGH);						// LED Off
	}

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

	if (semaphore_wifi_ntp_received)
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
	// else
	// {
	// 	if (WiFi.status() != WL_CONNECTED)
	// 		ssd1306_center_string("-- NO WiFi --", 16);
	// 	else
	// 		ssd1306_center_string("-- NO sNTP --", 16);
	// 	ssd1306_center_string    ("-- Syncing --", 32);
	// }

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

#ifdef DEBUG
	static int cnt = 0;
	sprintf(buf_count, "%d", cnt++);
#else
	sprintf(buf_count, "%d", wspr_tx_counter);
#endif

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

// void ssd1306_wifi_page()
// {
// 	LOG_I("Connected to %s (IP:%s/%s, GW:%s, RSSI %d)\n"
// 		, WiFi.SSID().c_str()
// 		, WiFi.localIP().toString().c_str()							// Send the IP address of the ESP8266 to the computer
// 		, WiFi.subnetMask().toString().c_str()
// 		, WiFi.gatewayIP().toString().c_str()
// 		, WiFi.RSSI());
// 	ssd1306_printf_P(1000, PSTR("IP:%s\nGW:%s\nRSSI:%d"),
// 		WiFi.localIP().toString().c_str(),
// 		WiFi.gatewayIP().toString().c_str(),
// 		WiFi.RSSI() );
// }

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
		String ssid("SSID:");
		ssid += WiFi.SSID();
		ssd1306_center_string(ssid.c_str(), 16+12+12+12);
	}

#ifdef DEBUG
	// The display will interupt the WSPR TX
	ssd1306_display_on();
#endif
	display.display();

// #ifdef DEBUG_ESP_PORT
// 	LOG_I("SSD1306[[ ");
// 	if (txt1 != NULL)	LOG_I("%s",txt1);
// 	if (txt2 != NULL) {	LOG_I(" / "); LOG_I("%s",txt2); }
// 	if (txt3 != NULL) {	LOG_I(" / "); LOG_I("%s",txt3); }
// 	LOG_I(" ]](%dms)\n", wait);
// #endif

	delay(wait);
}

void ssd1306_display_on()
{
	// LOG_I("Display On for %dsec.\n", config.displayOff);
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
