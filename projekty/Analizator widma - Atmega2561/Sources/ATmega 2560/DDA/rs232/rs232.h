/*
 * rs232.h
 *
 * Created: 2018-04-17 13:09:12
 *  Author: Pawel
 */ 


#ifndef RS232_H_
#define RS232_H_

#define BAUD 9600


void rs232_init(void);
void rs232_send_byte(uint8_t byte);
void rs232_send_string(const char* s);
void rs232_clear_console(void);
void rs232_send_int(int i);
void rs232_new_line(void);
void send_samples_via_uart();
void send_fft_result_via_uart();
void send_mag_via_uart();






#endif /* RS232_H_ */