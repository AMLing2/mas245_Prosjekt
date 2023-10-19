/*
 * PWM_blink.cpp
 *
 * Created: 19/10/2023 12:42
 * Author : mathi
 */ 

#define F_CPU 1000000 //1MHz, set cpu frequency in Hz, datablad side...
#include <avr/io.h>
#include <stdio.h>
#define __DELAY_BACKWARD_COMPATIBLE__ //required for delay til a funke med variabel
#include <util/delay.h>
#include <math.h>


//questions: 


void init()
{
	DDRD |= (1 << DDD7); //set pin 7 as output in data-direction of port D
	DDRD &= ~(1 << PIND6); //set pin 6 of port D as output
}

void ledBrightness(const double duty)
{
	  double T = 10.0; //ms
	  //double dutyCycle = PWM/0.87; //ms
	  double onTime = duty * T; //ms
	  
	  //uint32_t onTimeint = static_cast<uint32_t>(onTime); //float to ms
	  if(onTime >= T)
	  {
		  onTime = T; // protection so not negative
	  }
	  
	  //PWM code:
	  PORTD |= (1<<PD7);
	  _delay_ms(onTime);
	  if (onTime != T)
	  {
			PORTD &= ~(1<<PD7); //low
			_delay_ms(T-onTime);
	  }
			

}

int main(void)
{
	//initialize
	init();
	PORTD &= ~(1<<PD7); //set pin 7 low
	double a = 1;
	uint8_t pin6Value = 0;
	double angle = 0; //rad
	double pwmPerc = 0;
	const double increase = 0.5;
	
	while (true) // run forever
	{
		pin6Value = (PIND & (1 << PD6)) >> PD6; // read pin 6 value, 0 if low, 1 if high
		
		//button functionality: (blink speed)
		if (pin6Value == 0 && a <= 5.0) // if pin 6 of port D is low
		{
			a = a + increase; //increase
		}
		else if (pin6Value == 0 && a > 5.0);
		{
			a = 0; //reset speed
		}
		
		//soft blink:
		pwmPerc = (1 + sin(angle))*0.5; //math for blink
		ledBrightness(pwmPerc);
		
		//increase angle
		if (angle < (2.0 * M_PI))
		{
			angle = angle + a;
		}
		else
		{
			angle = 0;
		}	
	}
	return 0;
}