/*
    Author: Caleb Ceravolo
    
    Purpose: 
    To get data from an FPGA register as an integer
    Execution:
    getData {channel}
*/
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include "../functions.h"
//#include <signal.h>
int main(int argc, char *argv[]) {
    uint32_t result=fpga_safetran(string_to_int(argv[1]));
    printf("%i\n", result);
}