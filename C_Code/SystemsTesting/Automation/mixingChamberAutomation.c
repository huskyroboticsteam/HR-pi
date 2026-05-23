// pump KCL 45 ml (mixing while), spectromter(--- 20 ml? amount), NiH (25 ml), heat it up (90 C, 15 min), color sensor, dump
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <unistd.h>
#include "../../functions.h"
#include "../burnerController.c"
#include "../colorReadTest.c"
#include "../pumpToHeight.c"
#include "../colorRead.c"

// activation/deactivation pins
#define BURNER_PIN 
#define KCL_PIN
#define NIH_PIN
#define CENTRIFUGE_PUMP_PIN
#define COLOR_SENSOR_PIN
#define DUMP_PIN

// device adresses
#define ADS1015_ADDR 0x48

// pump times
#define KCL_PUMP_TIME
#define NIH_PUMP_TIME
#define SPECTROMETER_PUMP_TIME
#define BURNER_HEAT_TIME
#define DUMP_TIME

//burner
#define BURNER_TEMP
#define BURNER_RANGE

void mixingChamber() {
    //pump KCL
    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if(fd == -1) {
        printf("Failed ot open I2c for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return -1;
    }
    wiringPiSetup();
    wiringPiSetupPinType(WPI_PIN_WPI);
    pumpToTime(fd, KCL_PIN, KCL_PUMP_TIME);

    //spectromter
    pumpToTime(fd, SPECTROMETER_PIN, SPECTROMETER_PUMP_TIME);

    //pump NiH
    pumpToTime(fd, NIH_PIN, NIH_PUMP_TIME);

    //turn on burner for --- time
    burnerControl(BURNER_TEMP, BURNER_RANGE, BURNER_HEAT_TIME);

    //color sensor
    runColorRead();
    
    //dump
    pumpToTime(fd, DUMP_PIN, DUMP_TIME);
    
    return void;
}