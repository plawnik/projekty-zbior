#ifndef OSCILLOSCOPE_H_
#define OSCILLOSCOPE_H_

#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)

#define KEY_1 (1 << PC0) // timebase key
#define KEY_2 (1 << PC1) // range key
#define KEY_3 (1 << PC2) // trigger level

#define KEY_PORT C

static inline void adc_start_conversion(void) { ADCSRA |= (1 << ADSC); }
static inline void timer1_disable(void) {
  TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));
}
static inline void timer1_enable(void) { TCCR1B |= (1 << CS10); }

void adc_init(void);
void timer1_init(void);
void timer0_init(void);
void keys_init(void);

void fill_probe_buffer(uint16_t timebase);
void lcd_draw_scope(uint8_t *data, uint8_t colour, uint8_t background);
void lcd_draw_grid(void);
void lcd_draw_settings(void);
void oscilloscope(void);
void trigger(void);
void set_2_volts_per_div(void);
void set_8_volts_per_div(void);

#endif /* OSCILLOSCOPE_H_ */
