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


#ifdef IS_MAIN
int main(int argc, char *argv[]) {
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);
    uint32_t result=fpga_safetran(*vals);
    printf("%i\n", result);

    // print_bin(32,result);
}
#endif