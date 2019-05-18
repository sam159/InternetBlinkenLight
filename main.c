/*
Copyright 2019 Sam Stevens

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */ 

#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

#define POLY_MASK_16 0xD295

ISR(WDT_vect)
{
	//Keep watchdog in interrupt mode
	WDTCR |= (1<<WDIE);
}

uint16_t lsfr_16;

uint16_t lsfr_shift(uint16_t *val, uint16_t mask) {
	uint16_t feedback;
	
	feedback = *val & 1;
	*val >>= 1;
	if (feedback == 1) {
		*val ^= mask;
	}
	return *val;
}

int main(void)
{
	//Setup
	DDRB |= (1<<DDB0);
	PORTB = 0;
	
	lsfr_16 = 0x5555;
	
	//Power reduction
	PRR |= (1<<PRTIM0) | (1<<PRTIM1) | (1<<PRUSI) | (1<<PRADC);
	//Run at 8MHz/64 = 125KHz
	clock_prescale_set(clock_div_64);
	
	//Start watchdog
	wdt_enable(WDTO_250MS);
	//Interrupt rather than reset
	WDTCR |= (1<<WDIE);
	
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	
    //Loop-de-loop
    while (1) 
    {
		if ((lsfr_shift(&lsfr_16, POLY_MASK_16) & 1) == 1) {
			PORTB |= 1<<PORTB0;
		}
		else {
			PORTB &= ~(1<<PORTB0);
		}
		sleep_enable();
		sei();
		sleep_mode();
		sleep_disable();
    }
}

