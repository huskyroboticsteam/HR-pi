#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "waterLevelADS1015.h"
#include "waterLevel.h"

// Reads from ADC_Max.bin to find the range of adc counts
// Interpolates adc readings to then find the current height of liquid in the tank
// FOR MONITORING PURPOSES ONLY

static void run_live_monitor(int fd, int channel, int num_samples, int interval_ms, float cal_max_adc) {
    float prev_avg  = -1.0f;
    int stable_count = 0;

    printf("\n\n\n\n");

    while (1) {
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

        if (valid == 0) {
            printf("\033[4A\033[2KError: Wiring failure\n\n\n\n");
            continue;
        }

        float avg_raw    = (float)sum / valid;
        float voltage    = (avg_raw / ADC_MAX_COUNT) * VOLTAGE_REF;
        float signed_delta = (prev_avg < 0.0f) ? 0.0f : (avg_raw - prev_avg);
        float abs_delta  = (signed_delta < 0) ? -signed_delta : signed_delta;
        prev_avg = avg_raw;

        if (abs_delta > STABLE_DELTA_THRESHOLD) stable_count = 0;
        else if (stable_count < STABLE_CONFIRM_CYCLES) stable_count++;

        float pct = avg_raw / cal_max_adc;
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;

        float depth_mm = pct_to_mm(pct);

        printf("\033[4A");
        printf("\033[2KRaw ADC:  %6.1f  (delta %+.1f)\n", avg_raw, signed_delta);
        printf("\033[2KVoltage:  %.4f V\n", voltage);
        if (stable_count < STABLE_CONFIRM_CYCLES) {
            printf("\033[2KLevel:    stabilizing... %6.1f %%\n", pct * 100.0f);
            printf("\033[2KDepth:    stabilizing... %6.1f mm\n", depth_mm);
        } else {
            printf("\033[2KLevel:   %6.1f %%\n", pct * 100.0f);
            printf("\033[2KDepth:   %6.1f mm\n", depth_mm);
        }
        fflush(stdout);
        usleep(interval_ms * 1000);
    }
}

#ifdef BUILD_WATERLEVELREAD_MAIN
int main(int argc, char *argv[]) {
    int channel     = DEFAULT_CHANNEL;
    int num_samples = DEFAULT_NUM_SAMPLES;
    int interval_ms = DEFAULT_INTERVAL_MS;

    if (argc > 1) num_samples = atoi(argv[1]);
    if (argc > 2) interval_ms = atoi(argv[2]);

    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if (fd == -1) {
        printf("Failed to open I2C for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return -1;
    }

    FILE *f = fopen("ADC_Max.bin", "rb");
    if (!f) {
        printf("ADC_Max.bin not found — run calibrate first\n");
        return -1;
    }
    float cal_max_adc;
    fread(&cal_max_adc, sizeof(float), 1, f);
    fclose(f);

    printf("ADS1015 water level sensor -- AIN%d, %d-sample average, %d ms interval\n\n",
           channel, num_samples, interval_ms);

    run_live_monitor(fd, channel, num_samples, interval_ms, cal_max_adc);
    return 0;
}
#endif