/*
 * rs232.c
 *
 * Created: 2018-04-17 13:09:27
 *  Author: Pawel
 */

#include <avr/io.h>
#include <stdlib.h>
#include "rs232.h"

extern int16_t samples[1024];

void inline static rs232_set_baud(void) {
#include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
#if USE_2X
  UCSR1A |= (1 << U2X1);
#else
  UCSR1A &= ~(1 << U2X1);
#endif
}

void rs232_init(void) {
  rs232_set_baud();                       // set baud rate
  UCSR1C = (1 << UCSZ10) | (1 << UCSZ11); // 8N1 mode
  UCSR1B = (1 << TXEN1) | (1 << RXEN1);   // TX,RX enable
  DDRD |= (1 << PD3);                     // TX as output
}

void rs232_send_byte(uint8_t byte) {
  UDR1 = byte;
  while (!(UCSR1A & (1 << UDRE1)))
    ;
}

void rs232_send_string(const char *s) {
  while (*s)
    rs232_send_byte(*s++);
}

void rs232_clear_console(void) {
  // clear console ad set cursor to home pos
  // working only in VT100 mode
  char tab[] = {0x1B, '[', '2', 'J', 0x1B, '[', 'H', 0};
  rs232_send_string(&tab[0]);
}

void rs232_send_int(int i) {
  char text_buf[7];
  itoa(i, text_buf, 10);
  rs232_send_string(text_buf);
}

void rs232_new_line(void) { rs232_send_string("\n\r"); }

void send_samples_via_uart() {
  for (int i = 0; i < 1024; i++) {
    char text_buf[6];
    itoa(samples[i], text_buf, 10);
    rs232_send_string(text_buf);
    rs232_send_byte(' ');
  }
}

void send_fft_result_via_uart() {
  for (int i = 0; i < 1024; i++) {
    char text_buf[6];
    itoa(samples[i], text_buf, 10);
    rs232_send_int(i);
    rs232_send_string("-");
    rs232_send_string(text_buf);
    rs232_send_byte(' ');
  }
}

void send_mag_via_uart() {
  rs232_send_string("AT+START_SEND\n\r");
  for (int i = 0; i <= 256; i++) {
    char text_buf[6];
    itoa(samples[i], text_buf, 10);
    rs232_send_string(text_buf);
    rs232_send_byte(' ');
  }
  rs232_send_string("AT+STOP_SEND\n\r");
}
