# ESP_WSPR_TEMPERATURE
The program transmits the current temperature every hour, the temperature is encoded in the transmission time.
There are 30 2 minute slots in an hour, from 00-18, 20-38, 40-58 minutes. Which offer the opportunity to have a
number from 000 to 999 with just three WSPR transmissions.

The software is just a playground to create WSPR transmissions with an ESP8266, OLED display and a Si5351 synthesizer
to output the WSPR message from low to 160 MHz.