/*
* 1WireTempDisp.cpp
* Firmware for EKA161 Remote Display unit
* Enables display unit act as standalone DS18b20 thermometer.
* Created: 29.7.2017 18.20.52
* Author : Ketturi Electronics
*/

# define F_CPU 4000000UL

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "include/display.h"
#include "include/ds18b20/ds18b20.h"

#define READ_INTERVALL_MS 1000 //Time between temperature readings
#define OVER_TEMP 500		   //Temperature warning, degC * 10

char buffer[4] = {9, 9, 9} ; //Buffer for display output digits, initially shows "888" message

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
	OCR0A = 52; //F_CPU / 256 / 300Hz
	TCCR0A = 0x02; //Turnt on CTC mode
	TIFR |= 0x01; //Clear interupt flag
	TIMSK = 0x01; //enable timer compare interrupt
	TCCR0B = 0x04; //Set CS10 and CS12 bits for 1024 prescaler
	
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
	
	display_putc(buffer[activedisplay],dg1,dg2);
}

//ISR (INT0_vect){ //Interupt for buttons
//does nothing, but everytime triggered activedisplay corresponds button
//}

//Formats number as ascii with leading spaces
void print(int n) {
	memset(buffer, 0x00, 4); //Set leading space
	int len=0;
	int tmp = n;
	if (n<0) {
		tmp = -n;
	}
	do {
		buffer[len++] = tmp%10+1; //add number to buffer
		tmp/=10;
	} while(tmp && len<4-1);
	//Negative sign when negative number
	if (n<0)	flag_leds.led_neg = 1;
	else		flag_leds.led_neg = 0;
	// reverse numbers
	for(int i=0, j=4-2; i<j; i++, j--) {
		char c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	buffer[4-1] = 0;
}

void print_decimal(int16_t input){ //converts 99.9 float to int for screen with decimal
	int16_t output;
	if (input >= OVER_TEMP) //Set temperature warning if temperature reaches point
	flag_leds.led_1 = 1;
	
	if (input > -1000 && input < 1000){ //if value [-99.9,99.9] show with 1st decimal
		output = input; //move decimal point to left
		flag_leds.led_dec = 1;
	}
	else { //if value is [[-999,-100][100,999]] then do not show decimal
		flag_leds.led_dec = 0;
		output = input/10;
	}
	print(output);
}



// The main loop. Sets up hardware, then loops forever reading and displaying temperatures.
int main(void) {
	wdt_enable(WDTO_4S); //Enable watch dog with 4S countdown
	
	display_init(); //Initialize 7-segment display IO pins
	timer0_init();  //Initialize timer and start multiplexing display
	
	int temperature = 0;
	char errorcode = 0;
	
	ds18b20wsp( NULL, 0, 100, DS18B20_RES12); //Set resolution of sensor
	
	while((errorcode = ds18b20convert(NULL)) == DS18B20_ERROR_OK) {
		flag_leds.led_4 = 0;
		_delay_ms(READ_INTERVALL_MS); //delay of sensor reading interval
		flag_leds.led_4 = 1;
		
		if((errorcode = ds18b20read( NULL, &temperature)) == DS18B20_ERROR_OK){
			print_decimal(temperature*10/16); //Output temperature with 1 decimal
			wdt_reset(); //Reset watchdog
		}	
		else{
			break;
		}
	}

	//Show error if conversion fails and wait watchdog reset
	buffer[0] = 15;
	buffer[1] = 17;
	buffer[2] = errorcode+1;
	_delay_ms(4000);
}

