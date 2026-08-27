/*
    Author: Caleb Ceravolo

    Purpose:
    To print out the current angle 0-360 of the centrifuge absolute encoder

    Execution:
    centriAbs
*/

#include "../functions.h"
#include "../pins.h"
#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <signal.h>
//  #define pin 23

#ifdef IS_MAIN
int main(int argc, char *argv[]) {
  uint32_t result = fpga_safetran(ENC_CENTRIFUGE_ABS);
  printf("%f\n", ((result) * 360) / 1018.0);
  // degrees = (result*360)/1018
  // (degrees*1018)/360
  // print_bin(32,result);
}
#endif