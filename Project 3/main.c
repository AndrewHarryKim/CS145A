/*
 * GccApplication2.c
 *
 * Created: 4/20/2016 6:25:00 PM
 * Author : Josh
 */ 

#include "avr.h"
#include "lcd.h"
#include <stdlib.h>
#include <stdio.h>

#define DDR     DDRB
#define PORT    PORTB
#define RS_PIN  0
#define RW_PIN  1
#define EN_PIN  2


#define XTAL_FRQ 8000000lu

#define SET_BIT(p,i) ((p) |=  (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) &   (1 << (i)))

#define WDR() asm volatile("wdr"::)
#define NOP() asm volatile("nop"::)
#define RST() for(;;);


void ini_lcd(void);
void clr_lcd(void);
void pos_lcd(unsigned char r, unsigned char c);
void put_lcd(char c);
void puts_lcd1(const char *s);
void puts_lcd2(const char *s);
void ini_avr(void);
void wait_avr(unsigned short msec);
void wait_avr2(unsigned long msec);


struct note
{
	unsigned short frequency;
	unsigned short duration;
};

void playMusic(struct note **song, int n);
void playNote(float duration, float frequency);
unsigned short getFrequency(unsigned char c, unsigned char sharp);


//functions for lcd 
void
ini_avr(void)
{
	WDTCR = 15;
}
void
wait_avr(unsigned short msec)
{
	TCCR0 = 3;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 64) * 0.001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}
void
wait_avr2(unsigned long msec)
{
	TCCR0 = 2;
	while (msec--) {
		TCNT0 = (unsigned char)(256 - (XTAL_FRQ / 8) * 0.00001);
		SET_BIT(TIFR, TOV0);
		WDR();
		while (!GET_BIT(TIFR, TOV0));
	}
	TCCR0 = 0;
}


static inline void
set_data(unsigned char x)
{
	PORTD = x;
	DDRD = 0xff;
}

static inline unsigned char
get_data(void)
{
	DDRD = 0x00;
	return PIND;
}

static inline void
sleep_700ns(void)
{
	NOP();
	NOP();
	NOP();
}

static unsigned char
top(unsigned char rs)
{
	unsigned char d;
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	SET_BIT(PORT, RW_PIN);
	get_data();
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	d = get_data();
	CLR_BIT(PORT, EN_PIN);
	return d;
}

static void
output(unsigned char d, unsigned char rs)
{
	if (rs) SET_BIT(PORT, RS_PIN); else CLR_BIT(PORT, RS_PIN);
	CLR_BIT(PORT, RW_PIN);
	set_data(d);
	SET_BIT(PORT, EN_PIN);
	sleep_700ns();
	CLR_BIT(PORT, EN_PIN);
}

static void
write(unsigned char c, unsigned char rs)
{
	while (top(0) & 0x80);
	output(c, rs);
}

void
ini_lcd(void)
{
	SET_BIT(DDR, RS_PIN);
	SET_BIT(DDR, RW_PIN);
	SET_BIT(DDR, EN_PIN);
	wait_avr(16);
	output(0x30, 0);
	wait_avr(5);
	output(0x30, 0);
	wait_avr(1);
	write(0x3c, 0);
	write(0x0c, 0);
	write(0x06, 0);
	write(0x01, 0);
}

void
clr_lcd(void)
{
	write(0x01, 0);
}

void
pos_lcd(unsigned char r, unsigned char c)
{
	unsigned char n = r * 40 + c;
	write(0x02, 0);
	while (n--) {
		write(0x14, 0);
	}
}

void
put_lcd(char c)
{
	write(c, 1);
}

void
puts_lcd1(const char *s)
{
	char c;
	while ((c = pgm_read_byte(s++)) != 0) {
		write(c, 1);
	}
}

void
puts_lcd2(const char *s)
{
	char c;
	while ((c = *(s++)) != 0) {
		write(c, 1);
	}
}

//functions for timer and date

void inc_time()
{
	//assert(!"The method or operation is not implemented.");
}
//returns how many days in the month
unsigned short convert_num(unsigned short str[], int sizeOfArray){
	unsigned short k = sizeOfArray;
	unsigned short expo = 0;
	unsigned short num = 0;
	unsigned short temp = 0;
	for(unsigned short i = 0; i < sizeOfArray; i++, k--){
		for(unsigned short j = 0; j < k; j++){
			if(expo != 0)
				expo = (expo * (10));
			else
				expo = 1;
		}
		temp = str[i];
		num += (expo * (temp - 48));
		expo = 0;
	}
	return num;
}
void check_date(unsigned short month, unsigned short year, unsigned short* dd)
{
	unsigned short **days= &dd;
	if(month <= 7){
		if(month == 2){
			(*days)[0] = 50;
			//leap year
			if((year%4) == 0){
				(*days)[1] = 57;
			}else{
				(*days)[1] = 56;
			}
		}else if((month%2) == 0){
			(*days)[0] = 51;
			(*days)[1] = 48;
		}else{
			(*days)[0] = 51;
			(*days)[1] = 49;
		}
	}else{
		if((month%2) == 0){
			(*days)[0] = 51;
			(*days)[1] = 49;
		}else{
			(*days)[0] = 51;
			(*days)[1] = 48;
		}
	}
}

unsigned char sizeOf(char* string){
	unsigned char count = 0;
	for(; string[count++] != '\0';){}
	return --count;
}
int pressed(unsigned char r, unsigned char c){
	DDRC = 0x00;
	PORTC = 0xFF;
	SET_BIT(DDRC, r);
	CLR_BIT(PORTC, r);
	wait_avr(1);
	if(GET_BIT(PINC, c + 4))
		return 0;
	return 1;
}
int somethingPressed()
{
	unsigned char r,c;
	//Jaysus says nuffin pressed
	for(r = 0; r < 4; ++r){
		for(c = 0; c < 4; ++c){
			if(pressed(r,c)){
				return 1;
			}
		}
	}
	return 0;
}
unsigned char get_key(){
	unsigned char r;
	unsigned char c;
	// If A to D pressed
	for(r = 0; r < 4; ++r){
		if(pressed(r,3))
		{
			return 65+r;
		}
	}
	
	// 1 to 9 Pressed
	for(r = 0; r < 3; ++r){
		for(c = 0; c < 3; ++c){
			if(pressed(r,c)){
				return ((r*3)+c+1)+48;
			}
		}
	}
	//If zero pressed
	if(pressed(3,1)){
		return '0';
	}
	// If * pressed
	else if(pressed(3,0)){
		return 42;
	}
	// If # pressed
	else if(pressed(3,2)){
		return 35;
	}
	return '~';
}

unsigned char volLevel = 1;
int speedInc = 0;
unsigned char switchSong = 1;
int main()
{
    int n = 20;
    struct note *mySong = malloc( n * sizeof(struct note) );
	int s=100;
	ini_lcd();
	unsigned char top[] = "Happy Birthday  ";
	unsigned char top2[] = "Row Row Row     ";
	unsigned char bot[] = "Switch Song  [A]";
	
	puts_lcd2(top);
	pos_lcd(1,0);
	puts_lcd2(bot);
	
	while (1){
		unsigned char key=get_key();
		if((key =='2') && (volLevel < 3))
			volLevel++;
		if((key == '8') && (volLevel > 0))
			volLevel--;
		if((key == '6') && (speedInc < 100))
			speedInc += 50;
		if((key == '4') && (speedInc > 0))
			speedInc -= 50;
		if((key == 'A')){
			switchSong = 0;
			bot[14] = 'B';
			clr_lcd();
			pos_lcd(0,0);
			puts_lcd2(top2);
			pos_lcd(1,0);
			puts_lcd2(bot);
		}
		if((key == 'B')){
			switchSong = 1;
			bot[14] = 'A';
			clr_lcd();
			pos_lcd(0,0);
			puts_lcd2(top);
			pos_lcd(1,0);
			puts_lcd2(bot);
		}
		if(key == 'C')
			break;
		
			//happy birthday
		if(switchSong){
			wait_avr2(100);
			playNote(300,getFrequency('D', 0));
			wait_avr2(10);
			playNote(300,getFrequency('D', 0));
			playNote(340,getFrequency('E', 0));
			playNote(340,getFrequency('D', 0));
			playNote(340,getFrequency('G', 0));
			playNote(500,getFrequency('F', 0));
	
			playNote(300,getFrequency('D', 0));
			wait_avr2(10);
			playNote(300,getFrequency('D', 0));
			playNote(340,getFrequency('E', 0));
			playNote(340,getFrequency('D', 0));
			playNote(340,880);
			playNote(500,getFrequency('G', 0));
	
			playNote(340, 1174);
			playNote(340, 987);
			playNote(340,getFrequency('G', 0));
			playNote(340,getFrequency('F', 0));
			playNote(340,getFrequency('E', 0));

			playNote(340, 1046);
			wait_avr2(10);
			playNote(340, 1046);
			playNote(340, 987);
			playNote(340,getFrequency('G', 0));
			playNote(340,880);
			playNote(500,getFrequency('G', 0));
			//row row row your boat
		}else{		
			playNote(300,getFrequency('D', 0));
			wait_avr2(1000);
			playNote(300,getFrequency('D', 0));
			wait_avr2(1000);
			playNote(300,getFrequency('D', 0));
			wait_avr2(1000);
			playNote(220,getFrequency('E', 0));
			wait_avr2(1000);
			playNote(300,getFrequency('F', 1));
			wait_avr2(1000);
			playNote(300,getFrequency('F', 1));
			wait_avr2(1000);
			playNote(220,getFrequency('E', 0));
			wait_avr2(1000);
			playNote(300,getFrequency('F', 1));
			wait_avr2(1000);
			playNote(220,getFrequency('G', 0));
			wait_avr2(1000);
			playNote(500,880);
			wait_avr2(1000);
		
			playNote(300,1174);
			wait_avr2(500);
			playNote(300,1174);
			wait_avr2(500);
			playNote(300,1174);
			wait_avr2(500);
		
			playNote(300,880);
			wait_avr2(500);		
			playNote(300,880);
			wait_avr2(500);		
			playNote(300,880);
			wait_avr2(500);

			playNote(300,getFrequency('F', 1));
			wait_avr2(500);		
			playNote(300,getFrequency('F', 1));
			wait_avr2(500);
			playNote(300,getFrequency('F', 1));
			wait_avr2(500);

			playNote(300,getFrequency('D', 0));
			wait_avr2(500);
			playNote(300,getFrequency('D', 0));
			wait_avr2(500);
			playNote(300,getFrequency('D', 0));
			wait_avr2(500);
		
			playNote(300,880);
			wait_avr2(1000);
			playNote(220,getFrequency('G', 0));
			wait_avr2(1000);
			playNote(300,getFrequency('F', 1));
			wait_avr2(1000);
			playNote(220,getFrequency('E', 0));
			wait_avr2(1000);
			playNote(600,getFrequency('D', 0));
			wait_avr2(1000);
		}
	}
	
	return 0;
}

void playMusic(struct note **song, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		playNote((*song[i]).frequency, (*song[i]).duration);
	}
}

void playNote(float duration,float frequency)
{
	DDRA = 0xff;
	unsigned int i,cycles;
	double half_period;
	double wavelength;
	duration = duration - speedInc;

	wavelength=(1000/frequency);
	// Standard physics formula.

	cycles=duration/wavelength;
	//The number of cycles.

	half_period = wavelength/2;
	// The time between each toggle.
	for (i = 0; i < cycles; i++)
	{
		if(volLevel >= 1)
			SET_BIT(PORTA, 3);
		if(volLevel >= 2)
			SET_BIT(PORTA, 2);
		if(volLevel >= 3)
			SET_BIT(PORTA, 0);
		wait_avr2(half_period*100);
		if(volLevel >= 1)
			CLR_BIT(PORTA, 3);
		if(volLevel >= 2)
			CLR_BIT(PORTA, 2);
		if(volLevel >= 3)
			CLR_BIT(PORTA, 0);
		wait_avr2(half_period*100);	
	}
}

unsigned short getFrequency(unsigned char c, unsigned char sharp){
	//for regular notes
	if(!sharp){
		switch(c){
			case 'A' :
				return 440;
			case 'B' :
				return 493;			
			case 'C' :
				return 523;
			case 'D' :
				return 587;
			case 'E' :
				return 659;			
			case 'F' :
				return 698;
			case 'G' :
				return 783;											
		}
	}else{
		switch(c){
			case 'A' :
				return 466;
			case 'C' :
				return 554;
			case 'D' :
				return 622;			
			case 'F' :
				return 739;
			case 'G' :
				return 830;		
		}
	}
	return 0;
}