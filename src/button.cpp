// *********************************************
// WSPR Clock TX ESP8266.
// 08/06/2025 Fred Krom, pe0fko
// *********************************************
#include "header.h"

static		uint32_t			timer_ms_20ms_loop			= 0;

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_button()
{
	pinMode(BUTTON_INPUT, INPUT_PULLUP);				// Button for display on/off
	timer_ms_20ms_loop = millis();						// Set the timer for the next 20ms
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------
void loop_button() 
{
	static uint8_t switchStatusLast = HIGH;				// last status hardware switch

	if ((millis() - timer_ms_20ms_loop) < value_ms_20ms_loop)	// Every 20ms...
		return;

	// if (timer_ms_20ms_loop == 0) {		// First time, set the timer and skip the first boot time.
	// 	timer_ms_20ms_loop = millis();
	// 	return;
	// }
	// timer_ms_20ms_loop += value_ms_20ms_loop;
	timer_ms_20ms_loop = millis();							// Set the timer for the next 20ms

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

			// LOG_I("Button pressed, display_status=%d\n", display_status);
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
