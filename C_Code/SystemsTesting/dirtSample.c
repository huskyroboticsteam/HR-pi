#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
#include "Column/raise_lower_column.c"
#include "Column/rotateTo_column.c"
#include "Augur/PWM_Map.c"
#include "tofTest.c"

// FPGA PWM channel for the dirt-sample servo. Set by startup.sh (`fpwm 2 1500`).
#define DIRT_SAMPLE_CHANNEL 2

// Two encoded positions for dirt sample servo, in microseconds of PWM uptime (servo pulse width).
#define DIRT_SERVO_CLOSED 2500
#define DIRT_SERVO_OPEN 2300

// Column rotational positions for picking up dirt and depositing it.
// PLACEHOLDER VALUES, CHANGE!!
#define PICKUP_ROTATE_POSITION 0
#define DEPOSIT_ROTATE_POSITION 1

// Column vertical positions for picking up dirt and depositing it.
// PLACEHOLDER VALUES, CHANGE!!
#define COLUMN_LOWER_POSITION 0
#define COLUMN_RAISE_POSITION 1

// Speeds to rotate augur at.
// PLACEHOLDER VALUES, CHANGE!!
#define AUGUR_ON 50
#define AUGUR_OFF 0

// Distance where ToF sensor considers augur done. 
// PLACEHOLDER VALUE, CHANGE!!
#define AUGUR_DONE_DISTANCE 100

void collect_and_deposit_dirt(){
    // Move column to pickup position
    rotateTo(PICKUP_ROTATE_POSITION, H1A_3, H1A_4);

    // Move column down
    raiseLowerTo(COLUMN_LOWER_POSITION, COLUMN_RL_PIN2, COLUMN_RL_PIN1);

    // Start augur
    spin_augur(AUGUR_ON);

    // ToF sensor to measure when augur is done
    int zero = zeroCalibration();
    int distance = zero - tofReadDistance();
    while (distance < AUGUR_DONE_DISTANCE) {
        usleep(50000 /* 50ms, 20/sec */);
        distance = zero - tofReadDistance();
    }

    // Stop augur
    spin_augur(AUGUR_OFF);

    // Move column
    raiseLowerTo(COLUMN_RAISE_POSITION, COLUMN_RL_PIN2, COLUMN_RL_PIN1);

    // Move column to deposit position
    rotateTo(DEPOSIT_ROTATE_POSITION, H1A_3, H1A_4);

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
