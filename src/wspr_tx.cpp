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
Ticker	wsprTicker;												// WSPR TX ticker

// static	uint32_t	timer_us_wspr_bit		= 0;

uint8_t				wspr_symbols[WSPR_SYMBOL_COUNT];
uint32_t			wspr_symbol_index		= 0;
uint8_t				wspr_slot_type[WSPR_SLOTS_HOUR];			// 0=None, 1="CALL", 2="P/CALL/S", 3="<P/CALL/S>"
uint32_t			wspr_slot_band[WSPR_SLOTS_HOUR];			// Band freqency, SI5351_FREQ_MULT * (0 .. 200) Hz
uint32_t			wspr_slot_freq[WSPR_SLOTS_HOUR][3];			// TX frequency for every CLK output (0..2)

static	int32_t		wspr_sym_freq[4] = {0};

static	void		wspr_tx_freq(si5351_clock clk);
static	void		wspr_tx_enable(si5351_clock clk);
static	enum si5351_drive	get_drive_strength();

//---------------------------------------------------------------------------------
//---- SETUP....  SETUP....  SETUP....  SETUP....  SETUP....
//---------------------------------------------------------------------------------
void setup_wspr_tx()
{
	for(int i = 0; i < 4; ++i) 
	{
		float df = (float)i * 12000.0 / 8192.0;
		df *= SI5351_FREQ_MULT;
		df *= config.wspr_tone_mul;
		df += 0.5;
		// LOG_I("WSPR tone %d freq calc: %.2f Hz\n", i, df / SI5351_FREQ_MULT);
		wspr_sym_freq[i] = (int32_t)df;
	}
	LOG_I("WSPR tone frequencies (Hz): %.2f, %.2f, %.2f, %.2f\n", 
		(float)wspr_sym_freq[0] / SI5351_FREQ_MULT, 
		(float)wspr_sym_freq[1] / SI5351_FREQ_MULT, 
		(float)wspr_sym_freq[2] / SI5351_FREQ_MULT, 
		(float)wspr_sym_freq[3] / SI5351_FREQ_MULT
	);
}

//---------------------------------------------------------------------------------
//---- LOOP....  LOOP....  LOOP....  LOOP....  LOOP....
//---------------------------------------------------------------------------------

void loop_wspr_tx()
{
// 	// Send the WSPR bits into the air if active TX!
// 	// When started it will wait for the bit time and start a next bit.

// #ifdef FEATURE_CHECK_TIMING
// 	if (wspr_symbol_index != 0) 
// 	{
// 		uint32_t	diff = micros() - timer_us_wspr_bit;
// 		if (diff >= value_us_wspr_bit)
// 		{
// 			timer_us_wspr_bit += value_us_wspr_bit;
// 			wspr_tx_bit();										// Ok, transmit the net tone bit

// 			if (diff >= (value_us_wspr_bit+500UL))
// 			{
// 				LOG_I("WSPT-Bit %u overflow %d us.\n", wspr_symbol_index, diff - value_us_wspr_bit);
// 			}
// 		}
// 	}
// #else
// 	if ((wspr_symbol_index != 0) 
// 	&&  (micros() - timer_us_wspr_bit) >= value_us_wspr_bit)
// 	{
// 		timer_us_wspr_bit += value_us_wspr_bit;
// 		wspr_tx_bit();											// Ok, transmit the next tone bit
// 	}
// #endif
}

void wspr_tx_init(const char* call)
{
	if (wsprTicker.active())
	{
		LOG_W("WSPR TX already active, ignoring init request.\n");
		return;
	}

	if (!si5351_ready())
	{
		LOG_E("WSPR TX not started, SI5351 not ready.\n");
		return;
	}

	wspr.wspr_encode(call, config.user_locator.c_str(), config.user_power, wspr_symbols);

	// timer_us_wspr_bit = micros();		// Start timer close to the 1sec tick

	wsprTicker.attach_ms(value_ms_wspr_bit, wspr_tx_bit);	// Start WSPR TX ticker in system context
	wspr_tx_bit();											// Start the wspr TX for 110.592 sec

	LOG_I("WSPR TX Init: Hour:%2u Slot:%2u, CALL=%s, QTH=%s, Freq=%d/%d/%d(+%.2f)Hz, Power=%ddBm, Drive=%d\n", 
		hour_now, slot_now, 
		call,
		config.user_locator.c_str(),
		wspr_slot_freq[slot_now][0],
		wspr_slot_freq[slot_now][1],
		wspr_slot_freq[slot_now][2],
		wspr_slot_band[slot_now] / (float)SI5351_FREQ_MULT,
		config.user_power,
		config.si5351_drive_strength
	);

#ifdef FEATURE_PRINT_WSPR_SIMBOLS
	print_wspr_symbols(call, config.user_locator.c_str(), config.user_power, wspr_symbols);
#endif
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

			// LOG_D("TX WSPR #%d Started.\n", wspr_tx_counter);
		}

		wspr_tx_freq(SI5351_CLK0);
		wspr_tx_freq(SI5351_CLK1);
		wspr_tx_freq(SI5351_CLK2);

		wspr_symbol_index += 1;
	}
	else
	{
		wspr_tx_disable();											// Disable the WSPR TX Si5351
		wspr_symbol_index = 0;
		wspr_tx_counter += 1;
		wsprTicker.detach();										// Stop WSPR TX ticker

		// LOG_D("TX WSPR #%d Ended.\n", wspr_tx_counter);
	}
}

void wspr_tx_freq(si5351_clock clk)
{
	if (wspr_slot_type[slot_now] != WSPR_TX_NONE && 
		wspr_slot_freq[slot_now][clk] != WSPR_TX_FREQ_NONE)
	{
		// uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[slot_now][clk] + wspr_slot_band[slot_now]);
		// if ( si5351_clockgen.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) ) {
		// 	LOG_E("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
		// }

		// uint64_t wspr_frequency = SI5351_FREQ_MULT * (wspr_slot_freq[slot_now][clk] + wspr_slot_band[slot_now]);
		uint64_t wspr_frequency
			= wspr_slot_freq[slot_now][clk] * SI5351_FREQ_MULT
			+ wspr_slot_band[slot_now];

		if ( si5351_clockgen.set_freq( wspr_frequency + wspr_sym_freq[wspr_symbols[wspr_symbol_index]], clk ) ) {
			LOG_E("ERROR: wspr_tx_freq(%d) / SI5351::set_freq(...)\n", clk);
		}

		// {	// TESTING
		// 	// Lees PLL- en divider-instellingen
		// 	uint64_t pll_freq = si5351_clockgen.get_pll_frequency(SI5351_PLL_A);
		// 	uint8_t divider = si5351_clockgen.get_multisynth_divider(SI5351_CLK0);

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

		si5351_clockgen.set_clock_pwr(clk, 1);
		si5351_clockgen.output_enable(clk, 1);
		si5351_clockgen.drive_strength(clk, get_drive_strength());
	}
 }

// drive_strength: 
// Possible values: 2mA= dBm, 4mA=3dBm, 6mA= dBm, 8mA=10dBm
enum si5351_drive 
get_drive_strength()
{
	switch(config.si5351_drive_strength)
	{
		case 2:	return SI5351_DRIVE_2MA;
		case 4:	return SI5351_DRIVE_4MA;
		case 6:	return SI5351_DRIVE_6MA;
		case 8:	return SI5351_DRIVE_8MA;
		default: break;
	}
	return SI5351_DRIVE_8MA;
}

void wspr_tx_disable()
{
	si5351_clockgen.output_enable(SI5351_CLK0, 0);
	si5351_clockgen.set_clock_pwr(SI5351_CLK0, 0);

	si5351_clockgen.output_enable(SI5351_CLK1, 0);
	si5351_clockgen.set_clock_pwr(SI5351_CLK1, 0);

	si5351_clockgen.output_enable(SI5351_CLK2, 0);
	si5351_clockgen.set_clock_pwr(SI5351_CLK2, 0);
}


/*

//
// Task to handle WSPR TX
void WsprTask( void * pvParameters )
{
	(void) pvParameters;

	for( ;; )
	{
		loop_wspr_tx();

		vTaskDelay(1 / portTICK_PERIOD_MS);		// Yield to other tasks
	}
}


// TaskHandle_t WsprTaskHandle;
// xTaskCreate(WsprTask, "WsprTask", 2048, NULL, 1, &WsprTaskHandle);




#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"

// Semafoor handles
SemaphoreHandle_t binary_semaphore;
SemaphoreHandle_t mutex_semaphore;
SemaphoreHandle_t counting_semaphore;

// Gedeelde resource
int shared_counter = 0;

// Task die binaire semafoor geeft
void trigger_task(void *pvParameters)
{
    int count = 0;
    
    while(1) {
        printf("Trigger Task: Geef semafoor signaal (%d)\n", count++);
        xSemaphoreGive(binary_semaphore);
        
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

// Task die wacht op binaire semafoor
void wait_task(void *pvParameters)
{
    while(1) {
        printf("Wait Task: Wacht op semafoor...\n");
        
        if(xSemaphoreTake(binary_semaphore, portMAX_DELAY) == pdTRUE) {
            printf("  --> Wait Task: Semafoor ontvangen! Voer actie uit.\n");
            // Doe iets belangrijks hier
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
}

// Task die gedeelde resource gebruikt (met mutex)
void resource_task_1(void *pvParameters)
{
    while(1) {
        // Vraag mutex aan
        if(xSemaphoreTake(mutex_semaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
            printf("Task 1: Mutex verkregen, counter = %d\n", shared_counter);
            
            // Kritieke sectie - wijzig gedeelde variabele
            shared_counter++;
            vTaskDelay(100 / portTICK_PERIOD_MS); // Simuleer werk
            printf("Task 1: Counter verhoogd naar %d\n", shared_counter);
            
            // Geef mutex vrij
            xSemaphoreGive(mutex_semaphore);
        } else {
            printf("Task 1: Timeout bij wachten op mutex\n");
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Tweede task die dezelfde resource gebruikt
void resource_task_2(void *pvParameters)
{
    while(1) {
        if(xSemaphoreTake(mutex_semaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
            printf("Task 2: Mutex verkregen, counter = %d\n", shared_counter);
            
            shared_counter += 10;
            vTaskDelay(150 / portTICK_PERIOD_MS);
            printf("Task 2: Counter verhoogd naar %d\n", shared_counter);
            
            xSemaphoreGive(mutex_semaphore);
        } else {
            printf("Task 2: Timeout bij wachten op mutex\n");
        }
        
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
}

// Producer voor counting semaphore
void producer_task(void *pvParameters)
{
    int item = 0;
    
    while(1) {
        printf("Producer: Produceer item %d\n", item);
        
        // Geef counting semafoor (max 5 items)
        if(xSemaphoreGive(counting_semaphore) == pdTRUE) {
            printf("  --> Item %d toegevoegd\n", item);
            item++;
        } else {
            printf("  --> Buffer vol! Kan niet produceren\n");
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Consumer voor counting semaphore
void consumer_task(void *pvParameters)
{
    while(1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        // Neem counting semafoor
        if(xSemaphoreTake(counting_semaphore, 500 / portTICK_PERIOD_MS) == pdTRUE) {
            printf("Consumer: Item geconsumeerd\n");
            vTaskDelay(500 / portTICK_PERIOD_MS); // Simuleer verwerking
        } else {
            printf("Consumer: Geen items beschikbaar\n");
        }
    }
}

void app_main()
{
    printf("\n=== ESP8266 Semafoor Voorbeelden ===\n\n");
    
    // Maak binaire semafoor (voor signalering)
    binary_semaphore = xSemaphoreCreateBinary();
    
    // Maak mutex (voor gedeelde resources)
    mutex_semaphore = xSemaphoreCreateMutex();
    
    // Maak counting semaphore (max 5 items)
    counting_semaphore = xSemaphoreCreateCounting(5, 0);
    
    if(binary_semaphore == NULL || mutex_semaphore == NULL || counting_semaphore == NULL) {
        printf("FOUT: Semaforen aanmaken mislukt!\n");
        return;
    }
    
    printf("--- Binaire Semafoor Test ---\n");
    xTaskCreate(trigger_task, "TriggerTask", 2048, NULL, 5, NULL);
    xTaskCreate(wait_task, "WaitTask", 2048, NULL, 5, NULL);
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    printf("\n--- Mutex Test (Gedeelde Resource) ---\n");
    xTaskCreate(resource_task_1, "ResourceTask1", 2048, NULL, 5, NULL);
    xTaskCreate(resource_task_2, "ResourceTask2", 2048, NULL, 5, NULL);
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    printf("\n--- Counting Semafoor Test (Producer/Consumer) ---\n");
    xTaskCreate(producer_task, "ProducerTask", 2048, NULL, 5, NULL);
    xTaskCreate(consumer_task, "ConsumerTask", 2048, NULL, 5, NULL);
}

*/