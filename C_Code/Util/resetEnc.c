#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
//#include <signal.h>

#ifdef IS_MAIN
int main(int argc, char *argv[]) {
    wiringPiSetupPinType(WPI_PIN_WPI);
    fpga_reset_encoder(string_to_int(argv[1]));
}
#endif