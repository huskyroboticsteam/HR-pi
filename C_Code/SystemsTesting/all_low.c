// #include <stdio.h>
#include <wiringPi.h>
#include <stdint.h>
#include "../pins.h"

static int not_in(int num, int *list, int len) {
    for (int i = 0; i < len; i++) {
        if (num == *(list + i)) {
            return 0;
        }
    }
    return 1;
}

/* General code file I use to generate all set all commands */
static void drive_allowed_pins_low(void) {
    int not_list[7] = {SDA, SCL, MOSI, MISO, SCLK, CE, ONEWIRE};
    for (int i = 0; i < 32; i++) {
        if (not_in(i, not_list, 7)) {
            pinMode(i, OUTPUT);
            digitalWrite(i, 0);
        }
    }
}

#ifdef BUILD_ALL_LOW_MAIN
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    wiringPiSetupPinType(WPI_PIN_WPI);
    drive_allowed_pins_low();
    return 0;
}
#endif