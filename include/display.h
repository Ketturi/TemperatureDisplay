/*
* display.h
* Header file for 7-segment display driver
* Created: 29.7.2017 21.21.28
*  Author: Ketturi Electronics
*/


#ifndef display_H_
#define display_H_

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define LED_SEG_PORT	PORTB
#define LED_SEG_DDR		DDRB
#define LED_AUX_PORT	PORTD
#define LED_AUX_DDR		DDRD

#define LED_A   PB7 //Active LOW!
#define LED_B   PB6
#define LED_C   PB5
#define LED_D   PB4
#define LED_E   PB3
#define LED_F   PB2
#define LED_G   PB1
#define LED_DG1 PB0 //DP1, DP2, DP3
#define LED_DG2 PD6 //S1,  DP4, DP5

#define LED_CA1 PD5 //Active LOW!
#define LED_CA2 PD4
#define LED_CA3 PD3

//array of anode outputs
#define LED_CA_ARRAY {LED_CA1, LED_CA2, LED_CA3}

//Button pin, buttons connected to LED_CA1 and LED_CA2.
//Must be read with interrupt while multiplexing
#define BUT_INT PD2

#define US_ONTIME 1 //Time that digit is active and showing output, affects brigtness and duty cycle

//functions
extern void display_init();
extern void display_setfirstdigit();
extern uint8_t display_getactivedigit();
extern void display_putc(char, uint8_t, uint8_t);
extern void display_puthex(uint8_t);
extern uint8_t display_selnextdigit();

#endif /* display_H_ */