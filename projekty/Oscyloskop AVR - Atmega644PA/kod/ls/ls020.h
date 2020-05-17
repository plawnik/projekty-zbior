#ifndef LS_H_
#define LS_H_

// RS
#define LCD_RS_DDR DDRB
#define LCD_RS_PORT PORTB
#define LCD_RS PB0
// RESET
#define LCD_RESET_DDR DDRB
#define LCD_RESET_PORT PORTB
#define LCD_RESET PB1
// CS
#define LCD_CS_DDR DDRB
#define LCD_CS_PORT PORTB
#define LCD_CS PB4
// CLK
#define LCD_CLK_DDR DDRB
#define LCD_CLK_PORT PORTB
#define LCD_CLK PB7
// DATA/MOSI
#define LCD_DATA_DDR DDRB
#define LCD_DATA_PORT PORTB
#define LCD_DATA PB5
// LED
#define LCD_LED_DDR DDRB
#define LCD_LED_PORT PORTB
#define LCD_LED PB0

#define RED_COLOUR 0xE0
#define RED_DARK_COLOUR 0x80
#define ORANGE_COLOUR 0xEC
#define PURPLE_COLOUR 0xE2
#define PURPLE_DARK_COLOUR 0xA2
#define GREEN_COLOUER 0x1C
#define GREEN_DARK_COLOUR 0x0C
#define BLUE_COLOUR 0x03
#define BLUE_DARK_COLOUR 0x02
#define BLUE_LIGHT_COLOUR 0x97
#define TEAL_COLOUR 0x2F
#define YELLOW_COLOUR 0xFC
#define YELLOW_DARK_COLOUR 0xF4
#define BROWN_COLOUR 0x88
#define WHITE_COLOUR 0xFF
#define GRAY_COLOUR 0x92
#define BLACK_COLOUR 0x00
#define SMALL_FONT 1
#define BIG_FONT 2

void spi_init(void);
void spi_send_byte(uint8_t byte);
void spi_send_word(uint16_t word);

void lcd_command_byte(uint8_t command_byte);
void lcd_command_word(uint16_t command_word);

void lcd_data_byte(uint8_t data_byte);
void lcd_data_word(uint8_t data_word);

void lcd_init(void);
void lcd_clear(uint16_t colour);
void lcd_pixel(uint8_t x_pos, uint8_t y_pos, uint8_t colour);
void lcd_set_pos(uint8_t x_pos, uint8_t y_pos);
void lcd_set_area(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2);
void lcd_rectangle(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2,
                   uint8_t colour);
void lcd_frame(uint8_t x_pos, uint8_t y_pos, uint8_t width, uint8_t height,
               char colour);
void lcd_char_5x7(uint8_t x_pos, uint8_t y_pos, char character, uint8_t colour,
                  uint8_t background);
void lcd_char_10x14(uint8_t x_pos, uint8_t y_pos, char character,
                    uint8_t colour, uint8_t background);
void lcd_string(uint8_t x_pos, uint8_t y_pos, char *string, uint8_t colour,
                uint8_t background, uint8_t size);

void lcd_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint16_t colour);

#endif /* LS_H_ */
