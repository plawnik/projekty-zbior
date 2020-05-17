
#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "pcd8544.h"

const char ascii[114][5] __attribute__((progmem));

//******************************************************************************************************
//					Funkcja wysyłająca bajt do wyświetlacza przy
//							pomocy prorgamowego SPI
//	ARGUMENTY:  uint8_t dc - wybór między danymi(0), a komendą sterującą(1)
//				uint8_t data - przesyłane dane/komenda sterująca
//******************************************************************************************************
void pcd_write_byte(uint8_t dc,
                    uint8_t data) // pierwszy argument 1 - komenda , 0 - dane
{
  uint8_t i;
  if (dc)
    LCD_DC_CLR;
  else
    LCD_DC_SET;
  _delay_us(1); // min 100ns
  LCD_CE_CLR; // enable bus
  //_delay_us(2);		
  for (i = 0b10000000; i; i = i >> 1) {
    if (data & i)
      LCD_DIN_SET;
    else
      LCD_DIN_CLR;
    _delay_us(1); // min 100ns
    LCD_CLK_SET;
    _delay_us(1); // min 250ns
    LCD_CLK_CLR;
    //_delay_us(2);
  }
  LCD_CE_SET; // disable bus
}
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja inicjalizująca wyświetlacz
//******************************************************************************************************
void pcd_init(void) // must be first in program
{
  // wszystkie linie jako wyjścia
  DDR(LCD_CLKPORT) |= (1 << LCD_CLK);
  DDR(LCD_DCPORT) |= (1 << LCD_DC);
  DDR(LCD_DINPORT) |= (1 << LCD_DIN);
  DDR(LCD_CEPORT) |= (1 << LCD_CE);
  DDR(LCD_RSTPORT) |= (1 << LCD_RST)
  LCD_RST_CLR;
  _delay_ms(30); // max 30ms
  LCD_RST_SET;
  LCD_CE_SET;
  pcd_write_byte(PCD_COMMAND, 0x21); //* Rozszerzone rozkazy LCD
  pcd_write_byte(PCD_COMMAND, 0xC8); //* Ustawienie LCD Vop (Kontrast) 0b1xxx xxxx
  pcd_write_byte(PCD_COMMAND, 0x06); //* Ustawienie stabilizacji Temp
  pcd_write_byte(PCD_COMMAND, 0x13); //* LCD tryb bias 1:48 0b0001 0xxx
  pcd_write_byte(PCD_COMMAND, 0x20); //* LCD Standard Commands,Horizontal addressing mode
  pcd_write_byte(PCD_COMMAND, 0x0C); //* LCD w tryb normal mode
  pcd_cls();
}
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja czyszcząca wyświetlacz
//******************************************************************************************************
void pcd_cls(void) {
  // pcd_home();
  uint16_t i;

  for (i = 0; i < 505; i++) {
    pcd_write_byte(PCD_DATA, 0);
  }

  // ustawienie kursora w pozycji 0,0 zastepuje pcd_home
  pcd_write_byte(PCD_COMMAND, 0x40);
  pcd_write_byte(PCD_COMMAND, 0x80);
}
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja ustawiajaca kontrast
//	ARGUMENTY: uint8_t contrast - wartość zadanego kontrastu, przyjmuje
//				liczby z przedzialu 0-127
//******************************************************************************************************
#if USE_PCD_CONTRAST == 1
void pcd_contrast(uint8_t contrast) // u mnie dziaĹ‚a na 0xC6
{
  pcd_write_byte(PCD_COMMAND, 0x21); // uruchomienie rozszerzonych komend
  pcd_write_byte(PCD_COMMAND, 0x80 | contrast);
  pcd_write_byte(PCD_COMMAND, 0x20); //* LCD Standard Commands,Horizontal addressing mode
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja ustawiająca kursor w pozycji 0,0
//******************************************************************************************************
#if USE_PCD_HOME == 1
void pcd_home(void) {
  pcd_write_byte(PCD_COMMAND, 0x40); // wysokoĹ›Ä‡/high    0<y<5
  pcd_write_byte(PCD_COMMAND, 0x80); // szerokosc/width	0<x<83
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja ustawiająca kursor w zadanej
//pozycji
// ARGUMENTY:  uint8_t y - wybór wiersza, przyjmowane wartosci 0-5
//			   uint8_t x - wybór kolumny, przyjmuje wartości 0-13
//******************************************************************************************************
#if USE_PCD_LOCATE == 1
void pcd_locate(uint8_t y,
                uint8_t x) // x - kolumny 14(0 to 13), y - wiersze 6(0 to 5)
{
  if (y > 5)
    y = 5;
  if (x > 13)
    x = 13;

  pcd_write_byte(PCD_COMMAND, 0x40 | y);     // wiersze/lines    y = 0 to 5
  pcd_write_byte(PCD_COMMAND, 0x80 | x * 6); // kolumny/columns	x = 0 to 13
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja ustawiająca kursor w zadanej pozycji,
//wersja rozszerzona
// ARGUMENTY:  uint8_t y - wybór wiersza, przyjmowane wartosci 0-5
//			   uint8_t x - wybór odległosci od poczatku linii, przyjmuje
//				wartości 0-83
//******************************************************************************************************
#if USE_PCD_LOCATE_EX == 1
void pcd_locate_ex(uint8_t y, uint8_t x) {

  if (y > 5)
    y = 5;
  if (x > 83)
    x = 83;

  pcd_write_byte(PCD_COMMAND, 0x40 | y); // wiersze/lines    y = 0 to 5
  pcd_write_byte(PCD_COMMAND, 0x80 | x); // kolumny/columns	x = 0 to 83
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja odwracająca kolory pikesli
// 	ARGUMENTY: uint8_t onof - 1 on 0 of 
//******************************************************************************************************
#if USE_PCD_INVERT == 1
void pcd_invert(uint8_t onof) {
  if (onof) {
    pcd_write_byte(PCD_COMMAND, 0b00001101);
  } else {
    pcd_write_byte(PCD_COMMAND, 0b00001100);
  }
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca string do wyświetlacza z
//					pamięci RAM
//******************************************************************************************************
#if USE_PCD_INT == 1 || USE_PCD_STR == 1 || USE_PCD_HEX == 1 || USE_PCD_BIN == 1
void pcd_str(char *str) {

  register char znak;
  while ((znak = *(str++))) // dopóki nie koniec stringa /0
  {
    uint8_t i, j;
    for (i = 0; i < 5; i++) {
      j = pgm_read_byte(&(ascii[znak - 32][i]));
      pcd_write_byte(PCD_DATA, j);
    }
    pcd_write_byte(PCD_DATA, 0);
  }
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca liczbę dziesiętną do
//					wyświetlacza
//******************************************************************************************************
#if USE_PCD_INT == 1
void pcd_int(int a) {

  char buf[8];
  pcd_str(itoa(a, buf, 10));
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca liczbę szesnastkową do
//					wyświetlacza
//******************************************************************************************************
#if USE_PCD_HEX == 1
void pcd_hex(int a) {

  char buf[7];
  pcd_str(itoa(a, buf, 16));
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca liczbę binarną do
//					wyświetlacza
//******************************************************************************************************
#if USE_PCD_BIN == 1
void pcd_bin(int a) {

  char buf[20];
  pcd_str(itoa(a, buf, 2));
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca pojedyńczy znak z pamięci
//					RAM
//******************************************************************************************************
#if USE_PCD_CHAR == 1
void pcd_char(char symbol) {
  uint8_t i, j;
  // char polish[] ={ 'ą' , 'ć' , 'ę' , 'ł' , 'ń' , 'ó' , 'ś' , 'ź' , 'ż' , 'Ą'
  // , 'Ć' , 'Ę' , 'Ł' , 'Ń' , 'Ó' , 'Ś' , 'Ź' , 'Ż' };
  for (i = 0; i < 5; i++) {
    j = pgm_read_byte(&(ascii[symbol - 32][i]));
    pcd_write_byte(PCD_DATA, j); // w
  }
  pcd_write_byte(PCD_DATA, 0); // odstep miedzy znakami
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca stringa z pamięci
//					FLASH
//******************************************************************************************************
#if USE_PCD_STR_P == 1
void pcd_str_P(const char *str) {
  register char znak;
  while ((znak = pgm_read_byte(str++))) {
    uint8_t i, j;
    for (i = 0; i < 5; i++) {
      j = pgm_read_byte(&(ascii[znak - 32][i]));
      pcd_write_byte(PCD_DATA, j); // w
    }
    pcd_write_byte(PCD_DATA, 0); // w
  }
}
#endif
//-----------------------------------------------------------------------------------------------------

//******************************************************************************************************
//					Funkcja wysyłajaca stringa z pamięci
//					EEPROM
//******************************************************************************************************
#if USE_PCD_STR_E == 1
void pcd_str_E(char *str) {
  register char znak;
  while (1) {
    znak = eeprom_read_byte((uint8_t *)(str++));
    if (!znak || znak == 0xFF)
      break;
    else {
      uint8_t i, j;
      for (i = 0; i < 5; i++) {
        j = pgm_read_byte(&(ascii[znak - 32][i]));
        pcd_write_byte(PCD_DATA, j); // w
      }
      pcd_write_byte(PCD_DATA, 0); // w
    }
  }
}
#endif
//-----------------------------------------------------------------------------------------------------

#if USE_PCD_CHAR == 1 || USE_PCD_INT == 1 || USE_PCD_STR_P == 1 ||             \
    USE_PCD_STR == 1 || USE_PCD_STR_E == 1 || USE_PCD_HEX == 1 ||              \
    USE_PCD_BIN == 1
// tablica ze znakami ascii
const char ascii[114][5] __attribute__((progmem)) = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* spacja */
    {0x00, 0x00, 0x2f, 0x00, 0x00}, /* ! */
    {0x00, 0x07, 0x00, 0x07, 0x00}, /* " */
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, /* # */
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, /* $ */
    {0xc4, 0xc8, 0x10, 0x26, 0x46}, /* % */
    {0x36, 0x49, 0x55, 0x22, 0x50}, /* & */
    {0x00, 0x05, 0x03, 0x00, 0x00}, /* ' */
    {0x00, 0x1c, 0x22, 0x41, 0x00}, /* ( */
    {0x00, 0x41, 0x22, 0x1c, 0x00}, /* ) */

    {0x14, 0x08, 0x3E, 0x08, 0x14}, /* * */
    {0x08, 0x08, 0x3E, 0x08, 0x08}, /* + */
    {0x00, 0x00, 0x50, 0x30, 0x00}, /* , */
    {0x10, 0x10, 0x10, 0x10, 0x10}, /* - */
    {0x00, 0x60, 0x60, 0x00, 0x00}, /* . */
    {0x20, 0x10, 0x08, 0x04, 0x02}, /* / */
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */

    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */
    {0x00, 0x36, 0x36, 0x00, 0x00}, /* : */
    {0x00, 0x56, 0x36, 0x00, 0x00}, /* ; */
    {0x08, 0x14, 0x22, 0x41, 0x00}, /* < */
    {0x14, 0x14, 0x14, 0x14, 0x14}, /* = */

    {0x00, 0x41, 0x22, 0x14, 0x08}, /* > */
    {0x02, 0x01, 0x51, 0x09, 0x06}, /* ? */
    {0x32, 0x49, 0x59, 0x51, 0x3E}, /* @ */
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, /* G */

    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */

    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, /* W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
    {0x07, 0x08, 0x70, 0x08, 0x07}, /* Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */
    {0x00, 0x7F, 0x41, 0x41, 0x00}, /* [ */

    {0x02, 0x04, 0x08, 0x10, 0x20}, /* \ */
    {0x00, 0x41, 0x41, 0x7F, 0x00}, /* ] */
    {0x04, 0x02, 0x01, 0x02, 0x04}, /* ^ */
    {0x40, 0x40, 0x40, 0x40, 0x40}, /* _ */
    {0x00, 0x01, 0x02, 0x04, 0x00}, /* ' */
    {0x20, 0x54, 0x54, 0x54, 0x78}, /* a */
    {0x7F, 0x48, 0x44, 0x44, 0x38}, /* b */
    {0x38, 0x44, 0x44, 0x44, 0x20}, /* c */
    {0x38, 0x44, 0x44, 0x48, 0x7F}, /* d */
    {0x38, 0x54, 0x54, 0x54, 0x18}, /* e */

    {0x08, 0x7E, 0x09, 0x01, 0x02}, /* f */
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, /* g */
    {0x7F, 0x08, 0x04, 0x04, 0x78}, /* h */
    {0x00, 0x44, 0x7D, 0x40, 0x00}, /* i */
    {0x20, 0x40, 0x44, 0x3D, 0x00}, /* j */
    {0x7F, 0x10, 0x28, 0x44, 0x00}, /* k */
    {0x00, 0x41, 0x7F, 0x40, 0x00}, /* l */
    {0x7C, 0x04, 0x18, 0x04, 0x78}, /* m */
    {0x7C, 0x08, 0x04, 0x04, 0x78}, /* n */
    {0x38, 0x44, 0x44, 0x44, 0x38}, /* o */

    {0x7C, 0x14, 0x14, 0x14, 0x08}, /* p */
    {0x08, 0x14, 0x14, 0x18, 0x7C}, /* q */
    {0x7C, 0x08, 0x04, 0x04, 0x08}, /* r */
    {0x48, 0x54, 0x54, 0x54, 0x20}, /* s */
    {0x04, 0x3F, 0x44, 0x40, 0x20}, /* t */
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, /* u */
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, /* v */
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, /* w */
    {0x44, 0x28, 0x10, 0x28, 0x44}, /* x */
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, /* y */

    {0x44, 0x64, 0x54, 0x4C, 0x44}, /* z */

    {0x00, 0x08, 0x36, 0x41, 0x00}, /* { */
    {0x00, 0x00, 0x7f, 0x00, 0x00}, /* | */
    {0x00, 0x41, 0x36, 0x08, 0x00}, /* } */
    {0x10, 0x08, 0x08, 0x10, 0x08}, /* ~ */
    {0x00, 0x00, 0x00, 0x00, 0x00}, /* user def(ascii DEL)"*/
    {0x20, 0x54, 0x54, 0xd4, 0x78}, /* ą */
    {0x38, 0x44, 0x46, 0x45, 0x20}, /* ć */
    {0x38, 0x54, 0x54, 0xd4, 0x18}, /* ę */
    {0x00, 0x49, 0x7f, 0x44, 0x00}, /* ł */

    {0x7c, 0x08, 0x06, 0x05, 0x78}, /* ń */
    {0x38, 0x44, 0x46, 0x45, 0x38}, /* ó */
    {0x48, 0x54, 0x56, 0x55, 0x20}, /* ś */
    {0x44, 0x64, 0x56, 0x4d, 0x44}, /* ź */
    {0x44, 0x64, 0x55, 0x4c, 0x44}, /* ż */
    {0x7e, 0x11, 0x11, 0x91, 0x7e}, /* Ą */
    {0x3c, 0x42, 0x46, 0x43, 0x24}, /* Ć */
    {0x7f, 0x49, 0x49, 0xc9, 0x41}, /* Ę */
    {0x7f, 0x50, 0x48, 0x44, 0x40}, /* Ł */
    {0x7e, 0x04, 0x0b, 0x10, 0x7e}, /* Ń */

    {0x3c, 0x42, 0x46, 0x43, 0x3c}, /* Ó */
    {0x4c, 0x52, 0x56, 0x53, 0x22}, /* Ś */
    {0x42, 0x66, 0x53, 0x4a, 0x46}, /* Ź */
    {0x42, 0x62, 0x53, 0x4a, 0x46}, /* Ż */

};
#endif
