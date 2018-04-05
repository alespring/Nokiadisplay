/*

	Demo of glcd library with AVR8 microcontroller
	
	Tested on a custom made PCB (intended for another project)

	See ../README.md for connection details

*/

#include <avr/io.h>
#include "glcd/glcd.h"
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "glcd/fonts/Liberation_Sans15x21_Numbers.h"
#include "glcd/fonts/font5x7.h"
#include <avr/pgmspace.h>
#define F_CPU 16000000UL  // 1 MHz
#define ERROR 1
#define SUCCESS 0
#define EEWRITE 0xA0
#define EEREAD 0xA1

//
//avr 12c Interface-----------------------------------------------------------

//initialise TWI---
void TWIInit(void)
{
	//set SCL to 400kHz
	TWSR = 0x00;
	TWBR = 0x0C;
	//enable TWI
	TWCR = (1 << TWEN);
}//-----------------


//send start signal---
void TWIStart(void)
{	
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while ((TWCR & (1 << TWINT)) == 0);
}//--------------------


//send stop signal---
void TWIStop(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void TWIWrite(uint8_t u8data)
{
	TWDR = u8data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while((TWCR & (1 << TWINT)) == 0);
}//-------------------



//Write status---
uint8_t TWIReadNACK(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while((TWCR & (1 << TWINT)) == 0);
	return TWDR;
}

//reading status
//read byte with NACK
uint8_t TWIGetStatus(void)
{
	uint8_t status;
	//mask status
	status = TWSR & 0xF8;
	return status;
}


//implement EEPROM byte write function---
uint8_t EEWriteByte(uint16_t u16addr, uint8_t u8data)
{
	uint8_t addr_1, addr_h;
	addr_1 = u16addr;
	addr_h = (u16addr >> 8);
	
	TWIStart();
	if(TWIGetStatus() != 0x08)
		return ERROR;
		TWIWrite(EEWRITE);
		
	//select devise and send A2 A1 A0 address bits
	//TWIWrite((EEWRITE) | ((uint8_t)((u16addr & 0x0700)>>7)));
	if(TWIGetStatus() != 0x18)
		return ERROR;
		TWIWrite(addr_1);
		
	//send the rest od address
	if(TWIGetStatus() != 0x28)
	{
		return ERROR;
		TWIWrite(addr_h);
	}
	
	//write byte to eeprom
	if(TWIGetStatus() != 0x28)
	{
		return ERROR;
		TWIWrite(u8data);
	}
	
	if(TWIGetStatus() != 0x28)
		return ERROR;
		TWIStop();
	return SUCCESS;
}//---------------------------------------

//implement EEPROM byte read function
uint8_t EEReadByte(uint16_t u16addr )
{

	uint8_t addr_1, addr_h;
	addr_1 = u16addr;
	addr_h = (u16addr>>8);
	uint8_t u8data = 0;
	
	TWIStart();
	if(TWIGetStatus() != 0x08)
		return 2;
		TWIWrite(EEWRITE);
	
		//select devise and send A2 A1 A address bits
		if(TWIGetStatus() != 0x18)
			return 5;
			TWIWrite(addr_1);
		
		//send the rest of address
		if(TWIGetStatus() != 0x28)
			return 5;
			TWIWrite(addr_h);
		
		//write byte to eeprom
		TWIWrite(u8data);
		if(TWIGetStatus() != 0x28)
			return 5;
	TWIStart();
	
	if(TWIGetStatus() != 0x10)
		return 5;
		TWIWrite(EEREAD);
		
		if(TWIGetStatus != 0x40)
		return 5;
		u8data = TWIReadNACK();
		
	if(TWIGetStatus() != 0x58)
		return 7;
	TWIStop();
	
	//return SUCCESS;
	return u8data;
}


//--------------------------------------------------------------------------



/* Function prototypes */
static void setup(void);

static void setup(void)
{
	/* Set up glcd, also sets up SPI and relevent GPIO pins */
	glcd_init();
}

uint8_t ms, ms10,ms100,sec,min,entprell, state;



ISR (TIMER1_COMPA_vect)
{
	ms10++;
	if(entprell != 0)entprell--;
	if(ms10==10)	//10ms
	{
		ms10=0;
		ms100++;
	}
    if(ms100==10)	//100ms
	{
		ms100=0;
		sec++;
	}
	if(sec==10)	//Minute
	{
		sec=0;
		min++;
		if(state==11)state=10;
	}
}

/*const unsigned char bitmap[] PROGMEM= 
{ 
	 0x3e, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 
	 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 
	 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xf8, 0xf8, 
	 0xf8, 0xf8, 0xc0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfc, 
	 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 
	 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 
	 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x3e, 0x28, 0x00, 0x00, 0x01, 0x03, 
	 0x07, 0x07, 0x07, 0x8f, 0xcf, 0xcf, 0xcf, 0xcf, 0xc7, 0xc7, 0xe7, 
	 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 
	 0xf3, 0xfb, 0xfb, 0xf3, 0x87, 0x5f, 0x7f, 0xff, 0xff, 0xff, 0xfa, 
	 0xf8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xf8, 0xff, 
	 0xff, 0xff, 0xff, 0x3f, 0x83, 0xa3, 0xf3, 0xfb, 0xf3, 0xf3, 0xf3, 
	 0xf3, 0xf3, 0xf3, 0xf3, 0xe3, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xc7, 
	 0xc7, 0xc7, 0xcf, 0xcf, 0xcf, 0xcf, 0x8f, 0x07, 0x07, 0x03, 0x03, 
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
	 0x07, 0x0f, 0x1f, 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x9f, 
	 0x9f, 0x9f, 0xcf, 0xcf, 0xe7, 0xe7, 0xf7, 0xf3, 0xfb, 0xfb, 0xf9, 
	 0xfc, 0xfc, 0xf4, 0xf1, 0x47, 0x1f, 0x1f, 0x3f, 0xff, 0xfe, 0xf8, 
	 0xf8, 0xf8, 0xfe, 0x7e, 0x7f, 0x3f, 0x1f, 0x67, 0x65, 0xf1, 0xfc, 
	 0xfc, 0xfc, 0xf9, 0xfb, 0xf3, 0xe7, 0xe7, 0xe7, 0xcf, 0xcf, 0xcf, 
	 0x9f, 0x9f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1f, 0x0f, 
	 0x07, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, 0x0f, 0x0f, 0x1f, 0x3f, 0x3f, 
	 0x3f, 0x3f, 0x3f, 0x1f, 0x1f, 0x1f, 0x0f, 0xe7, 0xf5, 0xf8, 0xfc, 
	 0xff, 0xff, 0x7f, 0x7f, 0x3e, 0x01, 0x01, 0x01, 0x01, 0x01, 0x16, 
	 0x3e, 0x7f, 0xff, 0xfe, 0xfe, 0xfd, 0xfb, 0xe7, 0xa7, 0x0f, 0x1f, 
	 0x1f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x1f, 0x0f, 0x07, 0x02, 
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	 0x00, 0x00, 0x00, 0xf0, 0x90, 0x90, 0x90, 0x60, 0x00, 0x00, 0x80, 
	 0x40, 0x40, 0x40, 0x80, 0x00, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 
	 0x40, 0x80, 0x00, 0x00, 0x40, 0x40, 0x40, 0x80, 0x00, 0x00, 0xc0, 
	 0x80, 0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x10, 0x10, 
	 0x90, 0x90, 0xa0, 0x00, 0x00, 0xc0, 0x40, 0x40, 0x00, 0x80, 0x48, 
	 0x40, 0x48, 0x80, 0x00, 0xc0, 0x80, 0x40, 0x40, 0x80, 0x00, 0x80, 
	 0x40, 0x40, 0x40, 0x80, 0x00, 0xc0, 0x80, 0x40, 0x40, 0x00, 0x00, 
	 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
	 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x03, 0x04, 0x04, 0x04, 0x03, 
	 0x00, 0x07, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 
	 0x07, 0x05, 0x05, 0x07, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07, 
	 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x04, 0x04, 0x04, 0x03, 0x00, 
	 0x00, 0x07, 0x00, 0x00, 0x00, 0x03, 0x04, 0x04, 0x04, 0x03, 0x00, 
	 0x07, 0x00, 0x00, 0x00, 0x07, 0x00, 0x03, 0x05, 0x05, 0x05, 0x01, 
	 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 };*/
 
 
int main(void)
{

	uint8_t test = 22;
	char string [30] = " ";
	
	
	 //Backlight pin PL3, set as output, set high for 100% output 
	DDRB |= (1<<PB2);
	//PORTB |= (1<<PB2);
	PORTB &= ~(1<<PB2);
	
	DDRC &= ~(1<<PC0); 	//Eingang Hallsensor
	PORTC |= (1<<PC0);	//Pullup Hallsensor einschalten
	
	DDRC |=(1<<PC1); 	//Eingang Hallsensor
	PORTC |= (1<<PC1);	//Pullup Hallsensor einschalten
	
	
	DDRD &= ~((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//Taster 1-3
	PORTD |= ((1<<PD6) | (1<<PD2) | (1<<PD5)); 	//PUllups für Taster einschalten
	
	DDRD &= ~(1<<PD4); //T0 Counter Input
	TCCR0B |= (1<<CS02) | (1<<CS01) | (1<<CS00);//Counter 0 enabled clock on rising edge
	
	//Timer 1 Configuration
	OCR1A = 0x009C;	//OCR1A = 0x3D08;==1sec
	
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    //Set interrupt on compare match

    TCCR1B |= (1 << CS12) | (1 << CS10);
    // set prescaler to 1024 and start the timer

    sei();
    // enable interrupts
	
	setup();
	
	glcd_clear();
	glcd_write();
	
	min=1;
	//eeprom
	TWIInit();
	EEWriteByte(50,80);
	delay_ms(500);
	test = EEReadByte(50);
	
	//uint8_t variable = 224;//Wert definieren

	//Display
	glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
	glcd_clear_buffer();
	sprintf(string, "%d", test);
	glcd_draw_string_xy(0, 0, string);
	glcd_write();//Wert ausgeben
	
	


	while(1) 
	{
		/*switch(2)
		{
			case 1:	glcd_test_circles();
					break;
			case 2:	glcd_test_counter_and_graph();
					break;
			case 3:	glcd_test_text_up_down();
					break;
			case 4:	glcd_test_tiny_text();
					break;
			case 5:	glcd_test_hello_world();
					break;
			case 6:	glcd_test_rectangles();
					break;
			case 7:	glcd_test_scrolling_graph();
					break;
		}//end of switch*/
		
	glcd_write();
	
	}//End of while
	
	return 0;
}//end of main
