/*
 * helloWorld.cpp
 *
 * Created: 24/10/2023 4:51PM
 * Author : mathi
 */ 

#define F_CPU 1000000 //1MHz, set cpu frequency in Hz, datablad side...
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>


//questions: 

void initPorts()
{
	DDRB |= (1 << DDB1); //set pin 7 as output in data-direction of port D
	PORTB &= ~(1<<PB1); //set pin 7 low as initial value
}

int main()
{
	//initialize
	initPorts();
	
	PORTB |= (1<<PB1); //high
	
	return 0;
}