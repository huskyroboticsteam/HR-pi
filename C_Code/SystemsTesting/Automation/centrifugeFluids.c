// #include "../Centrifuge/rotateServoTo.c"
#include "../Pump/raiseLower_Pump.c"
#include "../Centrifuge/centrifugeSpin.c"
#include "../../functions.h"
#include "../../pins.h"
#include "../pumpToHeight.c"


// encoder positions we can call
#define POS_0 882
#define POS_1 121
#define POS_2 358
#define POS_3 625

#define SPIN_DURATION XX // How long we spin the centrifuge for (placeholder)
#define PUMP_HEIGHT_MM XX // placeholder
#define CAL_MAX_ADC XX // placeholder
// static void intHandler(int dummy) {
//     sigint = 1;
//     digitalWrite(SPIN_CHANNEL, 0);
//   }
// global volatile int sigint = 0;
void fluidsProcessing() {
    // rotate centrifuge to a set position
    rotateTo(POS_0); //PLACEHOLDER

    //Lower pump into centrifuge
    pump(0);

    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    pump_delta_mm(fd, FLUIDSPUMP_IN_PIN, PUMP_HEIGHT_MM, CAL_MAX_ADC);
    pump(1);

    spinFor(SPIN_DURATION);
    pump(0);
    pump_delta_mm(fd, FLUIDSPUMP_OUT_PIN, PUMP_HEIGHT_MM, CAL_MAX_ADC);
    pump(1)    
   
}

#ifdef BUILD_CENTRIFUGEFLUIDS_MAIN
int main(int argc, char* argv[]) {
    // signal(SIGINT, intHandler);
    // wiringPiSetupPinType(WPI_PIN_WPI);
    // int vals[argc - 1];
    // intparse(argc - 1, argv + 1, vals);
    fluidsProcessing();
}
#endif

