/*
 * display.cpp
 * 7-Segment simple display driver
 * Created: 29.7.2017 21.21.04
 * Author : Ketturi Electronics
 */ 

#include "display.h"
uint8_t display_digits[] = LED_CA_ARRAY;
volatile uint8_t display_activedigit = 0;

//Table of possible numbers and characters
static const uint8_t PROGMEM segment_table[] ={
	 //ABCDEFGP
	 0b11111100, //0
	 0b01100000, //1
	 0b11011010, //2
	 0b11110010, //3
	 0b01100110, //4
	 0b10110110, //5
	 0b10111110, //6
	 0b11100000, //7
	 0b11111110, //8
	 0b11100110, //9
	 0b11101110, //A
	 0b00111110, //b
	 0b10011100, //C
	 0b01111010, //d
	 0b10011110, //E
	 0b10001110, //F
	 0b00001010, //r
	 0b11111111  //TestAll
};

//Initialize display control pins
void display_init()
{
	//Set direction as output
	LED_SEG_DDR = 0xFF; //Cathode output
	LED_AUX_DDR |= (1<< LED_CA1);  // Anode output
	LED_AUX_DDR |= (1<< LED_CA2);
	LED_AUX_DDR |= (1<< LED_CA3);
	LED_AUX_DDR |= (1<< LED_DG2);
	
	LED_AUX_DDR &= ~(1<< BUT_INT); // Button input
	LED_SEG_PORT = 0xFF; //Turn all segments off
	
	LED_AUX_PORT |= (1<< LED_CA1);  // Set all anodes off
	LED_AUX_PORT |= (1<< LED_CA2);
	LED_AUX_PORT |= (1<< LED_CA3);
	LED_AUX_PORT |= (1<< LED_DG2);
}

//Jump back to 1st digit
void display_setfirstdigit()
{
	display_activedigit = 0;
}

//Outputs ascii character to segment lines
void display_putc(char c, uint8_t dg1, uint8_t dg2)
{	
    char cdisp;
	if (c == 0x00)
		cdisp = 0x00;
    else if (c >= 48 && c <=57)
		cdisp = pgm_read_byte(&segment_table[c-48]);
	else
		cdisp = pgm_read_byte(&segment_table[c]);
	
	if (dg1)
		cdisp |= (1 << 0);
	if (dg2)
		LED_AUX_PORT &= ~(1<< LED_DG2);
	else
		LED_AUX_PORT |=  (1<< LED_DG2);
	
	
	LED_SEG_PORT = ~cdisp;
	_delay_us(US_ONTIME);
}

//Outputs raw hex to segments
void display_puthex(uint8_t hex)
{
	LED_SEG_PORT = ~hex;
	_delay_us(US_ONTIME);
}

//Multiplexing, turns of display and then jumps to next display. Scanning left to right.
uint8_t display_selnextdigit()
{
	//Blank segments
	LED_SEG_PORT= 0xFF; 

	//Turn previous digit off
	LED_AUX_PORT |= (1 << display_digits[display_activedigit]);
	
	if (display_activedigit < sizeof(display_digits)-1)
		display_activedigit++;
	else
		display_activedigit = 0;
		
    //Turn new digit on
	LED_AUX_PORT &= ~(1 << display_digits[display_activedigit]);
	return display_activedigit;
}