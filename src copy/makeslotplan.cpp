// Project: WSPR TX
// File: ESP_WSPR_TX.cpp
// Created Date: 2023-10-01
//

#include <stdlib.h>
#include <Arduino.h>
#include <coredecls.h>
#include <time.h>
#include <TZ.h>
#include <sntp.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>			// ArduinoJson
#include <si5351.h>					// Etherkit
#include <JTEncode.h>				// Etherkit
#include <DallasTemperature.h>		// Temperature sensor Dallas DS18B20

#include "header.h"

static	void		makeSlotPlanEmpty();
static	void		makeSlotPlanZone();
static	void		makeSlotPlanClk(int clk, const char* zone);
static	void		makeSlotPlanTemp();

//
// Make a plan to TX in one of the 30 slots (2min inteval in a hour).
// The config is read from the json data if available.
//
void makeSlotPlan()
{
	makeSlotPlanEmpty();		// Clear/Empty the slot plan, no TX done yet.

	// Check if the TX is not enabled, default is true, then return.
//	bool tx = false;
//	if (jsonDoc["txenabled"].is<bool>())
	bool tx = jsonDoc["txenabled"] | true;

	LOG_I("TX %s by config.\n", tx ? "enabled" : "disabled");

	if (tx)
	{
		makeSlotPlanZone();			// Make a plan for the zones
		makeSlotPlanTemp();			// Make a plan for the temperature TX
	}

	// Print the slot plan if needed
#ifdef FEATURE_PRINT_TIMESLOT
	LOG_I("Time Slot:");
	for(uint8_t i=0; i < WSPR_SLOTS_HOUR; ++i) 
	{
		if ((i % 4) == 0) LOG_I("\n");
		LOG_I("    %02d:T%d[%d,%d,%d]+%d", i, 
				wspr_slot_type[i], 
				wspr_slot_freq[i][0], 
				wspr_slot_freq[i][1], 
				wspr_slot_freq[i][2], 
				wspr_slot_band[i]
		);
	}
	LOG_I("\n");
#endif
}


void 
makeSlotPlanEmpty()
{
	// LOG_I("Empty the slot plan\n");
	for (int i = 0; i < WSPR_SLOTS_HOUR; i++)
	{
		wspr_slot_type[i]		= WSPR_TX_NONE;
		wspr_slot_band[i]		= random(20, 180);
		wspr_slot_freq[i][0]	= WSPR_TX_FREQ_NONE;
		wspr_slot_freq[i][1]	= WSPR_TX_FREQ_NONE;
		wspr_slot_freq[i][2]	= WSPR_TX_FREQ_NONE;
	}
}


static	int
getHour2Slot(const char* time)
{
	if (time == NULL) 
		return -1;				// No time string

	// Convert the time hour:minute string to a slot number
	// 00:00 ==> 0, 00:02 ==> 1, 00:04 ==> 2, etc.
	// 23:58 ==> 23*30+58/2 = 719
	// LOG_I("Time %s\n", time);

	int hour = atoi(time);
	char* p = strchr(time, ':');
	if (p == NULL) return -1;	// No ':' found
	int min = atoi(p + 1);

	if (hour < 0 || hour > 24) return -1;	// Check hour 0-24!
	if (min < 0 || min > 59) return -1;

	return (hour * 30 + min / 2);
}

static	bool
getStartEndSlot(JsonObject& json, int& zoneStart, int& zoneEnd)
{
	int start, end;
	int sunrise, sunset;
	int sign;
	const char* str;

	// Get the start and end time from the JSON object
	start = getHour2Slot(json["start"]);
	end = getHour2Slot(json["end"]);

	{
		// The start time is not set, then use the sunrise time offset
		int sign = 0;
		const char* str;
		if ((str = json["sunrise"]) == NULL) {
			LOG_E("No sunrise time specified.\n");
			return false;
		}
		if 		(*str == '+') { sign =  1; str++; }
		else if	(*str == '-') { sign = -1; str++; }
		if ((start = getHour2Slot(str)) == -1) {
			LOG_E("Sunrise time conversion error\n");
			return false;
		}

		// "sunrise": "2025-04-23T06:18:19+02:00",
		int sunrise;
		str = QTH.getSunRise().c_str();
		if ((str = strchr(str, 'T')) == NULL) return false;
		str = str + 1;	// Skip the 'T' character
		if ((sunrise = getHour2Slot(str)) == -1) {
			LOG_E("QTH SunRise time conversion error\n");
			return false;
		}

		LOG_I("Time:%s, Start=%d, SunRise=%d, Sign=%d\n", str, start, sunrise, sign);
		start = start * sign + sunrise;
	}

	{
		// Klopt nog niet!!!!!!!!!!!!! Checkit

		int sign = 0;
		const char* str;
		if ((str = json["sunset"]) == NULL) {
			LOG_E("No sunset time specified.\n");
			// Check the "length" token....
			return false;
		}
		if 		(*str == '+') { sign =  1; str++; }
		else if	(*str == '-') { sign = -1; str++; }
		if ((end = getHour2Slot(str)) == -1) {
			LOG_E("Sunset time conversion error\n");
			return false;
		}

		// "sunset": "2025-04-23T20:48:37+02:00",
		int sunset;
		str = QTH.getSunSet().c_str();
		if ((str = strchr(str, 'T')) == NULL) return false;
		str = str + 1;	// Skip the 'T' character
		if ((sunset = getHour2Slot(str)) == -1) {
			LOG_E("QTH Sunset time conversion error\n");
			return false;
		}

		LOG_I("Time:%s, Start=%d, SunSet=%d, Sign=%d\n", str, start, sunset, sign);
		end = end * sign + sunset;
	}

	return true;
}

#if 1

void makeSlotPlanZone()
{
	// Check if this is the end of the last hour slot.
	int hour = hour_now;	// Get the current hour from the system time
	if (slot_now == 29) hour = (hour + 1) % 24;		// Next hour
	LOG_I("Slot Plan Hour: %02d, Slot %d\n", hour, slot_now);

	// Scan the time zones and make a plan for the clock
	if (jsonDoc["timezones"].is<JsonArray>())
	{
		JsonArray timezones = jsonDoc["timezones"];
		LOG_I("JSON Time-Zone:"); serializeJson(timezones, Serial);	LOG_I("\n");
		
		for (JsonObject tz : timezones) 
		{
			const char* listName = tz["list"];				// clk0N, clk0D, clk1, etc.

			if (!jsonDoc[listName].is<JsonArray>()) {
				LOG_E("No freqency list specified in \"%s\".\n", tz["name"] | "empty");
				continue;	// Skip if the list is not a array
			}
			JsonArray jsonList = jsonDoc[listName];

			int start, end;
			if (getStartEndSlot(tz, start, end) == false)
//			||	start >= end)	Sunrise/set probleem zelfde tijd!
			{
				LOG_E("Start/End slot %s time conversion error\n", tz["name"] | "empty");
				continue;
			}

	// // Check next if
	// 		if (start == -1 || end == -1 || start >= end) {
	// 			const char* name = tz["name"] | "empty";
	// 			LOG_E("Start(%d) / End(%d) slot %s time conversion error\n", start, end, name);
	// 			continue;
	// 		}

			int clk		= tz["clk"] | SI5351_CLK0;

			LOG_I("FreqList: %s on clk:%d (length %d), start=%02d:%02d, end=%02d:%02d\n", 
				listName, clk, jsonList.size(), start/30, (start%30)*2, end/30, (end%30)*2);

			size_t idx = start % jsonList.size();	// start in the correct list beginning

			for (int hslot = (hour * 30); hslot < ((hour+1) * 30); hslot += 1, idx += 1)
			{
				if (idx >= jsonList.size())	idx = 0;

				// LOG_I(">> Slot %d, start=%d, end=%d\n", hslot, start, end);
				if (hslot >= end)	break;
				if (hslot >= start)
				{
					int slot = hslot % 30;	// Slot number in the hour

					// LOG_I("Add %02d:%02d\n", hslot/30, hslot%30);

					int band = jsonList[idx].as<int>();
					int freq = getWsprBandFreq(band);

					LOG_I( "   Add Slot %03d, indx:%02d - Clk %d Band %d, Freq %d\n", hslot, idx, clk, band, freq);

					if (freq != WSPR_TX_FREQ_NONE)
						wspr_slot_type[slot] = getWsprSlotType(jsonDoc["wsprtype"][slot] | WSPR_TX_TYPE_2);

					wspr_slot_freq[slot][clk] = freq;
				}
			}
		}
	}
	else
	{
		LOG_I("No zone found, use default\n");
		makeSlotPlanClk(0, "clk0");	// Make a plan for the clock 0
		makeSlotPlanClk(1, "clk1");	// Make a plan for the clock 1
		makeSlotPlanClk(2, "clk2");	// Make a plan for the clock 2
	}
}

#else

void makeSlotPlanZone()
{
	// Scan the time zones and make a plan for the clock
	if (jsonDoc["timezones"].is<JsonArray>())
	{
		// Check if this is the end of the last hour slot.
		int hour = hour_now;	// Get the current hour from the system time
		if (slot_now == 29) hour = (hour + 1) % 24;		// Next hour
		LOG_I("Slot Plan Hour: %02d, Slot %d\n", hour, slot_now);

		JsonArray timezones = jsonDoc["timezones"];
		// JsonArray zones = jsonDoc["timezones"];
		// LOG_I("JSON Zone:"); serializeJson(timezones, Serial);	LOG_I("\n");
		// printJsonDoc("Zones", zones.to<JsonObject>());
		
		for (JsonObject tz : timezones) 
		{
			int start = tz["start"];			// 0-23
			int end = tz["end"];				// 0-24
			int clk = tz["clk"];     			// 0-2
			const char* list = tz["list"];		// "clk0N", etc.

			// Check validity of the zone
			if ((start >= 0) &&	(start < 23) 
			&&	(end >= 0) &&	(end <= 24)
			&&	(start < end))
			{
				// LOG_I("Zone: Start: %2d, End: %2d, Name: %s, now=%d\n", 
				// 	start, end, list, hour);

				if (hour >= start && hour < end)
				{
					LOG_I("Make zone: %s, clk=%d\n", 
						list, clk);

					makeSlotPlanClk(clk, list);	// Make a plan for the clock
				}
			}
			else
			{
				LOG_E("Zone: Start: %2d, End: %2d, Name: %s, now=%d\n", 
					start, end, list, hour);
			}
		}
	}
	else
	{
		LOG_I("No zone found, use default\n");
		makeSlotPlanClk(0, "clk0");	// Make a plan for the clock 0
		makeSlotPlanClk(1, "clk1");	// Make a plan for the clock 1
		makeSlotPlanClk(2, "clk2");	// Make a plan for the clock 2
	}
}
#endif


void 
makeSlotPlanClk(int clk, const char* zone)
{
	// LOG_I("Zone: %s on clk:%d\n", zone, clk);
	if (jsonDoc[zone].is<JsonArray>())
	{
		JsonArray slots = jsonDoc[zone];

		for (size_t slot = 0, idx = 0; slot < WSPR_SLOTS_HOUR;  slot++, idx++)
		{
			if (idx >= slots.size()) 
				idx = 0;	// Loop round the slots

			int band = slots[idx].as<int>();
			int freq = getWsprBandFreq(band);

			// LOG_I( "Slot %02d:%02d - Clk %d zone %s Band %d, Freq %d\n", slot, idx, clk, zone, band, freq);

			if (freq != WSPR_TX_FREQ_NONE)
				wspr_slot_type[slot] = getWsprSlotType(jsonDoc["wsprtype"][slot] | WSPR_TX_TYPE_2);

			wspr_slot_freq[slot][clk] = freq;
		}
	}
	else
	{
		LOG_E("Zone %s not found\n", zone);
		return;
	}
}


void 
makeSlotPlanTemp()
{
#if FEATURE_TEMPERATURE_TX

	//	Add the temperature coding in a seperate hf band.
	int band = jsonDoc["temp_wspr_band"] | WSPR_TX_NONE;

	if (band != WSPR_TX_NONE)	// && sensors.getDeviceCount() >= 1)
	{
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

		LOG_I("TX Slots: temperature %2.1f code [%d, %d, %d]\n", temperature_now, s1, s2, s3);

		wspr_slot_type[s1] = WSPR_TX_TYPE_2;
		wspr_slot_type[s2] = WSPR_TX_TYPE_2;
		wspr_slot_type[s3] = WSPR_TX_TYPE_2;

		int clk  = jsonDoc["temp_wspr_clk"]  | SI5351_CLK0;
		if (clk >= SI5351_CLK0 && clk <= SI5351_CLK2) 
		{
			wspr_slot_freq[s1][clk] = getWsprBandFreq(band);
			wspr_slot_freq[s2][clk] = getWsprBandFreq(band);
			wspr_slot_freq[s3][clk] = getWsprBandFreq(band);
		}
	}
#endif
}
