/*
 * oppgave8_ADC.cpp
 *
 * Created: 25/10/2023 14:37:49
 * Author : mathi
 */ 

#define F_CPU 1000000 //8MHZ cpu freq
#include <avr/io.h>
#include <avr/interrupt.h>

//questions
//REG = 0b0000 0000 format 
//OR:
//REG |= (1<<REG1) | (1<<REG2).... format
namespace adcVars
{
	volatile uint8_t adcUintL;
	volatile uint8_t adcUintH;
	volatile bool adcCompleted;
}

void init()
{
	//LED:
	DDRD |= (1 << DDD7); //set pin D7 as output in data-direction of port D
	PORTD &= ~(1<<PD7); //set pin D7 low as initial value
	//BUTTON:
	DDRD &= ~(1 << PIND6); //set pin 6 of port D as input
	PORTD |= (1<<PD6); //set pin 6 high to enable pull up-resistor to read state
	
	//using pin PC3 as ADC input
	
	//ADC initials:
	
	//ADCSRA = 0b 1010 1011; 
						  //bit 7 - enable ADC, bit 6 - start free running mode, bit 4 - enable auto trigger/ free running mode
						  //bit 3 - ADC interrupt enabled, edit last 3 bits later when PWM implemented
	ADCSRA |= (1<<ADEN) | (1<<ADATE) | (1<<ADIE) & ~(1<<ADPS2) | (1<<ADPS1) |(1<<ADPS0); //ADPS = divided by 8 for 125kHz
	//ADMUX =  0b 0100 0011; 
						// bit 7:6-external capacitor on AREF, bit 5- right adjusted, bit 3:0- using pin PC3
	ADMUX |= (1<<REFS0)| (1<<MUX1) | (1<<MUX0); //(1<<ADLAR) 
	adcVars::adcUintL = 0x0;
	adcVars::adcUintH = 0x0;
	adcVars::adcCompleted = false;
	
	//interrupts
	sei(); //enable global interrupts, enable I-bit of SREG
}

void ledBrightness(double duty)
{
	
}

ISR(ADC_vect) //interrupt for ADC convertion completion
{ 
	adcVars::adcUintL = ADCL; //adc low del 1 av 16 bit uint
	adcVars::adcUintH = ADCH; //adc  high del 2 av 16 bit uint
	adcVars::adcCompleted = true;
}

int main(void)
{
	init();
	double aVCC = 5.0; //5V reference
	double duty = 0.0;
	double adcVoltage = 0.0;
	uint16_t adc16bit = 0x0;
	ADCSRA |= (1<<ADSC)	//begin ADC conversion after init() completed
    while(true) //run forever
    {
		if(adcVars::adcCompleted) //alternatively could be ran in the ISR but i wanted to keep the ISR short
		{
			adc16bit = 0x0; //reset value
			//adcVars::adcUintL = (adcVars::adcUintH>>6)// shifts 
			adc16bit = static_cast<uint16_t>(adcVars::adcUintH) // bits 15:8 of adc value
			adc16bit = (adc16bit<<8)//mindre sketchy en å ha << in samme linje som over?
			adc16bit |= static_cast<uint16_t>(adcVars::adcUintL) //  bits 7:0 of adc value
			
			adcVoltage = (static_cast<double>(adc16bit) / 1024.0) * aVCC; //conversion from 10 bit uint to voltage
			duty = adcVoltage/aVCC; //conversion from voltage to 0-1 percentage for PWM
			adcVars::adcCompleted = false; // alternatively could read bit 6 of ADCSRA but might be interfered with by hardware
		}
		else
		{
			//do nothing, wait till next ADC completion
		}
		//ledBrightness(duty);
    }
}
