/*
 * PWM_blink.cpp
 *
 * Created: 19/10/2023 12:42
 * Author : mathi
 */ 

#define F_CPU 1000000 // 1 [MHz], set cpu frequency in [Hz], datablad side...
#include <avr/io.h>
#include <stdio.h>
#define __DELAY_BACKWARD_COMPATIBLE__ // Required for delay til a funke med en variabel
#include <util/delay.h>
#include <math.h>


void init()
{
	DDRB |= (1 << DDB1); //set pin 7 as output in data-direction of port D
	PORTB &= ~(1<<PB1);  //set pin 7 low
}

void ledBrightness(double dutyCycle,const double T)
{	
	  double onTime = dutyCycle * T; // [ms]
		
	  if(onTime >= T)
	  {
		  onTime = T; // protection so not negative time
	  }
	  
	  //PWM code:
	  PORTB |= (1<<PB1); //D7 high
	  _delay_ms(onTime);
      PORTB &= ~(1<<PB1); //D7 low
	  _delay_ms(T-onTime);
}

int main(void)
{
	//initialize
	init();
	//main variables:
	double t = 0.0;
	const double fs = 0.5;			  // Speed of pulse, [Hz]
	const double fsRad = fs*2.0*M_PI; // Speed of pulse, [rad/sec]
	
	const double pwmFreq = 490.0;		  // PWM speed, [Hz]
	const double T = (1000.0/pwmFreq); // [ms] for 1 PWM cycle, f = 1/T
	
	double dCycle = 0.0;
	
	while (true) // run forever
	{
		
		//soft blink:
		t = t + 0.01;
		dCycle = (1.0 - sin(fsRad*t))*0.5; // Sets duty cycle for PWM
		
		ledBrightness(dCycle, T);
	}
	return 0;
}