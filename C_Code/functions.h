#ifndef FPGA_UTILS_H
#define FPGA_UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// Function declarations
// void enc_dec(int16_t *amount, float *degrees);
extern int string_to_int(char * arg);
extern float string_to_float(char * arg);
extern void print_bin(int len, int in);
extern void print_arr(void * input, int len);
// extern void intparse(int len, char** strings, int * output);
// extern void floatparse(int len, char** strings, float * output);
void to_char_array(uint32_t base, unsigned char * output);
extern uint32_t fpga_command(uint32_t command);
extern uint32_t fpga_pwm_uptime(uint8_t motor, uint32_t pwm_uptime);
extern uint32_t fpga_pwm_period(uint8_t motor, uint32_t pwm_period);
extern uint32_t fpga_raw(uint32_t outgoing);
extern uint32_t fpga_datatran(uint8_t data_addr);
extern uint32_t fpga_safetran(uint8_t data_addr);
void fpga_fasttran(uint8_t data_addr, uint32_t* result);
extern uint32_t to_uint_value(unsigned char * input);
extern int length_of(char * point);
uint32_t fpga_reset_encoder(uint8_t encoder);

#endif // FPGA_UTILS_H
