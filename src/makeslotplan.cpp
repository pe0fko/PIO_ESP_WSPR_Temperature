// Project: WSPR TX
// File: ESP_WSPR_TX.cpp
// Created Date: 2023-10-01
//

#include "header.h"

static	void		makeSlotPlanEmpty();
static	void		makeSlotPlanZone();
static	void		makeSlotPlanClk(int clk, const char* zone);
static	void		makeSlotPlanTemp();
static	bool		getHour2Slot(const char* time, int& slot, bool useSign = false);
static	bool		isoTime2slot(const char* str, int& slot);
static	bool		getStartEndSlot(JsonObject& json, int& zoneStart, int& zoneEnd);
static	uint8_t		getWsprSlotType(int slot);
static	uint32_t	getWsprBandFreq(int band);

//
// Make a plan to TX in one of the 30 slots (2min inteval in a hour).
// The config is read from the json data if available.
//
void makeSlotPlan()
{
	makeSlotPlanEmpty();					// Clear/Empty the slot plan, no TX done yet.

	LOG_I("WSPR TX %s by config.\n", config.wspr_enable ? "enable" : "disable");
	if (config.wspr_enable) 
		makeSlotPlanZone();					// Make a plan for the zones

	LOG_I("Temperature TX %s by config.\n", config.temp_enable ? "enable" : "disable");
	if (config.temp_enable)
		makeSlotPlanTemp();					// Make a plan for the temperature TX

#ifdef FEATURE_PRINT_TIMESLOT
	for(uint8_t i=0; i < WSPR_SLOTS_HOUR; ++i) 
	{
		if ((i % 3) == 0) LOG_I("TimeSlot:");
		LOG_I("\t%02d:T%1d[%d,%d,%d]+%.2f%s", i, 
			wspr_slot_type[i], 	
			wspr_slot_freq[i][0],	wspr_slot_freq[i][1], wspr_slot_freq[i][2], 
			wspr_slot_band[i] / (float)SI5351_FREQ_MULT,
			(i % 3) == 2 ? "\n" : "" );
	}
#endif
}

void 
makeSlotPlanEmpty()
{
	// LOG_I("Empty the slot plan\n");
	for (int i = 0; i < WSPR_SLOTS_HOUR; i++)
	{
		wspr_slot_type[i]		= WSPR_TX_NONE;
		wspr_slot_freq[i][0]	= WSPR_TX_FREQ_NONE;
		wspr_slot_freq[i][1]	= WSPR_TX_FREQ_NONE;
		wspr_slot_freq[i][2]	= WSPR_TX_FREQ_NONE;

		if (config.wspr_band_freq > 0.0) {
			wspr_slot_band[i] = SI5351_FREQ_MULT * config.wspr_band_freq;	// Fixed band frequency
		} else {
#if 1
			wspr_slot_band[i] = SI5351_FREQ_MULT * random(20, 180);			// Random band frequency 20..180 Hz
#else
			#define NF(N)	((N) / (8192.0 / 12000.0))

			wspr_slot_band[i] = SI5351_FREQ_MULT * (NF(10) + random(NF(180))) * NF(1);

			// 8192.0 / 12000.0 = 0,682666666667
			// NF(180) => 180 / (8192.0 / 12000.0) = 263,67
			// NF(200) => 200 / (8192.0 / 12000.0) = 292,96

#endif
		}
	}
}

void makeSlotPlanZone()
{
	// Check if this is the end of the last hour slot.
	int hour = hour_now;	// Get the current hour from the system time
	if (slot_now == 29) hour = (hour + 1) % 24;		// Next hour
	LOG_I("Slot Plan Hour: %02dh (Slot %d)\n", hour, slot_now);

	// Scan the time zones and make a plan for the clock
	if (jsonDoc["timezones"].is<JsonArray>())
	{
		JsonArray timezones = jsonDoc["timezones"];
		// LOG_I("JSON Time-Zone:"); serializeJson(timezones, Serial);	LOG_I("\n");
		
		int start = 0;		// Start slot, 0..29
		int end = 0;		// End slot, 0..29

		for (JsonObject tz : timezones) 
		{
			// LOG_I(">>>>> TZ: \"%s\": Start=%d, End=%d.\n", tz["name"] | "empty", start, end);

			if (tz["enable"].isNull() == false && 
				tz["enable"].as<bool>() == false) {
					continue;	// Skip if enable field and not enabled(false)
			}

			if (!getStartEndSlot(tz, start, end)) {
				LOG_E("Start/End slot %s time conversion error\n", tz["name"] | "empty");
				continue;
			}

			if (tz["list"].isNull()) {
				LOG_E("No frequency list specified in \"%s\".\n", tz["name"] | "empty");
				continue;	// Skip if no list field
			}

			const char* listName = tz["list"];		// Frequency list name
			if (!jsonDoc[listName].is<JsonArray>()) {	// Check for list array type
				LOG_E("No frequency list \"%s\" defined, used in \"%s\".\n", listName, tz["name"] | "empty");
				continue;
			}
			JsonArray jsonList = jsonDoc[listName];

			int clk = tz["clk"] | SI5351_CLK0;

#ifdef DEBUG
			if (display_status == DISPLAY_ON) 
			{
				LOG_I("Freq-List: %s on clk:%d, start=%02d:%02d, end=%02d:%02d, List: ", 
					listName, clk, start/30, (start%30)*2, end/30, (end%30)*2);
				serializeJson(jsonList, Serial); 
				LOG_I("\n");
			}
#endif

			// size_t idx = start % jsonList.size();	// start in the correct list beginning
			size_t idx = 0;			// start in the hour band sequence
			for (int hslot = (hour * WSPR_SLOTS_HOUR); hslot < ((hour+1) * WSPR_SLOTS_HOUR); hslot += 1, idx += 1)
			{
				if (idx >= jsonList.size())	idx = 0;

				// LOG_I(">> Slot %d, start=%d, end=%d\n", hslot, start, end);
				if (hslot >= end)	break;
				if (hslot >= start)
				{
					int slot = hslot % WSPR_SLOTS_HOUR;	// Slot number in the hour
					int band = jsonList[idx].as<int>() | WSPR_TX_NONE;
					int freq = getWsprBandFreq(band);

					// LOG_I( "   Add Slot %02d:%02d, indx:%02d - Clk %d Band %d, Freq %d\n", 
					// 			hslot/30, hslot%30, idx, clk, band, freq);

					wspr_slot_freq[slot][clk]	= freq;
					if (freq != WSPR_TX_FREQ_NONE)
						wspr_slot_type[slot]	= getWsprSlotType(slot);
				}
			}

			start = end; end = 0;			// Reset for next use
			start %= WSPR_SLOTS_HOUR * 24;	// Keep in range 0..719 slots in a day
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

			wspr_slot_freq[slot][clk]	= freq;
			if (freq != WSPR_TX_FREQ_NONE)
				wspr_slot_type[slot]	= getWsprSlotType(slot);
		}
	}
	else
	{
		LOG_E("Zone %s not found\n", zone);
		return;
	}
}

/*
"temperature": { "band":21, "clk":0, "enable":true }
*/
void 
makeSlotPlanTemp()
{
#if FEATURE_TEMPERATURE_TX

	int band = config.temp_band;

	if (band != WSPR_TX_NONE)	// && sensors.getDeviceCount() >= 1)
	{
		int   s1,s2,s3,t;
		float tf;

		readTemperature();

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

		// int clk  = jsonDoc["temperature"]["clk"] | SI5351_CLK0;
		int clk  = config.temp_clk;
		if (clk >= SI5351_CLK0 && clk <= SI5351_CLK2) 
		{
			wspr_slot_type[s1] = WSPR_TX_TYPE_2;
			wspr_slot_type[s2] = WSPR_TX_TYPE_2;
			wspr_slot_type[s3] = WSPR_TX_TYPE_2;

			wspr_slot_band[s1] = SI5351_FREQ_MULT * 50;
			wspr_slot_band[s2] = SI5351_FREQ_MULT * 100;
			wspr_slot_band[s3] = SI5351_FREQ_MULT * 150;

			wspr_slot_freq[s1][clk] = getWsprBandFreq(band);
			wspr_slot_freq[s2][clk] = getWsprBandFreq(band);
			wspr_slot_freq[s3][clk] = getWsprBandFreq(band);
		}
	}
#endif
}


#if 1

// Convert the time hour:minute string to a slot number
// 00:00 ==> 0, 00:02 ==> 1, 00:04 ==> 2, etc.
// 23:58 ==> 23*30+58/2 = 719
bool
getHour2Slot(const char* time, int& slot, bool useSign)
{
	if (time == NULL) 	return false;			// No time string

	int sign = 1;
	if (useSign && (*time == '-' || *time == '+')) {
		if		(*time == '-') sign = -1;
		else if (*time == '+') sign = 1;
		time++;
	}

	int hour = atoi(time);
	if ((time = strchr(time, ':')) == NULL) return false;	// No ':' found
	int min = atoi(time + 1);
	if (hour > 24) return false;	// Check hour 0-24!
	if (min  > 59) return false;

	slot = sign * (hour * WSPR_SLOTS_HOUR + min / 2);
	return true;
}

#else

int time_to_day_slot(const char* data_time);

// Convert the time hour:minute string to a slot number
// 00:00 ==> 0, 00:02 ==> 1, 00:04 ==> 2, etc.
// 23:58 ==> 23*30+58/2 = 719
bool
getHour2Slot(const char* time, int& slot, bool useSign)
{
	if (time == NULL) 	return false;			// No time string

}

int time_to_day_slot(const char* data_time)
{
	// 2025-06-10 19:16:00 - Jaar: 125, Maand: 5, Dag: 10, Uur: 19, Minuut: 16
	struct tm ts;				// Tijdstructuur
	strptime(data_time, "%Y-%m-%d %H:%M:%S", &ts);

	// Slot in 1 dag is 720
	int slot = (ts.tm_hour * 60 + ts.tm_min) / 2;

	if (slot < 0 || slot >= 720) {
		log_e("Invalid time slot: %d", slot);
		return -1; // Invalid slot
	}
	// log_d("Time: %s, Slot: %d", data_time, slot);
	// log_d("Year: %d, Month: %d, Day: %d, Hour: %d, Minute: %d",
	// 	ts.tm_year, ts.tm_mon, ts.tm_mday, ts.tm_hour, ts.tm_min);

	// log_d("Time to day slot: %d", slot);
	return slot;
}
#endif





// Convert the ISO time string to a slot number
// "2025-04-23T06:18:19+02:00" ==> 6*30+18/2 = 189
bool
isoTime2slot(const char* str, int& slot)
{
	if (str == NULL) return false;
	if ((str = strchr(str, 'T')) == NULL) return false;
	str += 1;			// Skip the 'T' character

	if (!getHour2Slot(str, slot)) {
		LOG_E("ISO time conversion error\n");
		return false;
	}
	return true;
}

/* New table
	1	2	4	8	16	0			
	Start	End	Length	SunRise	SunSet	Begin		Start out	End out
3	1	1	0	0	0	0		Start	End
5	1	0	1	0	0	0		Start	Start+Length
6	0	1	1	0	0	0		End-Length	End
9	1	0	0	1	0	0		Start	SunRise
10	0	1	0	1	0	0		SunRise	End
12	0	0	1	1	0	0		SunRise	SunRise+Length
17	1	0	0	0	1	0		Start	SunSet
18	0	1	0	0	1	0		SunSet	End
20	0	0	1	0	1	0		SunSet-Length	SunSet
24	0	0	0	1	1	0		SunRise	SunSet
									
2	0	1	0	0	0	1		Begin	End
4	0	0	1	0	0	1		Begin	Begin+Length
8	0	0	0	1	0	1		Begin	SunRise
16	0	0	0	0	1	1		Begin	SunSet
*/

bool
getStartEndSlot(JsonObject& json, int& zoneStart, int& zoneEnd)
{
	int		start		= 0;
	int		end			= 0;
	int		length		= 0;
	int		sunrise		= 0;
	int		sunset		= 0;
	int		begin		= zoneStart;
	int		flags		= 0;

	flags		|= getHour2Slot(json["start"], start, false)	? 1 : 0;
	flags		|= getHour2Slot(json["end"], end, false)		? 2 : 0;
	flags		|= getHour2Slot(json["length"], length, true)	? 4 : 0;

	flags		|= json["sunrise"].isNull() == false
				&&	getHour2Slot(json["sunrise"], sunrise, true)
				&&	isoTime2slot(QTH.getSunRise().c_str(), sunrise) ? 8 : 0;

	flags		|= json["sunset"].isNull() == false
				&&	getHour2Slot(json["sunset"], sunset, true)
				&&	isoTime2slot(QTH.getSunSet().c_str(), sunset) ? 16 : 0;

	// LOG_I("FLAG:0x%02x, Start: %d, End: %d, Length: %d, Sunrise: %d, Sunset: %d\n", 
	// 	flags, start, end, length, sunrise, sunset);

	switch(flags) {
		case 2:		zoneStart = begin;			zoneEnd = end;				break;
		case 3:		zoneStart = start;			zoneEnd = end;				break;
		case 4:		zoneStart = begin;			zoneEnd = begin+length;		break;
		case 5:		zoneStart = start;			zoneEnd = start+length;		break;
		case 6:		zoneStart = end-length;		zoneEnd = end;				break;
		case 8:		zoneStart = begin;			zoneEnd = sunrise;			break;
		case 9:		zoneStart = start;			zoneEnd = sunrise;			break;
		case 10:	zoneStart = sunrise;		zoneEnd = end;				break;
		case 12:	zoneStart = sunrise;		zoneEnd = sunrise+length;	break;
		case 16:	zoneStart = begin;			zoneEnd = sunset;			break;
		case 17:	zoneStart = start;			zoneEnd = sunset;			break;
		case 18:	zoneStart = sunset;			zoneEnd = end;				break;
		case 20:	zoneStart = sunset-length;	zoneEnd = sunset;			break;
		case 24:	zoneStart = sunrise;		zoneEnd = sunset;			break;
		default:
			LOG_E("Invalid combination of start/end/length/sunrise/sunset! 0x%02x\n", flags);
			return false;
	};

	if (zoneStart < zoneEnd)
		return true;

	LOG_E("Invalid start stop time direction! 0x%02x\n", flags);
	
	return false;
}

uint8_t
getWsprSlotType(int slot)
{
	int type = WSPR_TX_TYPE_2;

	if (jsonDoc["wsprtype"].is<JsonArray>())
	{
		int length = jsonDoc["wsprtype"].size();
		if (length <= 0) return WSPR_TX_TYPE_2;
		type = jsonDoc["wsprtype"][slot % length] | WSPR_TX_TYPE_2;
	}

	if (type < WSPR_TX_NONE || type > WSPR_TX_TYPE_3) 
	{
		LOG_E("Wrong WSPR type %d selected.\n", type);
		type = WSPR_TX_TYPE_2;
	}

	return type;
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
		// case 70:	return	WSPR_TX_FREQ_4m;	// 4m   70.092400 - 70.092600?
		// case 144:	return	WSPR_TX_FREQ_2m;	// 2m  144.489900 - 144.490100
		// case -1:								// LF  ?
		// case 433:								// 70cm 433. - 433.
		// case 1296:								// 23cm  1296. - 1296.
		// 			LOG_E("Not implemented WSPR bands\n");
		// 			break;
		default:	LOG_E("Wrong WSPR band %d selected.\n", band);
					break;						// No TX mode
	}
	return WSPR_TX_FREQ_NONE;
}
