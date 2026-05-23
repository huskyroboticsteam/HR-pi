#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include "waterLevelADS1015.h"

// Handles interaction with the ADC Converter

static uint16_t bswap16(uint16_t v) {
    return (uint16_t)((v >> 8) | (v << 8));
}

int16_t ads1015_read(int fd, int channel) {
    uint16_t config = OS_START | MUX_AIN0_GND | PGA_4_096V | MODE_SINGLE |
                      DR_1600SPS | COMP_DISABLE;

    wiringPiI2CWriteReg16(fd, REG_CONFIG, bswap16(config));
    usleep(2000);

    uint16_t status;
    int timeout = 20;
    do {
        status = bswap16((uint16_t)wiringPiI2CReadReg16(fd, REG_CONFIG));
        usleep(500);
    } while (!(status & (1 << 15)) && --timeout);

    if (timeout == 0) return -32768;

    int16_t raw = (int16_t)bswap16((uint16_t)wiringPiI2CReadReg16(fd, REG_CONVERSION));
    raw >>= 4;
    return raw;
}

int ads1015_detect_change(int fd, int channel, int num_samples,
                          int interval_ms, float threshold, int confirm_cycles) {
    float prev_avg = -1.0f;

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
            usleep(interval_ms * 1000);
            continue;
        }

        float avg = (float)sum / valid;

        if (prev_avg < 0.0f) {
            prev_avg = avg;
            usleep(interval_ms * 1000);
            continue;
        }

        float delta = avg - prev_avg;
        if (delta < 0.0f) delta = -delta;

        if (delta > threshold) {
            return 1;  // change detected
        }

        prev_avg = avg;
        usleep(interval_ms * 1000);
    }
}