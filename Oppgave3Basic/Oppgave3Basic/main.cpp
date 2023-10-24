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
//1. where to write DDRB
//DDRB = 0 = input?

void initPorts()
{
	DDRD |= (1 << DDD7); //set pin 7 as output in data-direction of port D
	PORTD &= ~(1<<PD7); //set pin 7 low as initial value
}

int main()
{
	//initialize
	initPorts();
	
	PORTD |= (1<<PD7); //high
	
	return 0;
}