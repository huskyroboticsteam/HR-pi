#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include "../functions.h"
//#include <signal.h>
#define pin 23

#ifdef BUILD_GETD_MAIN
int main(int argc, char *argv[]) {
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);
    uint32_t result=fpga_safetran(*vals);
    printf("%i\n", result);

    // print_bin(32,result);
}
#endif