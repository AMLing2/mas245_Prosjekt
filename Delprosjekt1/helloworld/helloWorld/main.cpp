/*
 * helloWorld.cpp
 *
 * Created: 17/10/2023 10:43:29
 * Author : mathi
 */ 

#define F_CPU 1000000 //1MHz, set cpu frequency in Hz, datablad side...
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>


//questions: 
//1. where to write DDRB
//DDRB = 0 = input?

void init()
{
	DDRB |= (1 << DDB1); //set pin B1 as output in data-direction of port D
	DDRD &= ~(1 << PIND6); //set pin D6 of port D as input
	PORTB &= ~(1<<PB1); //set pin B1 low as initial value
	PORTD |= (1<<PD6); //set pin D6 high to enable pull up-resistor to read state
}

void blink()
{
		PORTB |= (1<<PB1); //high
		_delay_ms(100);
		PORTB &= ~(1<<PB1); //low
		_delay_ms(100);
}

int main(void)
{
	//initialize
	init();
	
	while (true) // run forever
	{
		if ((( ( PIND & (1 << PD6) ) >> PD6 ) == 0)) //enable only if button is low/pushed down
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