#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "oscilloscope.h"
#include "../ls/ls020.h"

extern uint8_t adc_bufor[1024];
extern uint8_t lcd_bufor[160];
extern volatile uint8_t key_status;
extern volatile uint16_t probe_counter;
extern volatile uint8_t adc_offset;
extern volatile uint8_t probe_offset;
extern volatile uint8_t scale;
extern volatile uint8_t range_flag;
extern volatile uint8_t key_flag;
extern volatile uint8_t max_adc;
extern volatile uint8_t min_adc;
extern volatile uint8_t draw_flag;
extern volatile uint8_t trigger_level;
extern uint8_t settings[5];
//  50ms/div	20ms   10ms	5ms	  2ms	1ms	 500us 200us itd
const uint16_t ocr_values[] = {59999, 23999, 11999, 5999, 2399, 1199,
                               599,   239,   119,   59,   23,   11};

const char timebase_texts[12][6] = {" 50ms", " 20ms", " 10ms", "  5ms",
                                    "  1ms", "500us", "200us", "100us",
                                    " 50us", " 20us", " 10us"};

void timer1_init(void) {
  TIMSK1 = (1 << OCIE1A); // interrupt output compare
  TCCR1B = (1 << WGM12);  // CTC mode
}

void timer0_init(void) {
  TCCR0B = (1 << CS02) | (1 << CS00);
  TIMSK0 = (1 << TOIE0);
}

void keys_init(void) {
  DDR(KEY_PORT) &= ~(KEY_1 | KEY_2 | KEY_3); // as outputs
  PORT(KEY_PORT) |= (KEY_1 | KEY_2 | KEY_3); // pull ups
}

void adc_init(void) {
  DDRA &= ~(1 << PA0);
  ADMUX |= (1 << REFS1) | (1 << REFS0) |
           (1 << ADLAR); // internal V_REF=2,56V , left adjust result
  ADCSRA |=
      (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0) | (1 << ADATE) |
      (1 << ADSC); // enable, auto trigger, max prescaler, start conversion
}

void set_2_volts_per_div(void) {
  DDRC &= ~((1 << PC6) | (1 << PC7));
  PORTC &= ~((1 << PC6) | (1 << PC7));
}
void set_8_volts_per_div(void) {
  DDRC |= (1 << PC6 | (1 << PC7));
  PORTC |= (1 << PC7);
}

void fill_probe_buffer(uint16_t timebase) {
  timer1_disable();
  OCR1A = ocr_values[timebase];
  probe_counter = 0;
  timer1_enable();
}

void oscilloscope(void) {
  while (1) {
    fill_probe_buffer(settings[0]);
    while (draw_flag == 0)
      ;
    trigger();
    lcd_draw_settings();
    lcd_draw_grid();
    lcd_draw_scope(adc_bufor, WHITE_COLOUR, BLACK_COLOUR);
    if (key_flag == 1) {
      // timebase
      if (key_status & KEY_1)
        settings[0] = settings[0] + 1;
      if (settings[0] == 11)
        settings[0] = 0;

      // range
      if ((key_status & KEY_2)) {
        if (settings[1] == 0) {
          settings[1] = 1;
          set_8_volts_per_div();
        } else if (settings[1] == 1) {
          settings[1] = 0;
          set_2_volts_per_div();
        }
      }

      // trigger lvl
      if ((key_status & KEY_3)) {
        settings[2]++;
        if (settings[2] > 4)
          settings[2] = 0;
      }

      // exit from flag
      key_status = 0;

      key_flag = 0;
    }

    draw_flag = 0;
  }
}
//(const char*)menu_tab[i]
void lcd_draw_settings(void) {
  // timebase
  lcd_string(0, 121, (char *)timebase_texts[settings[0]], BLACK_COLOUR,
             WHITE_COLOUR, SMALL_FONT);

  // trigger
  lcd_string(30, 121, " ?-", BLACK_COLOUR, WHITE_COLOUR, SMALL_FONT);
  char text_buffet[2];
  itoa(settings[2], text_buffet, 10);
  lcd_string(48, 121, text_buffet, BLACK_COLOUR, WHITE_COLOUR, SMALL_FONT);
  if (settings[1] == 0)
    lcd_string(55, 121, "2v/div", BLACK_COLOUR, WHITE_COLOUR, SMALL_FONT);
  if (settings[1] == 1)
    lcd_string(55, 121, "8v/div", BLACK_COLOUR, WHITE_COLOUR, SMALL_FONT);
}

void trigger(void) {
  max_adc = adc_bufor[0];
  min_adc = adc_bufor[0];
  uint8_t trigger = 0;

  for (int cnt = 0; cnt < 1024; cnt++) {
    if (adc_bufor[cnt] > max_adc)
      max_adc = adc_bufor[cnt];
    if (adc_bufor[cnt] < min_adc)
      min_adc = adc_bufor[cnt];
  }
  if (max_adc != min_adc)
    trigger = (((max_adc - min_adc) / 2) +
               min_adc); // Find the middle of the wave to be used as trigger.
  else
    trigger = max_adc;

  for (int cnt = 1; cnt < 1024; cnt++) {
    if ((adc_bufor[cnt] > trigger + settings[2]) &&
        (adc_bufor[cnt - 1] < trigger - settings[2])) {
      probe_offset = cnt;
      break;
    }
  }
}

void lcd_draw_scope(uint8_t *data, uint8_t colour, uint8_t background) {
  int8_t a = 0;

  for (uint8_t index = 0; index < 160; index++) {
    lcd_pixel(index, lcd_bufor[index], BLACK_COLOUR);
  }

  for (uint8_t index = 0; index < 160; index++) {
    a = adc_bufor[probe_offset + index] / 2;
    lcd_pixel(index, a, WHITE_COLOUR);
    lcd_bufor[index] = a;
  }
}

ISR(TIMER1_COMPA_vect) {
  // PORTC |= (1<<PC0);
  if (probe_counter == 1024) {
    timer1_disable(); /*PORTC&=~(1<<PC0);*/
    draw_flag = 1;
  }
  adc_bufor[probe_counter] = ADCH;
  probe_counter++;
}

ISR(TIMER0_OVF_vect) {
  if ((~PIN(KEY_PORT)) & (KEY_1 | KEY_2 | KEY_3)) {
    timer1_disable();
    draw_flag = 1;
    key_flag = 1;
    _delay_ms(25);
    if ((~PIN(KEY_PORT)) & (KEY_1 | KEY_2 | KEY_3)) {
      key_status = (~PINC);
      while ((~PIN(KEY_PORT)) & (KEY_1 | KEY_2 | KEY_3))
        ;
    }
  }
}

void lcd_draw_grid(void) {
  lcd_line(0, 0, 159, 0, WHITE_COLOUR);
  lcd_line(0, 119, 159, 119, WHITE_COLOUR);
  lcd_line(0, 0, 0, 119, WHITE_COLOUR);
  lcd_line(159, 0, 159, 119, WHITE_COLOUR);
  lcd_line(19, 0, 19, 119, WHITE_COLOUR);
  lcd_line(39, 0, 39, 119, WHITE_COLOUR);
  lcd_line(59, 0, 59, 119, WHITE_COLOUR);
  lcd_line(79, 0, 79, 119, WHITE_COLOUR);
  lcd_line(99, 0, 99, 119, WHITE_COLOUR);
  lcd_line(119, 0, 119, 119, WHITE_COLOUR);
  lcd_line(139, 0, 139, 119, WHITE_COLOUR);
  lcd_line(0, 19, 159, 19, WHITE_COLOUR);
  lcd_line(0, 39, 159, 39, WHITE_COLOUR);
  lcd_line(0, 59, 159, 59, WHITE_COLOUR);
  lcd_line(0, 79, 159, 79, WHITE_COLOUR);
  lcd_line(0, 99, 159, 99, WHITE_COLOUR);
}
 
