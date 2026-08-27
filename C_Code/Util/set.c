#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
//#include <signal.h>
int main(int argc, char *argv[]) {
    wiringPiSetupPinType(WPI_PIN_WPI);

    // int mode = getAlt(string_to_int(argv[1]));

    // if (mode==1){
    //     // printf("%i",digitalRead(string_to_int(argv[1])));
    //     digitalWrite(string_to_int(argv[1]), string_to_int(argv[2]));
    //     return 0;
    // } else {
    //     pinMode(string_to_int(argv[1]), OUTPUT);
    //     digitalWrite(string_to_int(argv[1]), string_to_int(argv[2]));
    //     return 0;
    // }
}