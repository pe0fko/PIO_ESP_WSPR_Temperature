// *********************************************
// WSPR TX Temperature ESP8266.
// 01/05/2021 Fred Krom, pe0fko
//
// The temperature sensor DS18B20 from Dallas
// Semiconductor is used to read the temperature
// and send it via WSPR TX.
// *********************************************

#include "header.h"
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20

static		OneWire				oneWire(ONE_WIRE_BUS);	// Temp sensor
static		DallasTemperature	sensors(&oneWire);		// Dallas DS18B20
float		temperature_now		= 0.0;

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_ds18b20()
{
	sensors.begin();							// Init the onewire for the DS18B20 temp sensor
	sensors.begin();							// Strange the first call sometimes misses the sensor

	sensors.setResolution(12);					// Set the resolution to 12 bit
	sensors.setWaitForConversion(true);			// Blocking wait for the conversion
	sensors.requestTemperatures();

	LOG_I("DS18B20 Setup: %d device(s) found, device #0 %.2fºC\n", 
		sensors.getDeviceCount(), 
		sensors.getTempCByIndex(0)
	);
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

void loop_ds18b20()
{
}

void readTemperature()
{
	sensors.begin();							// Init the onewire for the DS18B20 temp sensor???
	sensors.begin();							// Strange the first call sometimes misses the sensor
	sensors.requestTemperatures();

	temperature_now = sensors.getTempCByIndex(0) + config.temp_offset;
	LOG_I("Sensor DS18B20 (#%d) temperature %.1fºC\n", sensors.getDeviceCount(), temperature_now);
}
