/*
 * 1WireTempDisp.cpp
 * Firmware for Remote Display unit
 * Firmware enables display unit act as standalone
 * Temperature display
 * Created: 29.7.2017 18.20.52
 * Author : Ketturi Electronics
 */ 

# define F_CPU 8000000UL

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "display.h"
#include "ds1820.h"

char digit[4] = {14, 16, 16} ; //Buffer for display output digits, initially shows "Err" message

//Sets indicator leds in bitfield
struct indicator_leds {
	unsigned int led_1 : 1; //upmost indicator dot
	unsigned int led_2 : 1; //upper indicator dot
	unsigned int led_3 : 1; //lower indicator dot
	unsigned int led_4 : 1; //lowest indicator dot
	unsigned int led_neg : 1; //Negative sign
	unsigned int led_dec : 1; //1st decimal point
};
struct indicator_leds flag_leds;

void timer0_init() //Set and start multiplex timer
{  //Runs around 300Hz which should be fine update speed (around 100Hz for whole display)
	cli(); //disable global interrupts
	
	//set compare match register to desired timer count:
	OCR0A = 25; //F_CPU / 1024 / 50Hz
	TCCR0A = 0x02; //Turnt on CTC mode
	TIFR |= 0x01; //Clear interupt flag
	TIMSK = 0x01; //enable timer compare interrupt
	TCCR0B = 0x05; //Set CS10 and CS12 bits for 1024 prescaler
	
	//Set button interrupt input
	//GIMSK |=  (1 << INT0); //Enable INT0 vector
	//MCUCR |=  (1 << ISC01) | (1 <<ISC00); //Trigger on rising edge
	
	sei(); //enable global interrupts
}

// Timer call for refreshing display
ISR (TIMER0_COMPA_vect){
	int dg1 = 0, dg2 = 0;
	
	uint8_t activedisplay = display_selnextdigit();
	
	switch(activedisplay){ //Decode status leds to multiplex matrix
		case 0:
			dg2 = flag_leds.led_neg;
			dg1 = flag_leds.led_1;
		break;
		case 1:
			dg2 = flag_leds.led_dec;
			dg1 = flag_leds.led_3;
		break;
		case 2:
			dg2 = flag_leds.led_2;
			dg1 = flag_leds.led_4;
		break;
	}
	
	display_putc(digit[activedisplay],dg1,dg2);
}

//ISR (INT0_vect){ //does nothing at this moment
//}

//Formats number as ascii with leading spaces
void formatnumber(int n, char* buf, int bufsize) {
	memset(buf, 0x00, bufsize); //set leading space
	int len=0;
	int tmp = n;
	if (n<0) {
		tmp = -n;
	}
	do {
		buf[len++] = tmp%10 +'0';
		tmp/=10;
	} while(tmp && len<bufsize-1);
	/*if (tmp || len==bufsize-1 && n<0) {
		// error, number too large
		memset(buf, 'E', bufsize);
		buf[bufsize-1] = 0;
		return;
	}*/
	//Negative sign when negative number
	if (n<0)	flag_leds.led_neg = 1;
	else		flag_leds.led_neg = 0;
	// reverse numbers
	for(int i=0, j=bufsize-2; i<j; i++, j--) {
		char c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
	buf[bufsize-1] = 0;
}

int floattodec(signed int input){ //converts 99.9 float to int for screen with decimal
	short int output;
	if (input > -100 && input < 100){ //if value [-99.9,99.9] show with 1st decimal
		output = input * 10; //move decimal point to left
		flag_leds.led_dec = 1;
	}
	else { //if value is [[-999,-100][100,999]] then do not show decimal
		flag_leds.led_dec = 0;
		output = input;
	}
	return output;
}


/** The main loop. Sets up the 1-wire bus, then loops forever reading and displaying temperatures. */
int main(void) {
	display_init(); //Initialise 7-segment display IO pins
	timer0_init();  //Initialize timer and start multiplexing display
	static OWI_device devices[MAX_DEVICES];
	OWI_device *	  ds1820;
	signed int		  temperature = 0;
		
	OWI_Init(BUSES);
	// Do the bus search until all ids are read without crc error.
	while (SearchBuses(devices, MAX_DEVICES, BUSES) != SEARCH_SUCCESSFUL) {
	}

	// See if there is a DS1820 on a bus.
	ds1820 = FindFamily(DS1820_FAMILY_ID, devices, MAX_DEVICES);
			
	for(;;){ 
		if (ds1820 != NULL) {
			temperature =  DS1820_ReadTemperature((*ds1820).bus, (*ds1820).id) / 2;
			formatnumber(floattodec(temperature), digit, 4); //formats value to ascii for 7-segment
		}
		_delay_ms(500);		

	}
}

