#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
//#include <signal.h>
#define LEFTEN 4//4
#define RIGHTEN 5//5

static void run_column_test(int argc, char *argv[]) {
    wiringPiSetupPinType(WPI_PIN_WPI);
    pinMode(LEFTEN, OUTPUT);
    pinMode(RIGHTEN, OUTPUT);
    if (string_to_int(argv[1]) == 1) {
        digitalWrite(RIGHTEN, 0);
        digitalWrite(LEFTEN, 1);
    } else {
        digitalWrite(LEFTEN, 0);
        digitalWrite(RIGHTEN, 1);
    }
    if (argc > 1) {
    }
    sleep(1);
    pinMode(LEFTEN, PM_OFF);
    pinMode(RIGHTEN, PM_OFF);
}

int main(int argc, char *argv[]) {
    run_column_test(argc, argv);
    return 0;
}
