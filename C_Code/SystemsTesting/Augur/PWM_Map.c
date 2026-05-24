#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include <stdint.h>
#include "../../functions.h"
//#include <signal.h>
#include "../../pins.h"
#include <unistd.h>

// Map a value from one range to another.
int32_t map_range(int32_t value,
                  int32_t in_low,  int32_t in_high,
                  int32_t out_low, int32_t out_high)
{
    if (value < in_low)  value = in_low;
    if (value > in_high) value = in_high;

    return out_low +
           ((out_high - out_low) * (value - in_low)) / (in_high - in_low);
}

// Spins the augur motor at the given PWM uptime. Returns the result of the FPGA 
// command, which may be useful for debugging.
int32_t spin_augur(int32_t curr_uptime, int32_t target_uptime) {
    // int32_t highuptime;
    // int32_t lowuptime;
    for(int i = 0; i < 100; i+=10){
        fpga_pwm_uptime(AUGUR_CHANNEL, curr_uptime+((target_uptime-curr_uptime)/100.0f)*i);
        usleep(200000 /* 200ms */);
    }
    return fpga_pwm_uptime(AUGUR_CHANNEL, target_uptime);
}

#ifdef BUILD_PWMMAP_MAIN
int main(int argc, char *argv[]) {
    int vals[argc-1];
    intparse(argc-1, argv+1, vals);

    // vals[0] = motor channel
    // vals[1] = speed input (-100 to 100)
    // vals[2] (optional) = period in microseconds (set on first call)

    // Currently using placeholder numbers to test things - may need to change this line
    uint32_t uptime = (uint32_t)map_range(vals[1], -100, 100, 500, 1500);

    if (argc > 3) {
        uint32_t result1 = fpga_pwm_period(vals[0], vals[2]);
        print_bin(32, result1);
    }
    uint32_t result2 = fpga_pwm_uptime(vals[0], uptime);
    print_bin(32, result2);
}
#endif