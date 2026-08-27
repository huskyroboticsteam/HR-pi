#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include "../functions.h"
#include <math.h>  
//#include <signal.h>
#define pin 23
int main(int argc, char *argv[]) {

    uint32_t result=fpga_raw(0b10101010101010101010101010101010);

    print_bin(32,result);
}
