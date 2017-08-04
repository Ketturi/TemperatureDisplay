/* search.h - a part of avr-ds18b20 library
 *
 * Copyright (C) 2016 Jacek Wieczorek
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef DS18B20_ROMSEARCH_H
#define DS18B20_ROMSEARCH_H

#include <inttypes.h>

extern uint8_t ds18b20search( uint8_t *romcnt, uint8_t *roms, uint16_t buflen );

#endif
