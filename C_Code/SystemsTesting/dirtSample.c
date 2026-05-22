#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
#include "Column/raise_lower_column.c"
#include "Column/rotateTo_column.c"

// FPGA PWM channel for the dirt-sample servo. Set by startup.sh (`fpwm 2 1500`).
#define DIRT_SAMPLE_CHANNEL 2

// Two encoded positions for dirt sample servo, in microseconds of PWM uptime (servo pulse width).
#define DIRT_SERVO_CLOSED 2500
#define DIRT_SERVO_OPEN 2300

// Column rotational positions for picking up dirt and depositing it.
// PLACEHOLDER VALUES, CHANGE!!
#define PICKUP_POSITION 0
#define DEPOSIT_POSITION 1

// Column vertical positions for picking up dirt and depositing it.
// PLACEHOLDER VALUES, CHANGE!!
#define COLUMN_LOWER 0
#define COLUMN_RAISE 1

void collect_and_deposit_dirt(){
    // Move column to pickup position
    // rotateTo(0)

    // Move column down
    // raiseLowerTo(idk)

    // Start augur
    // Needs work

    // ToF sensor to measure when augur is done
    // Long file, need to look into this

    // Stop augur
    // Needs work

    // Move column
    // raiseLowerTo(idk)

    // Move column to deposit position
    // rotateTo(idk)

    // Open sample collector, then wait for dirt to fall out
    fpga_pwm_uptime(DIRT_SAMPLE_CHANNEL, DIRT_SERVO_OPEN);
    sleep(3);

    // Close sample collector
    fpga_pwm_uptime(DIRT_SAMPLE_CHANNEL, DIRT_SERVO_CLOSED);
}

#ifdef BUILD_DSAMPLE_MAIN
int main(int argc, char *argv[]) {
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);

    // vals[0]            = position selector (0 -> POS_A, 1 -> POS_B)
    // vals[1] (optional) = period in microseconds (set on first call)

    // Normal method, UNCOMMENT LATER
    /*
    uint32_t uptime = (vals[0] == 0) ? DIRT_SERVO_CLOSED : DIRT_SERVO_OPEN;

    uint32_t result2 = fpga_pwm_uptime(DIRT_SAMPLE_CHANNEL, uptime);
    print_bin(32, result2);
    */


    // Testing, REMOVE LATER
    printf("Starting dirt sample collection and deposition...\n");
    collect_and_deposit_dirt();
    printf("Done.\n");
}
#endif
