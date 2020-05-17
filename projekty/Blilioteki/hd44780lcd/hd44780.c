/*
 * hd44780.c
 *
 *  Created on: 20 lis 2016
 *      Author: Administrator
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "hd44780.h"
#include <stdlib.h>

static inline void hd_dir_out(void);
static void hd_nibble(uint8_t nibble);

static inline void hd_dir_out(void) {
  D4_OUT;
  D5_OUT;
  D6_OUT;
  D7_OUT;
}

// prepare data pins to send nibble(4bits)
static void hd_nibble(uint8_t nibble) {
  (nibble & (1 << 0)) ? (D4_HIGH) : (D4_LOW);
  (nibble & (1 << 1)) ? (D5_HIGH) : (D5_LOW);
  (nibble & (1 << 2)) ? (D6_HIGH) : (D6_LOW);
  (nibble & (1 << 3)) ? (D7_HIGH) : (D7_LOW);
}

// writing data to lcd
void hd_write_data(uint8_t mode, uint8_t byte) {
  // outs
  hd_dir_out();
  // mode commend or data
  mode ? (RS_LOW) : (RS_HIGH);
  // send older nibble
  E_HIGH;
  hd_nibble(byte >> 4);
  E_LOW;
  // send younger nibble
  E_HIGH;
  hd_nibble(byte);
  E_LOW;
  mode ? (_delay_ms(2)) : (_delay_us(40)); // delay
}

// function to init lcd
void hd_init(void) {
  // pins as outs and low state
  E_OUT;
  E_LOW;
  RS_OUT;
  RS_LOW;
  _delay_ms(15); // wake to power up
  // init sequence for 4bit bus
  E_HIGH;
  hd_nibble(3); // 8 bit mode
  E_LOW;
  _delay_ms(5);
  E_HIGH; // one more time
  E_LOW;
  _delay_us(100);
  E_HIGH; // second time
  E_LOW;
  _delay_us(100);
  E_HIGH;
  hd_nibble(2); // 4bit mode
  E_LOW;
  _delay_us(100);
  // default config display
  hd_write_data(LCD_CMD, 0b00101000); // 4bit mode, 5x7 font
  hd_write_data(LCD_CMD, 0b00001100); // turn on display
  hd_write_data(LCD_CMD, 0b00000110); // shift cursor

  hd_clear(); // clear display
}

void hd_clear(void) { hd_write_data(LCD_CMD, LCD_CLR); }

#if USE_HD_CHAR == 1
void hd_char(char character) { hd_write_data(LCD_DAT, character); }
#endif

#if USE_HD_STRING
void hd_string(char *string) {
  while (*string)
    hd_char(*string++);
}
#endif

#if USE_HD_FLASH_STRING == 1
void hd_flash_string(const char *string) {
  register char buffer;
  while ((buffer = pgm_read_byte(string++)))
    hd_char(buffer);
}
#endif

#if USE_HD_EEPROM_STRING == 1
void hd_eeprom_string(char *string) {
  register char buffer = 1;
  while (1) {
    buffer = eeprom_read_byte((uint8_t *)string++);
    //((buffer == 0x00) || (buffer == 0xFF)) ? break : hd_char(buffer);
    if ((buffer == 0x00) || (buffer == 0xFF))
      break;
    hd_char(buffer);
  }
}
#endif

#if USE_HD_INT == 1
void hd_int(int integer) {
  char buffer[7];
  itoa(integer, buffer, 10); // data , destination , system
  hd_string(buffer);
}
#endif

#if USE_HD_HEX == 1
void hd_hex(int hexadecimal) {
  char buffer[5];
  itoa(hexadecimal, buffer, 16); // data , destination , system
  hd_string(buffer);
}
#endif

#if USE_HD_BIN == 1
void hd_bin(int binary) {
  char buffer[17];
  itoa(binary, buffer, 2); // data , destination , system
  hd_string(buffer);
}
#endif

Format!Style:
C++ online code formatter © 2014 by KrzaQ

Powered by vibe.d, the D language and clang-format