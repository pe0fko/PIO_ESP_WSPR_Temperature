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

#include "header.h"
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20

static		OneWire				oneWire(ONE_WIRE_BUS);						// Temp sensor
static		DallasTemperature	sensors(&oneWire);							// Dallas DS18B20
float		temperature_now		= 0.0;

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_ds18b20()
{
	sensors.begin();							// Init the onewire for the DS18B20 temp sensor
	sensors.setResolution(12);					// Set the resolution to 12 bit
	// sensors.setWaitForConversion(true);		// No blocking wait for the conversion
	// sensors.setWaitForConversion(true);		// Block until the sensor is read
	sensors.requestTemperatures();

	LOG_I("DS18B20: %d device(s) found\n", sensors.getDeviceCount());

	readTemperature();							// Read the Dallas temperature one-wire sensor
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

void loop_ds18b20()
{
}

void readTemperature()
{
	sensors.setWaitForConversion(true);	// Block until the sensor is read
	sensors.requestTemperatures();

	// int devices = sensors.getDeviceCount();

	temperature_now = sensors.getTempCByIndex(0) + config.temp_cor;

	LOG_I("Sensor DS18B20 (#%d) temperature %.1fºC\n", sensors.getDeviceCount(), temperature_now);
}
