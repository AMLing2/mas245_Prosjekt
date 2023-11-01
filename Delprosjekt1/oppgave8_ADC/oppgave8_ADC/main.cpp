/*
 * oppgave8_ADC.cpp
 *
 * Created: 25/10/2023 14:37:49
 * Author : mathi
 */ 

//#define F_CPU 1000000 //8MHZ cpu freq
//#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

//questions
//REG = 0b0000 0000 format 
//OR:
//REG |= (1<<REG1) | (1<<REG2).... format
namespace adcVars
{
	volatile bool adcCompleted;
}

void init()
{
	// --------------------- PWM Configuration ---------------------

	// Fast PWM mode - 10 [bit] (Mode 7 from table 20-6)
	// Waveform Generation Mode
	TCCR1A |=  (1 << WGM11) | (1 << WGM10);
	TCCR1B |=  (1 << WGM12);
	TCCR1B &= ~(1 << WGM13);
	
	// Set Fast PWM Mode NON-INVERTING
	// PB1 --> Only COM1An
	TCCR1A &= ~(1 << COM1A0);	// COM1A0 = 0
	TCCR1A |=  (1 << COM1A1);	// COM1A1 = 1
	
	// Set prescaler:
		// f_Clk = 16 [MHz], f_des = 250 [kHz]
		// Ratio = prescaler = N = 16/0.25 = 64
	TCCR1B |=  (1 << CS11);
	TCCR1B &= ~(1 << CS12) & ~(1 << CS10);
	
	// --------------------- PWM Configuration ---------------------
	
	// Configure Output Compare Pin: PB1 = pin 15
	DDRB |= (1 << DDB1);
	
	//LED:
	//DDRD |= (1 << DDD7); //set pin D7 as output in data-direction of port D
	//PORTD &= ~(1<<PD7); //set pin D7 low as initial value
	//BUTTON:
	DDRD &= ~(1 << PIND6); //set pin 6 of port D as input
	PORTD |= (1<<PD6); //set pin 6 high to enable pull up-resistor to read state
	
	// --------------------- ADC Configuration ---------------------
    //using pin PC3 as ADC input
	
	//ADCSRA = 0b 1010 1011; 
						  //bit 7 - enable ADC, bit 6 - start free running mode, bit 4 - enable auto trigger/ free running mode
						  //bit 3 - ADC interrupt enabled, edit last 3 bits later when PWM implemented
	ADCSRA |= (1<<ADEN) | (1<<ADATE) | (1<<ADIE) & ( ~(1<<ADPS2) ) | (1<<ADPS1) | (1<<ADPS0); 
							//ADPS = divided by 8 for 125kHz? //ADCSRA = ADC mode
	//ADMUX =  0b 0100 0011; 
						// bit 7:6-external capacitor on AREF, bit 5- right adjusted, bit 3:0- using pin PC3
	ADMUX |= (1<<REFS0)| (1<<MUX1) | (1<<MUX0); //| (1<<ADLAR); //ADMUX = pins and AREF selection
	adcVars::adcCompleted = false;
	
	//interrupts
	sei(); //enable global interrupts, enable I-bit of SREG
}

ISR(ADC_vect) //interrupt for ADC convertion completion
{ 
	adcVars::adcCompleted = true;
}

int main(void)
{
	init();
	double aVCC = 5.0; //5V reference
	double duty = 0.0;
	double adcVoltage = 0.0;
	uint16_t adc16bit = 0x0;
	//OCR1A = 0x0;
	ADCSRA |= (1<<ADSC);	//begin ADC conversion after init() completed
	
    while(true) //run forever
    {
		if(adcVars::adcCompleted) //alternatively could be ran in the ISR but i wanted to keep the ISR short
		{
			adc16bit = ADC;
			
			adcVoltage = (static_cast<double>(adc16bit) / 1024.0) * aVCC; //conversion from 10 bit uint to voltage
			duty = adcVoltage/aVCC; //conversion from voltage to 0-1 percentage for PWM
			
			adcVars::adcCompleted = false; // alternatively could read bit 6 of ADCSRA but might be interfered with by hardware
		}
		else
		{
			//do nothing, wait till next ADC completion
		}
		
		if((duty > 0.0) & (duty < 1.0))
		//if(ADC >= 512)
		{
			OCR1A = static_cast<uint16_t>(duty * 1023.0); //pwm
		}
		else
		{
			//do nothing
		}
    }
}