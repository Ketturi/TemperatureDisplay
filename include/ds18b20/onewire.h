/* onewire.h - a part of avr-ds18b20 library
*
* Copyright (C) 2016 Jacek Wieczorek
* Modified by Ketturi Electronics (2017)
*
* This software may be modified and distributed under the terms
* of the MIT license.  See the LICENSE file for details.
*/

#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <inttypes.h>
#include <avr/io.h>

#define ONEWIRE_ERROR_OK 	0
#define ONEWIRE_ERROR_COMM 	1
#define ONEWIRE_DIR			DDRD
#define ONEWIRE_PORT		PORTD
#define ONEWIRE_PIN			PIND
#define ONEWIRE_RX			(1 << PD0)
#define ONEWIRE_TX			(1 >> PD1)
#define ONEWIRE_PULLDOWN    PORTD &= ~(1<< PD1)
#define ONEWIRE_PULLUP		PORTD |=(1<< PD1)

extern uint8_t onewireInit(void);
extern inline uint8_t onewireWriteBit( uint8_t bit );
extern void onewireWrite( uint8_t data );
extern inline uint8_t onewireReadBit();
extern uint8_t onewireRead();

#endif
