/*
 * DDA.c
 *
 * Created: 2018-04-16 18:06:43
 * Author : Pawel
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "rs232/rs232.h"
#include "timers/timers.h"
#include "custom_math.h"
#include "lcd/ls020.h"

#define KEYS_PORT PORTA
#define KEYS_DDR DDRA
#define KEYS_PIN PINA
#define KEY_1 (1 << PA1)
#define KEY_2 (1 << PA2)
#define KEY_3 (1 << PA3)
#define KEY_4 (1 << PA4)

#define max(a, b) ((a) < (b)) ? (b) : (a)

int16_t samples[1024]; // ADC samples buffer
// main program control structure, something like state machine
struct {
  volatile uint8_t meas_flag;
  volatile uint8_t draw_flag;
  volatile uint8_t rs232_flag;
  volatile uint8_t window_type;
  volatile uint8_t demo_mode;
  volatile uint8_t debug_mode;
  volatile uint8_t redraw_flag;
} flags = {0, 0, 0, 0, 1, 1, 1};

void disable_jtag() {
  asm volatile("cli"
               "\n\t" // disable interrupts
               "push r16"
               "\n\t" // save r18 val
               "in r16,0x34"
               "\n\t" // load MCUCSR reg
               "sbr r16, 0x80"
               "\n\r" // set JTD disable bit
               "out 0x34,r16"
               "\n\r" // write val to MCUCSR
               "out 0x34,r16"
               "\n\r" // second time
               "pop r16"
               "\n\t" // restore r18 val
               "sei"
               "\n\t" // enable interrupts
               );
}

void adc_init(void) {
  DDRF &= ~(1 << PF0);                   // PF0 as input (ADC0)
  PORTF &= ~(1 << PF0);                  // disable pullup
  ADMUX = (1 << REFS0);                  // AVCC as VREF (5V)
  ADCSRA = (1 << ADEN) |                 // enable ADC
           (1 << ADPS0) | (1 << ADPS2) | // clock prescaler 32
           (1 << ADATE) |                // free run mode
           (1 << ADSC);                  // start conversion
  DIDR0 = (1 << ADC0D);                  // disable digital IO on ADC0
}

void keys_init(void) {
  KEYS_DDR &= ~(KEY_1 | KEY_2 | KEY_3 | KEY_4);
  KEYS_PORT = (KEY_1 | KEY_2 | KEY_3 | KEY_4);
  DDRA |= (1 << PA0);
  PORTA &= ~(1 << PA0);
}

void fill_sample_buffer(void) {
  flags.meas_flag = 1; // set meas flag
  timer1_start();      // start filling
  while (flags.meas_flag == 1)
    ; // wait for the end
}

void draw_splash_screen(void) {
  lcd_string((176 - 15 * 6) / 2, 110, "PRACA DYPLOMOWA", WHITE_COLOUR,
             BLACK_COLOUR, SMALL_FONT);
  lcd_string(1, 100, "Realizacja algorytmow analizy", WHITE_COLOUR,
             BLACK_COLOUR, SMALL_FONT);
  lcd_string((176 - 25 * 6) / 2, 90, "widma czestotliwosciowego", WHITE_COLOUR,
             BLACK_COLOUR, SMALL_FONT);
  lcd_string((176 - 19 * 6) / 2, 80, "sygnalow okresowych", WHITE_COLOUR,
             BLACK_COLOUR, SMALL_FONT);
  lcd_string((176 - 21 * 6) / 2, 60, "KATOLICKI UNIWERSYTET", WHITE_COLOUR,
             BLACK_COLOUR, SMALL_FONT);
  lcd_string((176 - 20 * 6) / 2, 50, "LUBELSKI LUBLIN 2018", WHITE_COLOUR,
             BLACK_COLOUR, SMALL_FONT);
  lcd_string(1, 10, "WYKONAL:", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
  lcd_string(1, 2, "mgr inz. Pawel Lawnik", WHITE_COLOUR, BLACK_COLOUR,
             SMALL_FONT);
}

void draw_bars(void) {
  lcd_set_area(0, 157, 0, 131);
  for (uint8_t i = 1; i < 128; i++) {
    lcd_set_pos(i + 48, 4);
    int16_t tmp = max(samples[2 * i - 1], samples[2 * i]);
    tmp >>= 2; // 512 max amplitude value

    for (int j = 0; j < 128; j++) {
      if (tmp > j)
        lcd_data_byte(WHITE_COLOUR);
      else
        lcd_data_byte(BLACK_COLOUR);
    }
    // while(tmp>0){
    //	lcd_data_byte(WHITE_COLOUR);
    // tmp--;
    //}
  }
}

void draw_axis(void) {
  lcd_set_area(0, 157, 0, 131);
  lcd_line(48, 0, 48, 131, GREEN_COLOUR);
  lcd_line(44, 3, 175, 3, GREEN_COLOUR);
  lcd_line(44, 28, 48, 28, GREEN_COLOUR);
  lcd_line(44, 54, 48, 54, GREEN_COLOUR);
  lcd_line(44, 80, 48, 80, GREEN_COLOUR);
  lcd_line(44, 106, 48, 106, GREEN_COLOUR);
  lcd_line(44, 131, 48, 131, GREEN_COLOUR);
  lcd_line(58, 0, 58, 3, GREEN_COLOUR);
  lcd_line(68, 0, 68, 3, GREEN_COLOUR);
  lcd_line(78, 0, 78, 3, GREEN_COLOUR);
  lcd_line(88, 0, 88, 3, GREEN_COLOUR);
  lcd_line(98, 0, 98, 3, GREEN_COLOUR);
  lcd_line(108, 0, 108, 3, GREEN_COLOUR);
  lcd_line(118, 0, 118, 3, GREEN_COLOUR);
  lcd_line(128, 0, 128, 3, GREEN_COLOUR);
  lcd_line(138, 0, 138, 3, GREEN_COLOUR);
  lcd_line(148, 0, 148, 3, GREEN_COLOUR);
  lcd_line(158, 0, 158, 3, GREEN_COLOUR);
  lcd_line(168, 0, 168, 3, GREEN_COLOUR);
}

void draw_status(void) {
  flags.redraw_flag = 0;
  lcd_rectangle(0, 0, 43, 131, BLACK_COLOUR);
  // rs232 satus
  lcd_set_area(0, 0, 157, 131);
  lcd_string(1, 122, "RS:", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  if (flags.rs232_flag)
    lcd_string(19, 122, "ON  ", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  else
    lcd_string(19, 122, "OFF ", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  // window status
  lcd_string(1, 112, "W:", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  if (flags.window_type == HAMMING)
    lcd_string(11, 112, "HAMM", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  else if (flags.window_type == HANNING)
    lcd_string(11, 112, "HANN", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  else if (flags.window_type == SQUARE)
    lcd_string(11, 112, "SQRE", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  else if (flags.window_type == TRIANGLE)
    lcd_string(11, 112, "TRNG", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
  else if (flags.window_type == FLAT_TOP)
    lcd_string(11, 112, "FLAT", YELLOW_COLOUR, BLACK_COLOUR, SMALL_FONT);
}

void demo_mode(void) {
  lcd_rectangle(0, 0, 43, 131, BLACK_COLOUR);
  lcd_string(1, 16, "DEMO", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
  lcd_string(1, 8, "MODE", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
  flags.redraw_flag = 1;
  while (flags.demo_mode == 1) {
    lcd_string(1, 0, "SINE", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
    for (int i = 0; i < 16; i++) {
      if (!flags.demo_mode)
        return;
      fill_test_sinus(i);
      fix_fftr(samples, 10);
      fix_fft_mag(samples, 1024);
      draw_bars();
      send_mag_via_uart();
    }
    for (int i = 16; i > 0; i--) {
      if (!flags.demo_mode)
        return;
      fill_test_sinus(i);
      fix_fftr(samples, 10);
      fix_fft_mag(samples, 1024);
      draw_bars();
      send_mag_via_uart();
    }
    lcd_string(1, 0, "SAW ", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
    for (int i = 0; i < 16; i++) {
      if (!flags.demo_mode)
        return;
      fill_test_saw();
      fix_fftr(samples, 10);
      fix_fft_mag(samples, 1024);
      draw_bars();
      send_mag_via_uart();
    }
    lcd_string(1, 0, "SQRE", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
    for (int i = 0; i < 16; i++) {
      if (!flags.demo_mode)
        return;
      fill_test_square();
      fix_fftr(samples, 10);
      fix_fft_mag(samples, 1024);
      draw_bars();
      send_mag_via_uart();
    }
    lcd_string(1, 0, "RAND", WHITE_COLOUR, BLACK_COLOUR, SMALL_FONT);
    for (int i = 0; i < 32; i++) {
      if (!flags.demo_mode)
        return;
      fill_random_signal();
      fix_fftr(samples, 10);
      fix_fft_mag(samples, 1024);
      draw_bars();
      send_mag_via_uart();
    }
  }
}

int main(void) {
  // init peripherials
  disable_jtag();
  adc_init();
  keys_init();
  timer1_init();
  timer3_init();
  timer3_start();
  rs232_init();
  rs232_clear_console();
  // init lcd
  spi_init();
  lcd_init();
  lcd_clear(BLACK_COLOUR);
  draw_splash_screen();
  _delay_ms(5000);
  lcd_clear(BLACK_COLOUR);
  draw_axis();
  draw_status();

  // main loop
  while (1) {
    if (flags.demo_mode)
      demo_mode();
    if (flags.redraw_flag)
      draw_status();
    fill_sample_buffer();
    fix_fftr(samples, 10);
    fix_fft_mag(samples, 1024);
    draw_bars();
    if (flags.rs232_flag) {
      send_mag_via_uart();
    }
  }
}

//***************TIMER 1 IS WORKING AS TRIGGER*******************//
//********Interrupt must be execute in <468 clock cycles*********//
//**********************(24MHz*19,5us)***************************//
ISR(TIMER1_COMPA_vect) {
  static uint16_t cnt = 0; // local variable
  int16_t tmp = ADCW;
  if (cnt > 0) { // discard first measure
    samples[cnt - 1] = window(tmp, flags.window_type, (cnt - 1));
  }
  cnt++;                 // inc cnt
  if (cnt == 1025) {     // end of buffer
    timer1_stop();       // stop filling
    cnt = 0;             // reset probe cnt
    flags.meas_flag = 0; // clear meas flag
  }
}

ISR(TIMER3_COMPA_vect) {
  // check keys status
  if ((~KEYS_PIN) & (KEY_1 | KEY_2 | KEY_3 | KEY_4)) {
    cli();  // disable interrupts
    _delay_ms(20);
    if (~KEYS_PIN & (KEY_1 | KEY_2 | KEY_3 | KEY_4)) {
      if ((~KEYS_PIN) & (KEY_1)) {
        flags.demo_mode ^= 1;
      }
      if ((~KEYS_PIN) & (KEY_2)) {
        flags.window_type++;
        if (flags.window_type > FLAT_TOP)
          flags.window_type = SQUARE;
      }
      if ((~KEYS_PIN) & (KEY_3)) {
        flags.rs232_flag ^= 1;
      }
    }
    flags.redraw_flag = 1;
  }
  while ((~KEYS_PIN) & (KEY_1 | KEY_2 | KEY_3 | KEY_4))
    ;

  sei();    // enable interrupts
}