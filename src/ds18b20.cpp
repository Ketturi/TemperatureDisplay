/* ds18b20.c - a part of avr-ds18b20 library
*
* Copyright (C) 2016 Jacek Wieczorek
*
* This software may be modified and distributed under the terms
* of the MIT license.	See the LICENSE file for details.
*/

#include <stddef.h>
#include <util/delay.h>
#include "../include/ds18b20/ds18b20.h"
#include "../include/ds18b20/onewire.h"

static uint8_t ds18b20crc8( uint8_t *data, uint8_t length )
{
	//Generate 8bit CRC for given data (Maxim/Dallas)

	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t mix = 0;
	uint8_t crc = 0;
	uint8_t byte = 0;

	for ( i = 0; i < length; i++ )
	{
		byte = data[i];

		for( j = 0; j < 8; j++ )
		{
			mix = ( crc ^ byte ) & 0x01;
			crc >>= 1;
			if ( mix ) crc ^= 0x8C;
			byte >>= 1;
		}
	}
	return crc;
}

static void ds18b20match( uint8_t *rom )
{
	//Perform ROM match operation on DS18B20 devices
	//Or skip ROM matching if ptr is NULL

	uint8_t i = 0;

	//If rom pointer is NULL then read temperature without matching.
	if ( rom == NULL )
	{
		//Skip ROM
		onewireWrite( DS18B20_COMMAND_SKIP_ROM );
	}
	else
	{
		//Match ROM
		onewireWrite( DS18B20_COMMAND_MATCH_ROM );
		for ( i = 0; i < 8; i++ )
		onewireWrite( rom[i] );
	}
}

uint8_t ds18b20convert( uint8_t *rom )
{
	//Send conversion request to DS18B20 on one wire bus

	//Communication check
	if ( onewireInit( ) == ONEWIRE_ERROR_COMM )
	return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match( rom );

	//Convert temperature
	onewireWrite( DS18B20_COMMAND_CONVERT );

	return DS18B20_ERROR_OK;
}

uint8_t ds18b20rsp( uint8_t *rom, uint8_t *sp )
{
	//Read DS18B20 scratchpad

	uint8_t i = 0;

	//Communication check
	if ( onewireInit( ) == ONEWIRE_ERROR_COMM )
	return DS18B20_ERROR_COMM;

	//Match (or not) ROM
	ds18b20match( rom );

	//Read scratchpad
	onewireWrite( DS18B20_COMMAND_READ_SP );
	for ( i = 0; i < 9; i++ )
	sp[i] = onewireRead( );

	//Check pull-up
	if ( *( (uint64_t*) sp ) == 0 )
	return DS18B20_ERROR_PULL;

	//CRC check
	if ( ds18b20crc8( sp, 8 ) != sp[8] )
	return DS18B20_ERROR_CRC;

	return DS18B20_ERROR_OK;
}

uint8_t ds18b20wsp( uint8_t *rom, uint8_t th, uint8_t tl, uint8_t conf )
{
	//Writes DS18B20 scratchpad
	//th - thermostat high temperature
	//tl - thermostat low temperature
	//conf - configuration byte

	//Communication check
	if ( onewireInit( ) == ONEWIRE_ERROR_COMM )
	return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match( rom );

	//Write scratchpad
	onewireWrite( DS18B20_COMMAND_WRITE_SP );
	onewireWrite( th );
	onewireWrite( tl );
	onewireWrite( conf );

	return DS18B20_ERROR_OK;
}

uint8_t ds18b20csp( uint8_t *rom )
{
	//Copies DS18B20 scratchpad contents to its EEPROM

	//Communication check
	if ( onewireInit( ) == ONEWIRE_ERROR_COMM )
	return DS18B20_ERROR_COMM;

	//ROM match (or not)
	ds18b20match( rom );

	//Copy scratchpad
	onewireWrite( DS18B20_COMMAND_COPY_SP );

	//Set pin high
	//Poor DS18B20 feels better then...
	ONEWIRE_PULLUP;
	ONEWIRE_DIR |= ONEWIRE_TX;

	return DS18B20_ERROR_OK;
}

uint8_t ds18b20read( uint8_t *rom, int16_t *temperature )
{
	//Read temperature from DS18B20
	//Note: returns actual temperature * 16

	uint8_t sp[9];
	uint8_t ec = 0;

	//Communication, pull-up, CRC checks happen here
	ec = ds18b20rsp( rom, sp );

	if ( ec != DS18B20_ERROR_OK )
	{
		*temperature = 0;
		return ec;
	}

	//Get temperature from received data
	*temperature = (int16_t)( sp[1] << 8 ) + ( sp[0] & 0xFF );

	return DS18B20_ERROR_OK;
}

uint8_t ds18b20rom( uint8_t *rom )
{
	//Read DS18B20 rom

	unsigned char i = 0;

	if ( rom == NULL ) return DS18B20_ERROR_OTHER;

	//Communication check
	if ( onewireInit( ) == ONEWIRE_ERROR_COMM )
	return DS18B20_ERROR_COMM;

	//Read ROM
	onewireWrite( DS18B20_COMMAND_READ_ROM );
	for ( i = 0; i < 8; i++ )
	rom[i] = onewireRead( );

	//Pull-up check
	if ( ( rom[0] | rom[1] | rom[2] | rom[3] | rom[4] | rom[5] | rom[6] | rom[7] ) == 0 ) return DS18B20_ERROR_PULL;

	//Check CRC
	if ( ds18b20crc8( rom, 7 ) != rom[7] )
	{
		for ( i = 0; i < 8; i++ ) rom[i] = 0;
		return DS18B20_ERROR_CRC;
	}

	return DS18B20_ERROR_OK;
}
