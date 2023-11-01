/*
 * helloWorld.cpp
 *
 * Created: 25/10/2023 08:32
 * Author : mathi
 */ 

#define F_CPU 8000000 // 8 [MHz] CPU frequency
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>


//questions: 

namespace intVars
{
	volatile int count;
	volatile bool interrupted;
	volatile int timerInterrupt;
}

void init()
{
	DDRB |= (1 << DDB1);	//set pin 7 as output in data-direction of port D
	DDRD &= ~(1 << PIND6);	//set pin 6 of port D as input
	PORTB &= ~(1<<PB1);		//set pin 7 low as initial value
	PORTD |= (1<<PD6);		//set pin 6 high to enable pull up-resistor to read state
	
	// pin change interrupt:
	PCICR |= (1<<PCIE2);	// enable Pin Change Interrupt Control Register PCIE2 for PD6
	PCMSK2 |= (1<<PCINT22); // enable Pin Change Mask register for PCINT22 PD6, not external
	sei();					// enable global interrupt, enable I-bit of SREG
	
	// namespace initial:
	intVars::count = 0;
	intVars::timerInterrupt = 0;
	intVars::interrupted = false;
}

ISR(PCINT2_vect)	// Interrupt Service Routine
{
	if ((( ( PIND & (1 << PD6) ) >> PD6 ) == 0) & (intVars::timerInterrupt == 0)) //enable only if button is low/pushed down
	{
		intVars::interrupted = true;
		intVars::count++;
	}
	else //button is up
	{
		//do nothing
	}
	
}

int main(void)
{
	//initialize
	init();
	
	while (true) // run forever
	{
		if (intVars::interrupted)
		{
			intVars::timerInterrupt++;
			_delay_ms(1);
		}
		
		if (intVars::timerInterrupt >= 150)
		{
			intVars::timerInterrupt = 0;
			intVars::interrupted = false;
		}
		
		if ((intVars::count % 2) == 1) // if count is odd, turn light on, if count is even turn light off
		{
			PORTB |= (1<<PB1); //high
		}
		else
		{
			PORTB &= ~(1<<PB1); //low
		}
	}
	return 0;
}