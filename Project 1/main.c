/*
 * BlinkingLed.c
 *
 * Created: 4/6/2016 11:29:26 AM
 * Author : Josh Alpert
 *			Andrew Kim
 */ 

//#include <avr/io.h>
#include "avr.h"

void
wait(unsigned int msec)
{
	
	volatile int i;
	//Loops for the amount of cycles of i
	//Determined by the crystal clock speed or micro controller clock
	for(i = 0; i < msec; ++i){}
}

void
wait_avr(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);
		SET_BIT(TIFR, TOV0);
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}

int main(void)
{
    /* Replace with your application code */
	// 0 is Input 1 is Output
	//DDR[Var Name]= [x][x][x][x][x][x][x][x]
	//Initially set to [0][0][0][0][0][0][0][0]
	DDRA = 0x01; // Pin Values: [0][0][0][0][0][0][0][1]
    DDRD = 0;	 // Pin Values: [0][0][0][0][0][0][0][0]
	while (1) 
    {
		//Check if input
		if(!GET_BIT(PIND,7)){
			// Turns ON LED
			PORTA = 0x01;
			wait_avr(500);
			// Turns OFF LED
			PORTA = 0x00;
			wait_avr(500);
		}
		if(!GET_BIT(PIND, 6)){
			// Turns ON LED
			PORTA = 0x01;
			wait(40000);
			// Turns OFF LED
			PORTA = 0x00;
			wait(40000);
		}
	}
}

