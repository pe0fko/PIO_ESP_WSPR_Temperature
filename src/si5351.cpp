// *********************************************
// WSPR Clock TX ESP8266.
// 01/05/2021 Fred Krom, pe0fko
//
// Board: ESP8266	- LOLIN(WeMos) D1 R1 & mini
//					- CPU freq 80MHz
//					- Set Debug port on Serial1
//
// *********************************************

#include "header.h"

Si5351		si5351;

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_si5351()
{
	// Init the frequency generator SI5351
	LOG_I("SI5351 init: xtal:%d, correction:%d\n", SI5351_XTAL_FREQ, config.freq_cal_factor);

	ssd1306_printf_P(1000, PSTR("SI5351\nSetup"));

	// TCXO input to xtal pin 1?
	if ( si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL_FREQ, config.freq_cal_factor) )
	{
		// Disable the clock initially...
		wspr_tx_disable(SI5351_CLK0);
		wspr_tx_disable(SI5351_CLK1);
		wspr_tx_disable(SI5351_CLK2);

		ssd1306_printf_P(1000, PSTR("SI5351\nOk"));
	}
	else
	{
		LOG_E("SI5351 Initialize error\n");
		ssd1306_printf_P(1000, PSTR("SI5351\nERROR"));
	}

#ifdef FEATURE_CARRIER
	LOG_I("CW Carrier on: %fMHz (CLK%d)\n", (float)CARRIER_FREQUENCY/1e6, CARRIER_SI5351_CLK);
	si5351.set_freq( SI5351_FREQ_MULT * CARRIER_FREQUENCY, CARRIER_SI5351_CLK );
	si5351.drive_strength( CARRIER_SI5351_CLK, SI5351_DRIVE_8MA );		// 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
	si5351.set_clock_pwr( CARRIER_SI5351_CLK, 1);
	si5351.output_enable( CARRIER_SI5351_CLK, 1);
#endif
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

void loop_si5351()
{
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
		LOG_E("SI5351 I2C connect error 0x%04X\n", reg_val);
		ssd1306_printf_P(10*1000, PSTR("NO SI5351\n0x%04X"), reg_val);
		return false;
	}

	reg_val = si5351.si5351_read(SI5351_DEVICE_STATUS);
	reg_val &= 0b11100000;	// SYS_INIT | LOL_A | LOL_B
	if (reg_val)
	{
		LOG_E("WSPR TX, the SI5351 is not ready, 0x%02x\n", reg_val);
		ssd1306_printf_P(5*1000, PSTR("SI5351 Error\n0x%02x"), reg_val);
		return false;
	}

	return true;
}

// DEBUG SI5351
float calculate_step_size(float ref_freq, uint8_t pll_mult, uint8_t divider) 
{
	float base_step = ref_freq / (1ULL << 24); // 2^24 = 16.777.216
	return base_step * ((float)pll_mult / divider);
}
