#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include "../../functions.h"
#include "../Column/raise_lower_column.c"
#include "../Column/rotateTo_column.c"
#include "../Augur/PWM_Map.c"
#include "../tof.c" // No idea why this requries the c file, but the h file doesn't work for some reason.
#include "../tofTest.c"
#include <sys/param.h>

// FPGA PWM channel for the dirt-sample servo. Set by startup.sh (`fpwm 2 1500`).
#define DIRT_SAMPLE_CHANNEL 2

// Two encoded positions for dirt sample servo, in microseconds of PWM uptime (servo pulse width).
#define DIRT_SERVO_CLOSED 2350
#define DIRT_SERVO_OPEN 1900

// Column rotational positions for picking up dirt and depositing it.
#define PICKUP_ROTATE_POSITION 0
#define DEPOSIT_ROTATE_POSITION -90

// Column vertical positions for picking up dirt and depositing it.
#define COLUMN_LOWER_POSITION -39000
#define COLUMN_RAISE_POSITION 100

// PWM uptimespeeds to rotate augur at.
#define AUGUR_ON 1300
#define AUGUR_OFF 1500
#define AUGUR_REVERSE 1700

// Distance where ToF sensor considers augur done. 
// PLACEHOLDER VALUE, CHANGE!!
#define AUGUR_DONE_DISTANCE 25

void collect_and_deposit_dirt(){
    // Close sample collector
    fpga_pwm_uptime(DIRT_SAMPLE_CHANNEL, DIRT_SERVO_CLOSED);
    printf("Closed dirt sample channel\n");
    // Move column to top
    // raiseLowerTo(1000000, COLUMN_RL_PIN2, COLUMN_RL_PIN1);
    // printf("Moved column to top\n");
    // sleep(1);

    // Rotated column to pickup position
    rotateTo(PICKUP_ROTATE_POSITION, SPIN_COLUMN_PIN1, SPIN_COLUMN_PIN2);
    printf("Rotated column to pickup position\n");
    sleep(1);

    // Move column down
    raiseLowerTo(COLUMN_LOWER_POSITION, COLUMN_RL_PIN2, COLUMN_RL_PIN1);
    printf("Moved column to pickup\n");
    sleep(1);
    if (sigint){
        return;
    }
    // Start augur
    spin_augur(AUGUR_OFF, AUGUR_ON);

    puts("Augur on");
    // ToF sensor to measure when augur is done
    if (init()==-1){
        fprintf(stderr, "ERROR: TOF failed to connect");
    }
    sleep(1);
    const int zero = -92;
    sleep(1);
    int distance = zero - tofReadDistance();
    int32_t ground = read_position_ticks(); // NOTE: Each tick is ~0.012mm.
    // while (distance < AUGUR_DONE_DISTANCE) {
    int32_t ticks = ground; // Get augur position
    if(ticks < (COLUMN_LOWER_POSITION + 500)){ //Make sure augur is not too low
        fprintf(stderr, "ERROR: Column encoder reading %d ticks is below expected range. "
                "Check column position and encoder.\n", ticks);
    }
    else{
        printf("Starting decrement\n");
        // int32_t drill_dist = MIN(8333, abs(ground-COLUMN_LOWER_POSITION));
        while(abs(ground - ticks) < 8333){ // If augur has moved 10cm (8333 ticks) down, stop. Otherwise, dig down.
            int32_t target_distance = (ticks - 500);
            int32_t distance_remaining = ticks_remaining(ticks, target_distance);
            if (sigint){
                break;
            }
            digitalWrite(COLUMN_RL_PIN1, 1);
            printf("Drilling to %d\n", target_distance);
            while(distance_remaining > 10){
                if (sigint){
                    break;
                }
                digitalWrite(COLUMN_RL_PIN1, 1);
                usleep(20000);
                printf("Drilling dist rem %d\n", distance_remaining);
                ticks = read_position_ticks();
                distance_remaining = ticks_remaining(ticks, target_distance);
            }
            printf("Drilling dist remaining total %d\n", 8333-abs(ground - ticks));
            if (sigint){
                break;
            }
            digitalWrite(COLUMN_RL_PIN1, 0);
            if (ticks<=COLUMN_LOWER_POSITION){
                break;
            }
            sleep(2);
            distance = zero - tofReadDistance();
            if (distance >= AUGUR_DONE_DISTANCE){
                printf("TOF measured %d. We outta here\n", distance);
                break;
            }
            // }
            // digitalWrite(COLUMN_RL_PIN1, 0);
            // usleep(200000 /* 2ms, 5/sec */);
            // distance = zero - tofReadDistance();
        }
    }
    if (sigint){
        spin_augur(AUGUR_ON, AUGUR_OFF);
        return;
    }
    for (int i=0; i<20; i++){
        distance = zero - tofReadDistance();
        if (distance >= AUGUR_DONE_DISTANCE){
            printf("TOF measured %d. We outta here\n", distance);
            break;
        }
        sleep(1);
    }
    while (distance < AUGUR_DONE_DISTANCE){
        distance = zero - tofReadDistance();
        if (distance >= AUGUR_DONE_DISTANCE){
            printf("TOF measured %d. We outta here\n", distance);
        }
    }
    sleep(20);
    // Stop augur
    puts("Augur off");
    spin_augur(AUGUR_ON, AUGUR_OFF);
    sleep(1);
    if (sigint){
        return;
    }
    // Move column
    raiseLowerTo(COLUMN_RAISE_POSITION, COLUMN_RL_PIN2, COLUMN_RL_PIN1);
    sleep(1);

    // spin_augur()
    // Move column to deposit position
    rotateTo(DEPOSIT_ROTATE_POSITION, SPIN_COLUMN_PIN1, SPIN_COLUMN_PIN2);
    sleep(1);

    // Open sample collector, then wait for dirt to fall out
    fpga_pwm_uptime(DIRT_SAMPLE_CHANNEL, DIRT_SERVO_OPEN);
    sleep(3);

    // Close sample collector
    fpga_pwm_uptime(DIRT_SAMPLE_CHANNEL, DIRT_SERVO_CLOSED);
    sleep(1);

    rotateTo(PICKUP_ROTATE_POSITION, SPIN_COLUMN_PIN1, SPIN_COLUMN_PIN2);
    puts("Done");

}

#ifdef BUILD_DSAMPLE_MAIN
int main(int argc, char *argv[]) {
    wiringPiSetupPinType(WPI_PIN_WPI);
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);
    signal(SIGINT, intHandler);

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
