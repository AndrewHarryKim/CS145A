/*
 * GccApplication2.c
 *
 * Created: 4/20/2016 6:25:00 PM
 * Author : Andrew Kim
 			Josh Alpert
 */ 
#include "avr.h"
#include "lcd.h"

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
int main()
{
	
	ini_lcd();
	unsigned char top[] = "11:59:40 PM     ";
	unsigned char bot[] = "09/30/2016      ";
	unsigned short  mm[2] = {48,57};
	unsigned short  dd[2] = {51,48};
	unsigned short  yy[4] = {50,48,49,54};
					
	puts_lcd2(top);
	pos_lcd(1,0);
	puts_lcd2(bot);
	
	unsigned short hr[2] = {49,49};
	unsigned short min[2] = {53,57};
	unsigned short sec[2] = {52,48};
	unsigned char *place=bot;
	char i=0;
	while(1){
		unsigned char key=get_key();
		if(key =='~')
		{
			if(sec[1] != '9'){
				sec[1]++;
				top[7] = sec[1];
			}else if((sec[0] != '5')&&(sec[1] == '9')){
				sec[0]++;
				sec[1] = '0';
				top[6] = sec[0];
				top[7] = sec[1];
			}else if((sec[0] == '5')&&(sec[1] == '9')&&(min[1] != '9')){
				sec[0] = '0';
				sec[1] = '0';
				min[1]++;
				top[4] = min[1];
				top[6] = '0';
				top[7] = '0';
			}else if((min[0] != '5') && (min[1] == '9')){
				sec[0] = '0';
				sec[1] = '0';
				min[0]++; 
				min[1] = '0'; 
				top[3] = min[0];
				top[4] = '0';
				top[6] = '0';
				top[7] = '0';
			}else if((min[0] == '5')&&(min[1] == '9')&&(hr[0] != '1')){
				sec[0] = '0';
				sec[1] = '0';
				min[0] = '0';
				min[1] = '1';
				if(hr[1] == '9')
					hr[0] = '1';
				else
					hr[1]++;
				top[1] = hr[1];
				top[3] = '0';
				top[4] = '0';
				top[6] = '0';
				top[7] = '0';
			}else if ((hr[0] == '1') && (hr[1] != '2')){
				if(hr[1]=='1')
				{
					//TODO: Make sure to add functionality to inc days in date
					if(top[9]=='A')
					{
						top[9]='P';
					}
					else
					{
						inc_time();
						top[9]='A';
						unsigned short temp[2] = {48,49};
						check_date(convert_num(mm,2), convert_num(yy,4), temp);
						if(convert_num(dd, 2) == convert_num(temp,2)){
							//(convert_num(dd, 2) == 31){
							//last day of the month
							if((mm[0] == '1')&&(mm[1] == '2')){
								//last day of the year
								if(yy[3] != '9')
									yy[3]++;
								else if(yy[2] != '9'){
									yy[2]++;
									yy[3] = '0';
								}else if(yy[1] != '9'){
									yy[1]++;
									yy[2] = '0';
									yy[3] = '0';
								}else{
									yy[0]++;
									yy[1] = '0';
									yy[2] = '0';
									yy[3] = '0';
								}
								mm[0] = '0';
								mm[1] = '1';
								dd[0] = '0';
								dd[1] = '1';
							}else{
								if((mm[0] != '1')&&(mm[1] != '9'))
									mm[1]++;
								else if((mm[0] == '1')&&(mm[1] != '2'))
									mm[1]++;
								else if(mm[1] == '9'){
									mm[0] ='1';
									mm[1] ='0';
								}else{
									mm[0] = '0';
									mm[1] = '1';
								}
								dd[0] = '0';
								dd[1] = '1';		
							}
						}else{
							if(dd[1] != '9')
								dd[1]++;
							else{
								dd[0]++;
								dd[1] = '0';
							}		
						}
					}
				}
				sec[0] = '0';
				sec[1] = '0';
				min[0] = '0';
				min[1] = '0';
				hr[1]++;
				top[1] = hr[1];
				top[3] = '0';
				top[4] = '0';
				top[6] = '0';
				top[7] = '0';
			}else{
				
				sec[0] = '0';
				sec[1] = '0';
				min[0] = '0';
				min[1] = '0';
				hr[0] =	 '0';
				hr[1] =	 '1';
				top[0] = hr[0];	
				top[1] = '1';
				top[3] = '0';
				top[4] = '0';
				top[6] = '0';
				top[7] = '0';
			}

			wait_avr(1000);
			clr_lcd();
			puts_lcd2(top);
			pos_lcd(1,0);
			
			bot[0] = mm[0];
			bot[1] = mm[1];
			bot[3] = dd[0];
			bot[4] = dd[1];
			bot[6] = yy[0];
			bot[7] = yy[1];
			bot[8] = yy[2];
			bot[9] = yy[3];
			puts_lcd2(bot);
							
		}else
		{
			unsigned char change_time = 0;
			unsigned char change_date = 0;
			unsigned char new_text[16] = "  :  :   14A 24P";
			unsigned char new_date[16] = "  /  /          ";

			*place = new_text;
			while(1){
				key = get_key();
				if(key !='~')
				{
					if(key == '*'){
						change_time = 1;
						change_date = 0;
					}
					if(change_time == 1){						
						if(key != '*'){						
							new_text[i] = key;
							if(i < 2){
								hr[i] = new_text[i];
								top[i] = hr[i];
								if(i == 1){
									if((convert_num(hr,2) > 12)||(convert_num(hr,2) == 0))
										i = -1;
								}
							}
							else if(i < 5){
								min[i-3] = new_text[i];
								top[i] = min[i-3];
								if(i == 4){
									if(convert_num(min,2) > 59)
										i = 2;
								}
							}
							else if(i < 8){
								sec[i-6] = new_text[i];
								top[i] = sec[i-6];
								if(i == 7){
									if(convert_num(sec,2) > 59)
										i = 5;
								}
							}else if(i == 9){
								if(key == '1'){
									top[9] = 'A';
								}else if(key == '2'){
									top[9] = 'P';	
								}else{
									//need to reinput
									--i;
								}
							}
							++i;
							if((i == 2) ||(i == 5)){
								top[i] = ':';
								++i;
							}
							if(i==8){
								top[i] = ' ';
								++i;
							}
							if(i ==10){
								top[i] = 'M';
								++i;	
							}
							
							new_text[i] = '_';

						}
						clr_lcd();
						pos_lcd(0,0);
						puts_lcd2(top);
						pos_lcd(1,0);
						puts_lcd2(new_text);
						
						if(i >= 10){
							i = 0;
							change_time = 0;
							break;
						}
						while(somethingPressed()==1){}
					} 
	
					
					if(key == '#'){
						change_date = 1;
						change_time = 0;
					}
					if(change_date == 1){
						if(key != '#'){
							new_date[i] = key;					
							if(i < 2){
								mm[i] = new_date[i];
								bot[i] = mm[i];
								if(i == 1){
									if((convert_num(mm,2) > 12) || (convert_num(mm,2) == 0)) 
										i = -1;
								}
							}
							else if(i < 5){
								dd[i-3] = key;
								bot[i] = dd[i-3];
								if(convert_num(dd,2) == 0)
									i = 2; 
							}
							else if(i < 10){
								yy[i-6] = key;
								bot[i] = yy[i-6];
								unsigned short temp2[2] = {48,49};
								check_date(convert_num(mm,2), convert_num(yy,4), temp2);
								if(i == 9){
									if(convert_num(dd, 2) > convert_num(temp2,2))
										i = 2;
								}
							}
							++i;
							if((i == 2) ||(i == 5)){
								bot[i] = '/';
								++i;
							}

						}
						clr_lcd();
						pos_lcd(0,0);
						puts_lcd2(new_date);
						pos_lcd(1,0);
						puts_lcd2(bot);
						
						if(i == 10){
							i = 0;
							change_date = 0;
							puts_lcd2(bot);
							break;
						}
						while(somethingPressed()==1){}

					} else if((change_date == 0)&&(change_time == 0))
						break;
					key='~';
				}
			}
		}
				
	}
	return 0;
}

