// pump KCL 45 ml (mixing while), spectromter(--- 20 ml? amount), NiH (25 ml), heat it up (90 C, 15 min), color sensor, dump
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "../../functions.h"
#include "../burnerController.c"
#include "../pumpToHeight.c"
#include "../colorRead.c"

// Unified Ctrl+C handler: trips the stop flag in every module that polls one,
// so SIGINT during a pump phase actually breaks the pump loop (not just the burner).
static void mc_intHandler(int _) {
    (void)_;
    burner_sigint = 1;
    pump_sigint = 1;
}

// Called between phases: if a Ctrl+C arrived during the previous step, shut off
// the mixer (the only shared GPIO this function owns) and tell the caller to bail.
// Pumps and burnerControl each turn their own outputs off on exit.
static int mc_should_abort(void) {
    if (pump_sigint || burner_sigint) {
        digitalWrite(MIXER_PIN, 0);
        printf("\nAborted by SIGINT.\n");
        return 1;
    }
    return 0;
}

// pump times
#define KCL_PUMP_TIME 1
#define NIH_PUMP_TIME 2
#define SPECTROMETER_PUMP_TIME 
#define BURNER_HEAT_TIME 900 //seconds
#define DUMP_TIME 

//burner
#define BURNER_TEMP 90
#define BURNER_RANGE 0.5

void mixingChamber() {
    signal(SIGINT, mc_intHandler);
    //pump KCL
    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if(fd == -1) {
        printf("Failed ot open I2c for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return;
    }
    wiringPiSetupPinType(WPI_PIN_WPI);

    // Mixer runs through all three pump phases. burnerControl re-asserts and
    // then turns it off at the end of heating, so we don't touch it again
    // before color read / dump.
    pinMode(MIXER_PIN, OUTPUT);
    digitalWrite(MIXER_PIN, 1);

    pumpToTime(fd, KCL_PIN, KCL_PUMP_TIME);
    if (mc_should_abort()) return;

    digitalWrite(MIXER_PIN, 0);

    //spectromter
    pumpToTime(fd, SPEC_PIN, SPECTROMETER_PUMP_TIME);
    if (mc_should_abort()) return;

    //pump NiH
    pumpToTime(fd, NINHYDRIN_PIN, NIH_PUMP_TIME);
    if (mc_should_abort()) return;

    //turn on burner for --- time
    burnerControl(BURNER_TEMP, BURNER_RANGE, BURNER_HEAT_TIME);
    if (mc_should_abort()) return;

    //color sensor
    runColorRead();
    if (mc_should_abort()) return;

    //dump
    pumpToTime(fd, DISPOSAL_PIN, DUMP_TIME);
}

int main(void) {
    mixingChamber();
    return 0;
}