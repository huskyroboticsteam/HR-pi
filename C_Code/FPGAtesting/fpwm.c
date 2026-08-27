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
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);
    if (argc>3) {
        uint32_t result1=fpga_pwm_period(*vals, vals[2]);
        print_bin(32,result1);
    }
    uint32_t result2=fpga_pwm_uptime(*vals, vals[1]);
    print_bin(32,result2);
}
#endif