/*
 * helloWorld.cpp
 *
 * Created: 17/10/2023 10:43:29
 * Author : mathi
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

//questions: 
//1. where to write DDRB
//DDRB = 0 = input?

void init()
{
	DDRB |= (1 << DDB3); //set pin 3 as output in data-direction of port B
	DDRB &= ~(1 << PB2); //set pin 2 of port B as output?
}

void blink()
{
			PORTB |= (1<<PB3); //high
			_delay_ms(100);
			PORTB &= ~(1<<PB3); //low
			_delay_ms(100);
}

int main(void)
{
	//initialize
	init();
	PORTB &= ~(1<<PB3); //set pin 3 low
	bool a = true;
	uint8_t pin2Value = 0;
	
	while (true) // run forever
	{
		pin2Value = (PINB & (1 << PB2)) >> PB2; // read pin 2 value, 0 if low, 1 if high
		
		if (pin2Value == 1) // if pin 2 of port B is high
		{
			a = ~a; //turn on or off blinking
		}
		if (a)
		{
			blink();
		}
		
	}
	return 0;
}