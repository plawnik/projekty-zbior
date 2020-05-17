/*
 * custom_math.h
 *
 * Created: 26.04.2018 19:45:39
 *  Author: Pawel
 */ 


#ifndef CUSTOM_MATH_H_
#define CUSTOM_MATH_H_


#define SQUARE 0
#define HANNING 1
#define HAMMING 2
#define TRIANGLE 3
#define FLAT_TOP 4



int16_t sinus(int k);
int16_t cosinus(int k);
void fix_fft(int16_t fr[], int16_t fi[], int16_t m);
void fix_fftr(int16_t f[], int m);
void fix_fft_mag(int16_t f[], int16_t k);
int16_t window(int16_t val,int8_t w, int16_t idx);
uint16_t square_root(uint32_t n);

void fill_test_sinus(int h);
void fill_test_saw(void);
void fill_test_square(void);
void fill_random_signal(void);





#endif /* CUSTOM_MATH_H_ */