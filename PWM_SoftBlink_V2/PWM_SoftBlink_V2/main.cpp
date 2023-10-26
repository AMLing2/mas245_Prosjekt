/*
 * PWM_SoftBlink_V2.cpp
 *
 * Created: 25.10.2023 14:26:14
 * Author : Thomas
 */ 

#define F_CPU 8000000	// 8 [MHz] from datasheet: ATMEGA168-20PU
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void init()
{
	// Define LED-pin
	//const uint8_t ledPin = PB1;	// Pin 15 on ATMEGA168-20PU
	
  // ----------------- PWM Configuration -----------------

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
	

  // ----------------- PWM Configuration -----------------
  
}

int main(void)
{
	init();	// Initialize

    while (true) 
    {
		// Do nothing.
    }
	return 0;
}

