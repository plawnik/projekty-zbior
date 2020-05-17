/*
 * hd44780.h
 *
 *  Created on: 20 lis 2016
 *      Author: Administrator
 */

#ifndef HD44780_H_
#define HD44780_H_

// TO:DO RW PIN , 8BIT MODE , USER DEFCHAR

//*******************************************************************************************//
//						SET LCD CONNECTIONS TO MICROCONTROLLER								 //
//						AND COLUMNS AND ROWS VALUES											 //
//	 ______________________________________________________________							 //
//	|	1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16					   |	1. GND		9.  D2	 //
//	|	 ______________________________________________________	   |	2. VCC		10. D3	 //
//	|	|													   |   |	3. BIAS		11. D4	 //
//	|	|			      TYPICAL LCD HD44780      			   |   |	4. RS		12. D5	 //
//	|	|													   |   |	5. RW		13. D6	 //
//	|	|			      DISPLAY CONNECTIONS				   |   |	6. E		14. D7	 //
//	|	|______________________________________________________|   |	7. D0		15. LED+ //
//	|______________________________________________________________|	8. D1		16. LED- //
//																							 //
//*******************************************************************************************//
// COLUMNS AND ROWS
#define HD_COLUMNS 16
#define HD_ROWS 2
// CONNECTION LCD TO UC	  __
#define LCD_E_PORT B  // |__
#define LCD_E_PIN 0   // |__
                      //  __   __
#define LCD_RS_PORT B // |__| |__
#define LCD_RS_PIN 1  // | \   __|
                      //  _
#define LCD_D4_PORT B // | \ /_|
#define LCD_D4_PIN 2  // |_/   |
                      //  _   __
#define LCD_D5_PORT B // | \ |__
#define LCD_D5_PIN 3  // |_/  __|
                      //  _   __
#define LCD_D6_PORT B // | \ |__
#define LCD_D6_PIN 4  // |_/ |__|
                      //  _  ___
#define LCD_D7_PORT B // | \   /
#define LCD_D7_PIN 5  // |_/  /
//*******************************************************************************************//

//*******************************************************************************************//
// 								DEFINE USES FUNCTION
//*******************************************************************************************//
#define USE_HD_CHAR 1          //
#define USE_HD_STRING 1        //	from ram
#define USE_HD_FLASH_STRING 0  //  from flash
#define USE_HD_EEPROM_STRING 1 //	from eeprom

#define USE_HD_INT 1
#define USE_HD_HEX 0
#define USE_HD_BIN 0

//*******************************************************************************************//
// 								OTHER DEFINITION , DONT TOUCH
//*******************************************************************************************//
#if USE_HD_INT == 1 || USE_HD_HEX == 1 || USE_HD_BIN == 1
#define USE_HD_STRING 1
#endif

#if USE_HD_STRING == 1 || USE_HD_EEPROM_STRING == 1 || USE_HD_FLASH_STRING == 1
#define USE_HD_CHAR 1
#endif

#define GLUE(a, b) a##b

// E wire
// DDR
#define LCD_DDR_E_S(s) GLUE(DDR, s)
#define LCD_DDR_E LCD_DDR_E_S(LCD_E_PORT) // DDRX in LCD_DDR_E definition
// PORT
#define LCD_PORT_E_S(s) GLUE(PORT, s)
#define LCD_PORT_E LCD_PORT_E_S(LCD_E_PORT) // PORTX in LCD_PORT_E definition
// PIN
#define LCD_PIN_E_S(s) GLUE(PIN, s)
#define LCD_PIN_E LCD_PIN_E_S(LCD_E_PORT) // PORTX in LCD_PORT_E definition
// other for E wire
#define E_HIGH LCD_PORT_E |= (1 << LCD_E_PIN) // set E pin high
#define E_LOW LCD_PORT_E &= ~(1 << LCD_E_PIN) // set E pin low
#define E_IN LCD_DDR_E &= ~(1 << LCD_E_PIN)   // set E pin as input
#define E_OUT LCD_DDR_E |= (1 << LCD_E_PIN)   // set E pin as output
#define E_STATE  				(LCD_PIN_E|(1<<LCD_E_PIN)		// get state E pin

// RS wire (comments like E wire)
#define LCD_DDR_RS_S(s) GLUE(DDR, s)
#define LCD_DDR_RS LCD_DDR_RS_S(LCD_RS_PORT)

#define LCD_PORT_RS_S(s) GLUE(PORT, s)
#define LCD_PORT_RS LCD_PORT_RS_S(LCD_RS_PORT)

#define LCD_PIN_RS_S(s) GLUE(PIN, s)
#define LCD_PIN_RS LCD_PIN_RS_S(LCD_RS_PORT)

#define RS_HIGH LCD_PORT_RS |= (1 << LCD_RS_PIN)
#define RS_LOW LCD_PORT_RS &= ~(1 << LCD_RS_PIN)
#define RS_IN LCD_DDR_RS &= ~(1 << LCD_RS_PIN)
#define RS_OUT LCD_DDR_RS |= (1 << LCD_RS_PIN)
#define RS_STATE				(LCD_PIN_RS|(1<<LCD_RS_PIN)

// RW wire (comments like E wire)
#define LCD_DDR_RW_S(s) GLUE(DDR, s)
#define LCD_DDR_RW LCD_DDR_RW_S(LCD_RW_PORT)

#define LCD_PORT_RW_S(s) GLUE(PORT, s)
#define LCD_PORT_RW LCD_PORT_RW_S(LCD_RW_PORT)

#define LCD_PIN_RW_S(s) GLUE(PIN, s)
#define LCD_PIN_RW LCD_PIN_RW_S(LCD_RW_PORT)

#define RW_HIGH LCD_PORT_RW |= (1 << LCD_RW_PIN)
#define RW_LOW LCD_PORT_RW &= ~(1 << LCD_RW_PIN)
#define RW_IN LCD_DDR_RW &= ~(1 << LCD_RW_PIN)
#define RW_OUT LCD_DDR_RW |= (1 << LCD_RW_PIN)
#define RW_STATE  				(LCD_PIN_RW|(1<<LCD_RW_PIN)

// D4 wire (comments like E wire)
#define LCD_DDR_D4_S(s) GLUE(DDR, s)
#define LCD_DDR_D4 LCD_DDR_D4_S(LCD_D4_PORT)

#define LCD_PORT_D4_S(s) GLUE(PORT, s)
#define LCD_PORT_D4 LCD_PORT_D4_S(LCD_D4_PORT)

#define LCD_PIN_D4_S(s) GLUE(PIN, s)
#define LCD_PIN_D4 LCD_PIN_D4_S(LCD_D4_PORT)

#define D4_HIGH LCD_PORT_D4 |= (1 << LCD_D4_PIN)
#define D4_LOW LCD_PORT_D4 &= ~(1 << LCD_D4_PIN)
#define D4_IN LCD_DDR_D4 &= ~(1 << LCD_D4_PIN)
#define D4_OUT LCD_DDR_D4 |= (1 << LCD_D4_PIN)
#define D4_STATE  				(LCD_PIN_D4|(1<<LCD_D4_PIN)

// D5 wire (comments like E wire)
#define LCD_DDR_D5_S(s) GLUE(DDR, s)
#define LCD_DDR_D5 LCD_DDR_D5_S(LCD_D5_PORT)

#define LCD_PORT_D5_S(s) GLUE(PORT, s)
#define LCD_PORT_D5 LCD_PORT_D5_S(LCD_D5_PORT)

#define LCD_PIN_D5_S(s) GLUE(PIN, s)
#define LCD_PIN_D5 LCD_PIN_D5_S(LCD_D5_PORT)

#define D5_HIGH LCD_PORT_D5 |= (1 << LCD_D5_PIN)
#define D5_LOW LCD_PORT_D5 &= ~(1 << LCD_D5_PIN)
#define D5_IN LCD_DDR_D5 &= ~(1 << LCD_D5_PIN)
#define D5_OUT LCD_DDR_D5 |= (1 << LCD_D5_PIN)
#define D5_STATE  				(LCD_PIN_D5|(1<<LCD_D5_PIN)

// D6 wire (comments like E wire)
#define LCD_DDR_D6_S(s) GLUE(DDR, s)
#define LCD_DDR_D6 LCD_DDR_D6_S(LCD_D6_PORT)

#define LCD_PORT_D6_S(s) GLUE(PORT, s)
#define LCD_PORT_D6 LCD_PORT_D6_S(LCD_D6_PORT)

#define LCD_PIN_D6_S(s) GLUE(PIN, s)
#define LCD_PIN_D6 LCD_PIN_D6_S(LCD_D6_PORT)

#define D6_HIGH LCD_PORT_D6 |= (1 << LCD_D6_PIN)
#define D6_LOW LCD_PORT_D6 &= ~(1 << LCD_D6_PIN)
#define D6_IN LCD_DDR_D6 &= ~(1 << LCD_D6_PIN)
#define D6_OUT LCD_DDR_D6 |= (1 << LCD_D6_PIN)
#define D6_STATE  				(LCD_PIN_D6|(1<<LCD_D6_PIN)

// D7 wire (comments like E wire)
#define LCD_DDR_D7_S(s) GLUE(DDR, s)
#define LCD_DDR_D7 LCD_DDR_D7_S(LCD_D7_PORT)

#define LCD_PORT_D7_S(s) GLUE(PORT, s)
#define LCD_PORT_D7 LCD_PORT_D7_S(LCD_D7_PORT)

#define LCD_PIN_D7_S(s) GLUE(PIN, s)
#define LCD_PIN_D7 LCD_PIN_D7_S(LCD_D7_PORT)

#define D7_HIGH LCD_PORT_D7 |= (1 << LCD_D7_PIN)
#define D7_LOW LCD_PORT_D7 &= ~(1 << LCD_D7_PIN)
#define D7_IN LCD_DDR_D7 &= ~(1 << LCD_D7_PIN)
#define D7_OUT LCD_DDR_D7 |= (1 << LCD_D7_PIN)
#define D7_STATE  				(LCD_PIN_D7|(1<<LCD_D7_PIN)

// definitions to LCD
#define LCD_DAT 0x00
#define LCD_CMD 0x01
#define LCD_CLR 0x01
//*******************************************************************************************//

//*******************************************************************************************//
// 								AVILABLE FUNCTION
//*******************************************************************************************//
void hd_write_data(uint8_t mode, uint8_t byte);
void hd_init(void);
void hd_clear(void);

void hd_char(char character);
void hd_string(char *string);
void hd_flash_string(const char *string);
void hd_eeprom_string(char *string);

void hd_int(int integer);
void hd_hex(int hexadecimal);
void hd_bin(int binary);
//*******************************************************************************************//