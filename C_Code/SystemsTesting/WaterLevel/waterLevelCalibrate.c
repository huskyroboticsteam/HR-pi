#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "waterLevelADS1015.h"

// Calibrates the adc count of the empty tank,
// Then the tank must be filled to fully submerge the sensor
// After a stability period, the maximum amount of adc counts
// is recorded to ADC_Max.bin

#define MAX_STABLE_THRESHOLD    2.0f
#define MIN_ADC_MAX_COUNTS      1000
#define MAX_STABLE_CONFIRM_CYCLES 10

#ifdef BUILD_WATERLEVELCALIBRATE_MAIN
int main() {
    int channel     = DEFAULT_CHANNEL;
    int num_samples = DEFAULT_NUM_SAMPLES;

    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if (fd == -1) {
        printf("Failed to open I2C for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return -1;
    }

    float prev_avg  = -1.0f;
    float current_avg = 0.0f;
    int stable_count = 0;

    printf("Starting Auto-Calibration... Filling container.\n");

    while (stable_count < MAX_STABLE_CONFIRM_CYCLES) {
        long sum = 0;
        int valid = 0;

        for (int i = 0; i < num_samples; i++) {
            int16_t raw = ads1015_read(fd, channel);
            if (raw != -32768) {
                sum += raw;
                valid++;
            }
            usleep(1000);
        }

        if (valid > 0) {
            current_avg = (float)sum / valid;

            if (prev_avg >= 0.0f) {
                float delta = current_avg - prev_avg;
                float abs_delta = (delta < 0) ? -delta : delta;

                if (abs_delta <= MAX_STABLE_THRESHOLD && current_avg > MIN_ADC_MAX_COUNTS) {
                    stable_count++;
                    printf("\033[2KStability detected: %d/%d\r", stable_count, MAX_STABLE_CONFIRM_CYCLES);
                } else {
                    stable_count = 0;
                    printf("\033[2KFilling... Current ADC: %.1f\r", current_avg);
                }
            }
            prev_avg = current_avg;
        }
        fflush(stdout);
        usleep(50000);
    }

    printf("\nCalibrated Max ADC Count: %.1f\n", current_avg);

    FILE *f = fopen("ADC_Max.bin", "wb");
    if (!f) {
        printf("Failed to open ADC_Max.bin for writing\n");
        return -1;
    }
    fwrite(&current_avg, sizeof(float), 1, f);
    fclose(f);

    return 0;
}
#endif