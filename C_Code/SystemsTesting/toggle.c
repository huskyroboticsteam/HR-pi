#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
//#include <signal.h>

static void toggle_or_drive_high(int pin) {
    int mode = getAlt(pin);
    if (mode == 1) {
        digitalWrite(pin, !digitalRead(pin));
    } else {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 1);
    }
}

static int run_toggle(int argc, char *argv[]) {
    wiringPiSetupPinType(WPI_PIN_WPI);
    int vals[argc - 1];
    intparse(argc - 1, argv + 1, vals);
    for (int i = 0; i < argc - 1; i++) {
        toggle_or_drive_high(vals[i]);
    }
    return 0;
}

#ifdef BUILD_TOGGLE_MAIN
int main(int argc, char *argv[]) {
    return run_toggle(argc, argv);
}
#endif