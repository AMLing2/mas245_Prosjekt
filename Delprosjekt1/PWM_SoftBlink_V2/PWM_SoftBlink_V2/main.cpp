/*
 * PWM_SoftBlink_V2.cpp
 *
 * Created: 25.10.2023 14:26:14
 * Author : Thomas
 */ 

// CPU frequency is 1 [MHz], after prescaler of 8 with fuse bit.
#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>	// instead of cmath, less work since std library missing
#define PI 3.14159265

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
}

int main(void)
{
	init();	// Initialize

// ------------- Main Variables -------------

	double t = 0.0;		// [seconds]
	const double f = 0.1;				// Speed of pulse, [Hz]
	const double fsRad = f*2.0*PI;		// Speed of pulse, [rad/sec]

		// Set the Duty Cycle
	double dutyCycle = 0.5;	// Default Duty Cycle
	OCR1A = 0x0;
	
// ------------- Main Variables -------------
	
    while (true) 
    {
			// Updates PWM (Duty Cycle)
		if ((dutyCycle > 0.0) & (dutyCycle < 1.0))	// Safeguard for [%]
		{ OCR1A = static_cast<uint16_t>(dutyCycle * 1023.0); }	// [bit]
		
		
			// Soft Blink sine
		t += 0.0027; // f_OCnxPWM^(-1)
		dutyCycle = (1.0 - sin(fsRad*t))*0.5; // Sets duty cycle for PWM
    }
	return 0;
}

