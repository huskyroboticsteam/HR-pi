/*
    Author: Caleb Ceravolo
    
    Purpose: 
    To provide console command for controlling pwm pins on FPGA
    Units are microseconds (aka 1E-6 seconds)

    Execution:
    fpwm {channel} {uptime} [period]
*/

#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include <stdint.h>
#include "../functions.h"
//#include <signal.h>
#define pin 23

#ifdef IS_MAIN
int main(int argc, char *argv[]) {
    if (argc>3) {
        uint32_t result1=fpga_pwm_period(*vals, string_to_int(argv[3]));
        print_bin(32,result1);
    }
    uint32_t result2=fpga_pwm_uptime(*vals, string_to_int(argv[2]));
    print_bin(32,result2);
}
#endif