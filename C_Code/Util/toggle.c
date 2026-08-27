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
    for (int i = 0; i < argc - 1; i++) {
        toggle_or_drive_high(string_to_int(argv[i+1]));
    }
    return 0;
}

#ifdef IS_MAIN
int main(int argc, char *argv[]) {
    return run_toggle(argc, argv);
}
#endif