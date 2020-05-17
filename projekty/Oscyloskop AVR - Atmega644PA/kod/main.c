#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "ls/ls020.h"
#include "oscilloscope/oscilloscope.h"

void draw_intro(void);

volatile uint16_t probe_counter;
volatile uint8_t draw_flag;
volatile uint8_t range_flag;
volatile uint8_t key_flag;
volatile uint8_t key_status = 0;
volatile uint8_t adc_offset = 63;
volatile uint8_t probe_offset = 0;
volatile uint8_t scale = 2;
uint8_t adc_bufor[1024];
uint8_t lcd_bufor[160];

volatile uint8_t max_adc;
volatile uint8_t min_adc;

// | TIMEBASE | RANGE | TRIGGER_lvl | APROXIMATION | OTHER  |
uint8_t settings[5] = {0, 0, 2, 4, 5};

int main() {
  // init peripherials
  keys_init();
  lcd_init();
  draw_intro();
  _delay_ms(505);
  lcd_clear(BLACK_COLOUR);
  adc_init();
  timer1_init();
  timer0_init();
  sei();
  _delay_ms(100);
  oscilloscope(); // call main oscilloscope program
}

void draw_intro(void) {
  lcd_clear(BLACK_COLOUR);
  lcd_string(5, 114, "PRACA", WHITE_COLOUR, BLACK_COLOUR, BIG_FONT);
  lcd_string(68, 99, "DYPLOMOWA", WHITE_COLOUR, BLACK_COLOUR, BIG_FONT);
  lcd_string(36, 90, "Projekt cyfrowego", WHITE_COLOUR, BLACK_COLOUR,
             SMALL_FONT);
  lcd_string(15, 82, "oscyloskopu z graficznym", WHITE_COLOUR, BLACK_COLOUR,
             SMALL_FONT);
  lcd_string(12, 74, "wyswietlaczem LCD opartym", WHITE_COLOUR, BLACK_COLOUR,
             SMALL_FONT);
  lcd_string(33, 66, "o architekture AVR", WHITE_COLOUR, BLACK_COLOUR,
             SMALL_FONT);
  lcd_string(40, 10, "Wykonal Pawel Lawnik", WHITE_COLOUR, BLACK_COLOUR,
             SMALL_FONT);
  _delay_ms(4000);
}
