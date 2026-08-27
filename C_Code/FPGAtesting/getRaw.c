#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include "../functions.h"
//#include <signal.h>

// #ifdef IS_MAIN
int main(int argc, char *argv[]) {
    uint32_t result=fpga_safetran(string_to_int(argv[1]));
    // printf("%i\n", result);

    print_bin(32,result);
}
// #endif