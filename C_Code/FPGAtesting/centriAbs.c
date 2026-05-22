#include "../functions.h"
#include "../pins.h"
#include <stdint.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <signal.h>
//  #define pin 23

#ifdef BUILD_CENTRIABS_MAIN
int main(int argc, char *argv[]) {
  // int vals[argc-1];
  // intparse(argc-1, argv+1, vals);
  uint32_t result = fpga_safetran(ENC_CENTRIFUGE_ABS);
  printf("%f\n", ((result) * 360) / 1018.0);
  // degrees = (result*360)/1018
  // (degrees*1018)/360
  // print_bin(32,result);
}
#endif