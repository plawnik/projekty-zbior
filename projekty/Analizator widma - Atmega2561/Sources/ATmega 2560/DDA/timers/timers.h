/*
 * timers.h
 *
 * Created: 2018-04-18 13:01:00
 *  Author: Pawel
 */

#ifndef TIMERS_H_
#define TIMERS_H_

// configuration for 24MHz clock source

void inline timer1_init(void) {
  TCCR1A = (1 << WGM11);  // CTC mode
  OCR1A = 233;            // ~51,2kHz[19,53 us]
  TIMSK1 = (1 << OCIE1A); // comp A interrupt enable
}

void inline timer1_start(void) {
  TCNT1 = 0;             // restart value
  TCCR1B |= (1 << CS10); // start timer
}
void inline timer1_stop(void) {
  TCCR1B &= ~(1 << CS10); // stop timer
}

void inline timer3_init(void) {
  TCCR3A = (1 << WGM31);  // CTC mode
  OCR3A = 60000;          // 0.2kHz[5ms]
  TIMSK3 = (1 << OCIE3A); // comp A interrupt enable
}

void inline timer3_start(void) {
  TCNT3 = 0;             // restart value
  TCCR3B |= (1 << CS30); // start timer
}

void inline timer3_stop(void) {
  TCCR3B &= ~(1 << CS30); // stop timer
}

#endif /* TIMERS_H_ */