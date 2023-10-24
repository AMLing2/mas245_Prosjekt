/*
 * PWM_blink.cpp
 *
 * Created: 19/10/2023 12:42
 * Author : mathi
 */ 

#define F_CPU 1000000 //1MHz, set cpu frequency in Hz, datablad side...
#include <avr/io.h>
#include <stdio.h>
#define __DELAY_BACKWARD_COMPATIBLE__ //required for delay til a funke med en variabel
#include <util/delay.h>
#include <math.h>


//questions: 


void init()
{
	DDRD |= (1 << DDD7); //set pin 7 as output in data-direction of port D
	DDRD &= ~(1 << PIND6); //set pin 6 of port D as input
	PORTD &= ~(1<<PD7); //set pin 7 low
	PORTD |= (1<<PD6); //set pin 6 high to enable pull up-resistor to read state
}

void ledBrightness(const double dutyCycle,const double freq)
{
	  const double T = (1.0/freq) * static_cast<double>(10^3); //time in ms for 1 PWM cycle, f = 1/T
	  double onTime = dutyCycle * T; //ms
	  
	  //uint32_t onTimeint = static_cast<uint32_t>(onTime); //float to uint ms
	  if(onTime >= T)
	  {
		  onTime = T; // protection so not negative time
	  }
	  
	  //PWM code:
	  PORTD |= (1<<PD7); //D7 high
	  _delay_ms(onTime);
	  if (onTime != T)
	  {
			PORTD &= ~(1<<PD7); //D7 low
			_delay_ms(T-onTime);
	  }
}

int main(void)
{
	//initialize
	init();
	//main variables:
	double a = 1.0;
	uint8_t pin6Value = 0;
	double angle = 0.0; //rad
	double pwmPerc = 0.0;
	const double increase = 0.5;
	
	while (true) // run forever
	{
		pin6Value = (PIND & (1 << PD6)) >> PD6; // read pin 6 value, 0 if low, 1 if high
		
		//button functionality: change blink speed
		if (pin6Value == 0 && a <= 5.0) // if pin 6 of port D is low
		{
			a = a + increase; //increase angle increaser
		}
		else if (pin6Value == 0 && a > 5.0);
		{
			a = 0.0; //reset speed
		}
		
		//soft blink:
		//pwmPerc = (1 + sin(angle))*0.5; //math for blink
		pwmPerc = 0.5;
		ledBrightness(pwmPerc,120.0);
		
		//increase angle
		if (angle < (2.0 * M_PI))
		{
			angle = angle + a;
		}
		else
		{
			angle = 0.0;
		}
	}
	return 0;
}