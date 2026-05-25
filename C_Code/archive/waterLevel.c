#include <stdint.h>
#include <unistd.h>
#include "waterLevel.h"

// Useful functions that interpret ADC counts into measureable units (mm)

static const struct { float pct; float mm; } CAL[] = {
    { 0.00f,  0.0f },
    { 0.48f,  5.0f },
    { 0.70f, 10.0f },
    { 0.80f, 15.0f },
    { 0.89f, 20.0f },
    { 0.94f, 25.0f },
    { 0.97f, 30.0f },
    { 1.00f, 35.0f },
};
#define CAL_POINTS (int)(sizeof(CAL) / sizeof(CAL[0]))

float pct_to_mm(float pct) {
    if (pct <= CAL[0].pct) return CAL[0].mm;
    if (pct >= CAL[CAL_POINTS - 1].pct) return CAL[CAL_POINTS - 1].mm;
    for (int i = 1; i < CAL_POINTS; i++) {
        if (pct <= CAL[i].pct) {
            float t = (pct - CAL[i-1].pct) / (CAL[i].pct - CAL[i-1].pct);
            return CAL[i-1].mm + t * (CAL[i].mm - CAL[i-1].mm);
        }
    }
    return CAL[CAL_POINTS - 1].mm;
}

static float sample_avg_adc(int fd, int channel, int num_samples) {
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
    if (valid == 0) return -1.0f;
    return (float)sum / valid;
}

static float adc_to_mm(float avg_raw, float cal_max_adc) {
    float pct = avg_raw / cal_max_adc;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 1.0f) pct = 1.0f;
    return pct_to_mm(pct);
}

float waterLevel_read_mm(int fd, int channel, int num_samples, float cal_max_adc) {
    float avg_raw = sample_avg_adc(fd, channel, num_samples);
    if (avg_raw < 0.0f) return -1.0f;
    return adc_to_mm(avg_raw, cal_max_adc);
}

float waterLevel_read_stable_mm(int fd, int channel, int num_samples,
                                int interval_ms, float cal_max_adc) {
    float prev_avg = -1.0f;
    float avg_raw  = -1.0f;
    int stable_count = 0;

    while (stable_count < STABLE_CONFIRM_CYCLES) {
        avg_raw = sample_avg_adc(fd, channel, num_samples);
        if (avg_raw < 0.0f) {
            usleep(interval_ms * 1000);
            continue;
        }

        float abs_delta = (prev_avg < 0.0f) ? STABLE_DELTA_THRESHOLD + 1.0f
                                            : avg_raw - prev_avg;
        if (abs_delta < 0) abs_delta = -abs_delta;
        prev_avg = avg_raw;

        if (abs_delta > STABLE_DELTA_THRESHOLD) stable_count = 0;
        else stable_count++;

        usleep(interval_ms * 1000);
    }

    return adc_to_mm(avg_raw, cal_max_adc);
}
