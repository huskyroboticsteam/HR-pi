#include "../../functions.h"
#include "../../pins.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

/**
 * Finds the angular velocity of the centrifuge in encoder ticks per second.
 *
 * @param sample_time_us The time in microseconds to wait between taking encoder readings.
 * @return The angular velocity in ticks per second.
 */
double findVelocity(int sample_time_us) {
    struct timeval start, end;
    
    // Read the initial encoder position and time
    uint32_t start_ticks = fpga_safetran(ENC_CENTRIFUGE_INC);
    gettimeofday(&start, NULL);
    
    // Wait for the specified sample time
    usleep(sample_time_us);
    
    // Read the final encoder position and time
    uint32_t end_ticks = fpga_safetran(ENC_CENTRIFUGE_INC);
    gettimeofday(&end, NULL);
    
    // Calculate elapsed time in microseconds
    long elapsed_usec = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
    
    // Calculate the difference in ticks.
    // Casting to int32_t naturally handles uint32_t overflow/underflow.
    int32_t tick_diff = (int32_t)(end_ticks - start_ticks);
    
    // Calculate and return velocity in ticks per second
    double velocity = ((double)tick_diff / elapsed_usec) * 1000000.0;
    
    return velocity;
}

