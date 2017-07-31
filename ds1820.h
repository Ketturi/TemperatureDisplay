/*
 * ds1820.h
 * 1-wire implementation include
 * Created: 31.7.2017 13.34.10
 * Author : Ketturi Electronics
 */ 


#ifndef DS1820_H_
#define DS1820_H_

 #include "1wire/polled/OWIPolled.h"
 #include "1wire/polled/OWIHighLevelFunctions.h"
 #include "1wire/polled/OWIBitFunctions.h"
 #include "1wire/common_files/OWIcrc.h"


 
 #define DS1820_FAMILY_ID			0x10
 #define DS1820_START_CONVERSION	0x44
 #define DS1820_READ_SCRATCHPAD		0xbe
 #define DS1820_ERROR				-1000   // Return code. Outside temperature range.

 #define SEARCH_SUCCESSFUL			0x00
 #define SEARCH_CRC_ERROR			0x01

 #define FALSE	0
 #define TRUE	1

 #define MAX_DEVICES	4       //!< Max number of devices to search for.
 #define BUSES  (OWI_PIN_0)		//!< Buses to search.// If more than one devices, can be changed as #define BUSES   (OWI_PIN_0) |
								//!(OWI_PIN_1) etc
 
 /** 
 	*  The OWI_device data type holds information about what bus each device 
	*  is connected to, and its 64 bit identifier.
	*/
 typedef struct OWI_device { 
 	unsigned char bus;      //!< A bitmask of the bus the device is connected to.
 	unsigned char id[8];    //!< The 64 bit identifier.
 } OWI_device;

 //function prototypes
 unsigned char SearchBuses(OWI_device * devices, unsigned char len, unsigned char buses);
 OWI_device * FindFamily(unsigned char familyID, OWI_device * devices, unsigned char size);
 signed int DS1820_ReadTemperature(unsigned char bus, unsigned char * id);

 
/*! \brief  Perform a 1-Wire search
 * 
 *  This function shows how the OWI_SearchRom function can be used to
 *  discover all slaves on the bus. It will also CRC check the 64 bit
 *  identifiers.
 * 
 *  \param  devices Pointer to an array of type OWI_device. The discovered
 *                  devices will be placed from the beginning of this array.
 * 
 *  \param  len     The length of the device array. (Max. number of elements).
 * 
 *  \param  buses   Bitmask of the buses to perform search on.
 * 
 *  \retval SEARCH_SUCCESSFUL   Search completed successfully.
 *  \retval SEARCH_CRC_ERROR    A CRC error occured. Probably because of noise
 *                              during transmission.
 */
unsigned char SearchBuses(OWI_device * devices, unsigned char len, unsigned char buses) {
	unsigned char i, j;
	unsigned char presence;
	unsigned char * newID;
	unsigned char * currentID;
	unsigned char currentBus;
	unsigned char lastDeviation;
	unsigned char numDevices;
	
	// Initialize all addresses as zero, on bus 0 (does not exist).
	// Do a search on the bus to discover all addresses.
	for (i = 0; i < len; i++) {
		devices[i].bus = 0x00;
		for (j = 0; j < 8; j++) {
			devices[i].id[j] = 0x00;
		}
	}
	
	// Find the buses with slave devices.
	presence = OWI_DetectPresence(BUSES);
	
	numDevices = 0;
	newID = devices[0].id;
	
	// Go through all buses with slave devices.
	for (currentBus = 0x01; currentBus; currentBus <<= 1) {
		lastDeviation = 0;
		currentID = newID;
		if (currentBus & presence) { // Devices available on this bus.
			// Do slave search on each bus, and place identifiers and corresponding
			// bus "addresses" in the array.
			do {
				memcpy(newID, currentID, 8);
				OWI_DetectPresence(currentBus);
				lastDeviation = OWI_SearchRom(newID, lastDeviation, currentBus);
				currentID = newID;
				devices[numDevices].bus = currentBus;
				numDevices++;
				newID=devices[numDevices].id;
			}  while (lastDeviation != OWI_ROM_SEARCH_FINISHED);
		}
	}
	
	// Go through all the devices and do CRC check.
	for (i = 0; i < numDevices; i++) {
		// If any id has a crc error, return error.
		if(OWI_CheckRomCRC(devices[i].id) != OWI_CRC_OK) {
			return SEARCH_CRC_ERROR;
		}
	}
	// Else, return Successful.
	return SEARCH_SUCCESSFUL;
}

/*! \brief  Find the first device of a family based on the family id
 * 
 *  This function returns a pointer to a device in the device array
 *  that matches the specified family.
 * 
 *  \param  familyID    The 8 bit family ID to search for.
 * 
 *  \param  devices     An array of devices to search through.
 * 
 *  \param  size        The size of the array 'devices'
 * 
 *  \return A pointer to a device of the family.
 *  \retval NULL    if no device of the family was found.
 */
OWI_device * FindFamily(unsigned char familyID, OWI_device * devices, unsigned char size) {
	unsigned char i = 0;
	
	// Search through the array.
	while (i < size) {
		// Return the pointer if there is a family id match.
		if ((*devices).id[0] == familyID) {
			return devices;
		}
		devices++;
		i++;
	}
	// Else, return NULL.
	return NULL;
}


/*! \brief  Read the temperature from a DS1820 temperature sensor.
 * 
 *  This function will start a conversion and read back the temperature
 *  from a DS1820 temperature sensor.
 * 
 *  \param  bus A bitmask of the bus where the DS1820 is located.
 * 
 *  \param  id  The 64 bit identifier of the DS1820.
 * 
 *  \return The 16 bit signed temperature read from the DS1820.
 * The DS1820 must have Vdd pin connected for this code to work.
 */
signed int DS1820_ReadTemperature(unsigned char bus, unsigned char * id) {
	signed int temperature;
	
	// Reset, presence.
	if (!OWI_DetectPresence(bus)) {
		return DS1820_ERROR; // Error
	}
	// Match the id found earlier.
	OWI_MatchRom(id, bus);
	// Send start conversion command.
	OWI_SendByte(DS1820_START_CONVERSION, bus);
	// Wait until conversion is finished.
	// Bus line is held low until conversion is finished.
	while (!OWI_ReadBit(bus))
		;
	
	// Reset, presence.
	if (!OWI_DetectPresence(bus)) {
		return -1000; // Error
	}
	// Match id again.
	OWI_MatchRom(id, bus);
	// Send READ SCRATCHPAD command.
	OWI_SendByte(DS1820_READ_SCRATCHPAD, bus);
	// Read only two first bytes (temperature low, temperature high)
	// and place them in the 16 bit temperature variable.
	temperature = OWI_ReceiveByte(bus);
	temperature |= (OWI_ReceiveByte(bus) << 8);
	
	return temperature;
}

#endif /* DS1820_H_ */