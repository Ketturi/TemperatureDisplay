/* onewire.c - a part of avr-ds18b20 library
*
* Copyright (C) 2016 Jacek Wieczorek
* Modified by Ketturi Electronics (2017)
*
* This software may be modified and distributed under the terms
* of the MIT license.	See the LICENSE file for details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#include "../include/ds18b20/onewire.h"

uint8_t onewireInit()
{
	//Init one wire bus (it's basically reset pulse)

	uint8_t response = 0;
	uint8_t sreg = SREG; //Store status register

	cli( ); //Disable interrupts
	
	DDRD |= (1<< PD1); //Set TX as output
	ONEWIRE_PULLDOWN; //Pull onewire line low
	
	_delay_us( 600 );
	
	ONEWIRE_DIR &= ~ONEWIRE_RX; //Set RX port to input
	ONEWIRE_PULLUP;

	_delay_us( 100 );

	response = ONEWIRE_PIN & ONEWIRE_RX; //Read input

	_delay_us( 200 );

	ONEWIRE_PULLUP;

	_delay_us( 600 );

	SREG = sreg; //Restore status register

	return response != 0 ? ONEWIRE_ERROR_COMM : ONEWIRE_ERROR_OK;
	sei();
}

inline uint8_t onewireWriteBit( uint8_t bit )
{
	
	uint8_t sreg = SREG;

	cli( );

	ONEWIRE_PULLDOWN; //Write 0 to onewire line
	
	if ( bit != 0 ) _delay_us( 10 ); //Set timeslot start delay
	else _delay_us( 80 );

	ONEWIRE_PULLUP; //Set line up

	if ( bit != 0 ) _delay_us( 80 ); //Set timeslot stop delay
	else _delay_us( 5 );

	SREG = sreg;

	return bit != 0;
}

void onewireWrite( uint8_t data )
{
	//Write byte to one wire bus

	uint8_t sreg = SREG; //Store status register
	uint8_t i = 0;

	cli( );

	for ( i = 1; i != 0; i <<= 1 ) //Write byte in 8 single bit writes
	onewireWriteBit( data & i );

	SREG = sreg;
}

inline uint8_t onewireReadBit()
{
	uint8_t bit = 0;
	uint8_t sreg = SREG;

	cli( );
	
	ONEWIRE_PULLDOWN;
	
	_delay_us( 8 );
	
	ONEWIRE_DIR &= ~ONEWIRE_RX; //Set RX port to input
	ONEWIRE_PULLUP; //Set onewire pullup
	
	_delay_us( 8 );
	bit = ( ( ONEWIRE_PIN & ONEWIRE_RX ) != 0 ); //Read input
	_delay_us( 60 );
	SREG = sreg;

	return bit;
}

uint8_t onewireRead()
{
	//Read byte from one wire data bus

	uint8_t sreg = SREG; //Store status register
	uint8_t data = 0;
	uint8_t i = 0;

	cli( ); //Disable interrupts

	for ( i = 1; i != 0; i <<= 1 ) //Read byte in 8 single bit reads
	data |= onewireReadBit() * i;

	SREG = sreg;

	return data;
}
