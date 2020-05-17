#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "ls020.h"
#include "font5x7.h"

uint8_t S65_WIDTH = 176; // x max
uint8_t S65_HEIGHT = 132; // y max

void spi_init(void) {
  // ports configuration
  LCD_RS_DDR |= (1 << LCD_RS);       // RS pin as output
  LCD_RESET_DDR |= (1 << LCD_RESET); // RESET pin as output
  LCD_CS_DDR |= (1 << LCD_CS);       // CS pin as output
  LCD_DATA_DDR |= (1 << LCD_DATA);   // DATA pin as output
  LCD_CLK_DDR |= (1 << LCD_CLK);     // CLK pin as output

  // SPI interface configuration
  SPCR = (1 << MSTR) | (1 << SPE); // master mode, SPI enable
  SPSR = (1 << SPI2X);             // double SPI frequency
}

void spi_send_byte(uint8_t byte) {
  SPDR = byte; // send data
  while (!(SPSR & (1 << SPIF)))
    ; // wait for end transfer
}

void spi_send_word(uint16_t word) {
  spi_send_byte(word >> 8); // send older byte
  spi_send_byte(word);      // send younger byte
}

void lcd_command_byte(uint8_t command_byte) {
  LCD_RS_PORT |= (1 << LCD_RS);  // send as command
  LCD_CS_PORT &= ~(1 << LCD_CS); // transfer enable
  spi_send_byte(command_byte);   // send byte
  LCD_CS_PORT |= (1 << LCD_CS);  // transfer disable
}

void lcd_command_word(uint16_t command_word) {
  lcd_command_byte(command_word >> 8); // send older byte
  lcd_command_byte(command_word);      // send younger byte
}

void lcd_data_byte(uint8_t data_byte) {
  LCD_RS_PORT &= ~(1 << LCD_RS); // send as data
  LCD_CS_PORT &= ~(1 << LCD_CS); // transfer enable
  spi_send_byte(data_byte);      // send data byte
  LCD_CS_PORT |= (1 << LCD_CS);  // transfer disable
}

void lcd_data_word(uint8_t data_word) {
  LCD_RS_PORT &= ~(1 << LCD_RS); // send as data
  LCD_CS_PORT &= ~(1 << LCD_CS); // transfer enable
  spi_send_word(data_word);      // send data word
  LCD_CS_PORT |= (1 << LCD_CS);  // transfer disable
}

void lcd_init(void) {
  spi_init();

  LCD_RESET_PORT &= ~(1 << LCD_RESET); // hardware reset
  _delay_ms(1);
  LCD_RESET_PORT |= (1 << LCD_RESET);
  _delay_ms(1);
  // software reset sequence
  lcd_command_byte(0xFD);
  _delay_us(10);
  lcd_command_byte(0xFD);
  _delay_us(10);
  lcd_command_byte(0xFD);
  _delay_us(10);
  lcd_command_byte(0xFD);
  _delay_ms(68);

  //--------------init tables----------------------
  const uint8_t init0[20] = {0xEF, 0x00, 0xEE, 0x04, 0x1B, 0x04, 0xFE,
                             0xFE, 0xFE, 0xFE, 0xEF, 0x90, 0x4A, 0x04,
                             0x7F, 0x3F, 0xEE, 0x04, 0x43, 0x06};

  const uint8_t init1[46] = {
      0xEF, 0x90, 0x09, 0x83, 0x08, 0x00, 0x0B, 0xAF, 0x0A, 0x00, 0x05, 0x00,
      0x06, 0x00, 0x07, 0x00, 0xEF, 0x00, 0xEE, 0x0C, 0xEF, 0x90, 0x00, 0x80,
      0xEF, 0xB0, 0x49, 0x02, 0xEF, 0x00, 0x7F, 0x01, 0xE1, 0x81, 0xE2, 0x02,
      0xE2, 0x76, 0xE1, 0x83, 0x80, 0x01, 0xEF, 0x90, 0x00, 0x00};

  for (uint8_t i = 0; i < 20; i++) {
    lcd_command_byte(init0[i]);
    _delay_us(10);
  }
  _delay_ms(10);

  for (uint8_t i = 0; i < 46; i++) {
    lcd_command_byte(init1[i]);
    _delay_us(10);
  }
  _delay_ms(10);
  lcd_command_word(0xE800 + (0 & 0x01) * 0x40);
}

void lcd_clear(uint16_t colour) { lcd_rectangle(0, 0, 175, 131, colour); }

void lcd_pixel(uint8_t x_pos, uint8_t y_pos, uint8_t colour) {
  lcd_set_pos(x_pos, y_pos);
  lcd_data_byte(colour);
}

void lcd_set_pos(uint8_t x_pos, uint8_t y_pos) {
  lcd_command_word(0x0700 + x_pos);
  lcd_command_word(0x0600 + y_pos);
}

void lcd_set_area(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  lcd_command_word(0x0500);
  lcd_command_word(0x0A00 + x1);
  lcd_command_word(0x0B00 + x2);
  lcd_command_word(0x0800 + y1);
  lcd_command_word(0x0900 + y2);
}

void lcd_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
                   uint8_t colour) {
  lcd_set_area(x1, y1, x2, y2);

  for (uint8_t x = x1; x <= x2; x++) {
    for (uint8_t y = y1; y <= y2; y++)
      lcd_data_byte(colour);
  }
}

void lcd_frame(uint8_t x_pos, uint8_t y_pos, uint8_t width, uint8_t height,
               char colour) {
  unsigned char j;
  // draw sides
  for (j = 0; j < height; j++) {
    lcd_pixel(x_pos, y_pos + j, colour);
    lcd_pixel(x_pos + width - 1, y_pos + j, colour);
  }
  // draw bases
  for (j = 0; j < width; j++) {
    lcd_pixel(x_pos + j, y_pos, colour);
    lcd_pixel(x_pos + j, y_pos + height - 1, colour);
  }
}

void lcd_char_5x7(uint8_t x_pos, uint8_t y_pos, char character, uint8_t colour,
                  uint8_t background) {
  lcd_rectangle(x_pos, y_pos, x_pos + 5, y_pos + 7, background);

  for (uint8_t i = 0; i < 5; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      uint8_t a = pgm_read_byte((&font5x7[(character - 32)][i]));
      if ((a << j) & 0x80)
        lcd_pixel(x_pos + i, y_pos + j, colour);
    }
  }
}

void lcd_char_10x14(uint8_t x_pos, uint8_t y_pos, char character,
                    uint8_t colour, uint8_t background) {
  lcd_rectangle(x_pos, y_pos, x_pos + 10, y_pos + 16, background);

  for (uint8_t i = 0; i < 5; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      uint8_t a = pgm_read_byte((&font5x7[(character - 32)][i]));
      if ((a << j) & 0x80) {
        lcd_pixel(x_pos + 2 * i, y_pos + 2 * j, colour);
        lcd_pixel(x_pos + 2 * i + 1, y_pos + 2 * j + 1, colour);
        lcd_pixel(x_pos + 2 * i, y_pos + 2 * j + 1, colour);
        lcd_pixel(x_pos + 2 * i + 1, y_pos + 2 * j, colour);
      }
    }
  }
}

void lcd_string(uint8_t x_pos, uint8_t y_pos, char *string, uint8_t colour,
                uint8_t background, uint8_t size) {
  uint8_t char_cnt = 0;
  while (string[char_cnt]) {
    if (size == SMALL_FONT) {
      lcd_char_5x7(x_pos, y_pos, string[char_cnt], colour, background);
      x_pos = x_pos + 6;
      char_cnt++;
    }
    if (size == BIG_FONT) {
      lcd_char_10x14(x_pos, y_pos, string[char_cnt], colour, background);
      x_pos = x_pos + 12;
      char_cnt++;
    }
  }
}

void lcd_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour) {
  // -----------SOURCE:
  // http://www.cs.unc.edu/~mcmillan/comp136/Lecture6/Lines.html
  // ----------------
  int16_t dx, dy, sx, sy, f;
  uint16_t i;

  dx = x2 - x1;
  dy = y2 - y1;

  if (dx < 0) {
    dx = -dx;
    sx = -1;
  } else {
    sx = 1;
  }

  if (dy < 0) {
    dy = -dy;
    sy = -1;
  } else {
    sy = 1;
  }

  dx <<= 1;
  dy <<= 1;

  lcd_pixel(x1, y1, colour);

  if (dx > dy) {
    i = dx;
    i >>= 1;
    f = dy - i;
    while (x1 != x2) {
      if (f >= 0) {
        y1 += sy;
        f -= dx;
      }
      x1 += sx;
      f += dy;
      lcd_pixel(x1, y1, colour);
    }
  } else {
    i = dy;
    i >>= 1;
    f = dx - i;
    while (y1 != y2) {
      if (f >= 0) {
        x1 += sx;
        f -= dy;
      }
      y1 += sy;
      f += dx;
      lcd_pixel(x1, y1, colour);
    }
  }
}
