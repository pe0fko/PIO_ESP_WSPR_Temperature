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
//

#include "header.h"

static	JTEncode	wspr;

static	uint32_t	timer_us_wspr_bit		= 0;

uint8_t				wspr_symbols[WSPR_SYMBOL_COUNT];
uint32_t			wspr_symbol_index		= 0;
uint8_t				wspr_slot_type[WSPR_SLOTS_HOUR];			// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
uint8_t				wspr_slot_band[WSPR_SLOTS_HOUR];			// Band freqency, 0 .. 200 Hz
uint32_t			wspr_slot_freq[WSPR_SLOTS_HOUR][3];			// TX frequency for every CLK output (0..2)

const	int32_t      	wspr_sym_freq[4] =
{	static_cast<uint32_t> ( 0.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 1.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 2.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
,	static_cast<uint32_t> ( 3.0 * 12000.0/8192.0 * (float)SI5351_FREQ_MULT + 0.5)
};
	

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_wspr_tx()
{
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

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
				LOG_I("WSPT-Bit %u overflow %d us.\n", wspr_symbol_index, diff - value_us_wspr_bit);
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

void wspr_tx_init(const char* call)
{
	timer_us_wspr_bit = micros();		// Start timer close to the 1sec tick

	if (si5351_ready())
	{
		LOG_I("WSPR TX Init: Hour:%2u Slot:%2u, CALL=%s, QTH=%s, Freq=%d/%d/%d(+%d)Hz, Power=%ddBm\n", 
			hour_now, slot_now, 
			call,
			config.qth.c_str(),
			wspr_slot_freq[slot_now][0],
			wspr_slot_freq[slot_now][1],
			wspr_slot_freq[slot_now][2],
			wspr_slot_band[slot_now],
			config.power
		);

		wspr.wspr_encode(call, config.qth.c_str(), config.power, wspr_symbols);

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
		print_wspr_symbols(call, config.qth.c_str(), config.power, wspr_symbols);
#endif

		// timer_us_wspr_bit = micros();
		// wspr_symbol_index = 0;
		wspr_tx_bit();
	}
	else
	{
		LOG_E("WSPR TX not started, SI5351 not ready.\n");
	}
}

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
// "PE0FKO JO32 10"
// SYMBOL: 3,3,2,0,2,2,0,2,3,0,2,2,3,3,3,0,0,0,1,2,0,1,2,3,1,1,3,2,0,2,2,2,2,0,1,2,0,1,2,3,2,0,0,0,2,0,3,0,1,1,2,2,1,3,0,3,0,2,2,3,1,2,1,0,0,2,0,3,1,2,1,0,3,0,3,0,1,0,0,1,0,0,1,2,3,1,2,0,2,3,1,2,3,2,3,2,0,2,1,2,0,0,0,0,1,2,2,1,2,0,1,3,1,2,3,1,2,0,1,3,2,3,0,2,0,3,1,1,2,0,2,2,2,1,0,1,2,2,3,3,0,2,2,2,2,2,2,1,1,2,1,0,1,3,2,2,0,1,3,2,2,2
void print_wspr_symbols(const char* call, const char* loc, uint8_t power, uint8_t symbols[])
{
	LOG_I("%s %s %ddBm:\n  ", call, loc, power);
	for (uint8_t i = 0; i < WSPR_SYMBOL_COUNT; )
	{
		LOG_I("%d,", symbols[i++]);
		if (i % 41 == 0) LOG_I("\n  ");
	}
	LOG_I("\n");
}
#endif

void wspr_tx_bit()
{
	// if (wspr_symbol_index != WSPR_SYMBOL_COUNT)
	if (wspr_symbol_index < WSPR_SYMBOL_COUNT)
	{
		if (wspr_symbol_index == 0)   							// On first bit enable the tx output.
		{
			// // DEBUG ///////////////////////////////////////////////////////////////////////////////////////////////////
			// struct timeval tv;
			// gettimeofday(&tv, NULL);					// Get the current time in sec and usec
			// LOG_D("WSPR start time [%ld us] %s", tv.tv_usec, ctime(&tv.tv_sec));
			// // DEBUG ///////////////////////////////////////////////////////////////////////////////////////////////////
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

		// LOG_D("TX WSPR #%d Ended.\n", wspr_tx_counter);
	}
}

void wspr_tx_freq(si5351_clock clk)
{
	if (wspr_slot_type[slot_now] != WSPR_TX_NONE && 
		wspr_slot_freq[slot_now][clk] != WSPR_TX_FREQ_NONE)
	{
		uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[slot_now][clk] + wspr_slot_band[slot_now]);

		if (si5351.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) ) {
			LOG_E("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
		}

		// {	// TESTING
		// 	// Lees PLL- en divider-instellingen
		// 	uint64_t pll_freq = si5351.get_pll_frequency(SI5351_PLL_A);
		// 	uint8_t divider = si5351.get_multisynth_divider(SI5351_CLK0);

		// 	Serial.print("PLL Freq: "); Serial.println(pll_freq);
		// 	Serial.print("Divider: "); Serial.println(divider);
		// 	LOG_I("WSPR Freq=%d step=%f\n", 
		// 		wspr_slot_freq[slot_now][0], wspr_slot_freq[slot_now][1], wspr_slot_freq[slot_now][2]);
		// }
	}
}

void wspr_tx_enable(si5351_clock clk)
{
	if (wspr_slot_type[slot_now] != WSPR_TX_NONE && 
		wspr_slot_freq[slot_now][clk] != 0)
	{
		// LOG_D("TX WSPR start CLK%d: slot %d, freq %.6fMHz + %dHz\n", 
		// 		clk, slot_now, 
		// 		wspr_slot_freq[slot_now][clk] / 1000000.0, 
		// 		wspr_slot_band[slot_now]);

//		si5351.drive_strength(clk, SI5351_DRIVE_8MA); 			// 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
		si5351.set_clock_pwr(clk, 1);
		si5351.output_enable(clk, 1);
		si5351.drive_strength(clk, SI5351_DRIVE_8MA);
	}
 }

void wspr_tx_disable(si5351_clock clk)
{
	si5351.output_enable(clk, 0);
	si5351.set_clock_pwr(clk, 0);
}
