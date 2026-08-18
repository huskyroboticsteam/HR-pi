#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../functions.h"

// ADS1015 I2C address: ADDR pin to GND
#define ADS1015_ADDR       0x48

// Register pointers
#define REG_CONVERSION     0x00
#define REG_CONFIG         0x01

// Config register fields
#define OS_START           (1 << 15)    // start single-shot conversion
#define MUX_AIN0_GND       (0x04 << 12)
#define MUX_AIN1_GND       (0x05 << 12)
#define MUX_AIN2_GND       (0x06 << 12)
#define MUX_AIN3_GND       (0x07 << 12)
#define PGA_4_096V         (0x03 << 9)  // ±4.096V FSR, safe for 3.3V and 5V supplies
#define MODE_SINGLE        (1 << 8)     // single-shot, not continuous
#define DR_1600SPS         (0x01 << 5)  // Consider dropping to increase precision
#define COMP_DISABLE       0x03         // disable comparator output

// ADC full-scale for the 12-bit result (signed, positive half = 2047)
#define ADC_MAX_COUNT      2047
#define VOLTAGE_REF        1.024f       // must match PGA_4_096V

// ── Calibration lookup table ──────────────────────────────────────────────────
// Each entry is {semi-stable ADC average, water depth in mm}.
// Readings were taken after the initial insertion spike settled but before
// electrolysis drift began. Add more rows or extend the range as needed.
static const struct { float pct; float mm; } CAL[] = {
    { 0.00f, 0.0f  },
    { 0.48f, 5.0f  },
    { 0.70f, 10.0f },
    { 0.80f, 15.0f },
    { 0.89f, 20.0f },
    { 0.94f, 25.0f },
    { 0.97f, 30.0f },
    { 1.00f, 35.0f },
};

#define CAL_POINTS (int)(sizeof(CAL) / sizeof(CAL[0]))
#define CAL_MAX_MM  35.0f

// ADC counts/cycle below which the reading is considered settled.
// Raise this if it declares "stable" too early; lower it if it takes too long.
#define STABLE_DELTA_THRESHOLD  7.0f
// How many consecutive stable cycles before the reading is trusted.
#define STABLE_CONFIRM_CYCLES   3
// ─────────────────────────────────────────────────────────────────────────────

#define DEFAULT_CHANNEL         0
#define DEFAULT_NUM_SAMPLES     50
#define DEFAULT_INTERVAL_MS     100

// ADS1015 sends 16-bit values MSB first; wiringPiI2CReadReg16 returns them
// in host (little-endian) order, so bytes must be swapped.
static uint16_t bswap16(uint16_t v) {
    return (uint16_t)((v >> 8) | (v << 8));
}

// Perform one single-shot conversion on the given channel (0-3).
// Returns the signed 12-bit ADC result, or -32768 on error.
static int16_t ads1015_read(int fd, int channel) {
    uint16_t config = OS_START | MUX_AIN0_GND | PGA_4_096V | MODE_SINGLE |
                      DR_1600SPS | COMP_DISABLE;

    // Write config register (byte-swap to big-endian before sending)
    wiringPiI2CWriteReg16(fd, REG_CONFIG, bswap16(config));

    // Wait for conversion: 1/1600 SPS ~= 0.625 ms; 2 ms gives comfortable margin
    usleep(2000);

    // Poll OS bit (bit 15) until it reads 1 (conversion complete)
    uint16_t status;
    int timeout = 20;
    do {
        status = bswap16((uint16_t)wiringPiI2CReadReg16(fd, REG_CONFIG));
        usleep(500);
    } while (!(status & (1 << 15)) && --timeout);

    if (timeout == 0) return -32768;

    // Read 16-bit conversion register, result is left-aligned, shift right 4 (ADC is 12-bit)
    int16_t raw = (int16_t)bswap16((uint16_t)wiringPiI2CReadReg16(fd, REG_CONVERSION));
    raw >>= 4;
    return raw;
}

// Convert a raw ADC average to mm depth using piecewise linear interpolation.
static float pct_to_mm(float pct) {
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

// Monitors in pct fill based on max ADC count
void run_live_monitor(int fd, int channel, int num_samples, int interval_ms, float cal_max_adc) {
    float prev_avg = -1.0f;
    int stable_count = 0;

    // Clear initial space for the 4-line UI
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

        float avg_raw = (float)sum / valid;
        float voltage = (avg_raw / ADC_MAX_COUNT) * VOLTAGE_REF;
        float signed_delta = (prev_avg < 0.0f) ? 0.0f : (avg_raw - prev_avg);
        float abs_delta = (signed_delta < 0) ? -signed_delta : signed_delta;
        prev_avg = avg_raw;

        if (abs_delta > STABLE_DELTA_THRESHOLD) stable_count = 0;
        else if (stable_count < STABLE_CONFIRM_CYCLES) stable_count++;

        float pct = avg_raw / cal_max_adc;
        if (pct < 0) pct = 0.0f; // Clamp values
        if (pct > 1.0f) pct = 1.0f;
        
        // Use your piecewise function to get mm
        float depth_mm = pct_to_mm(pct); 

        // UI Update
        printf("\033[4A");
        printf("\033[2KRaw ADC:  %6.1f  (delta %+.1f)\n", avg_raw, signed_delta);
        printf("\033[2KVoltage:  %.4f V\n", voltage);
        if (stable_count < STABLE_CONFIRM_CYCLES) {
            printf("\033[2KLevel:    stabilizing... %6.1f %%\n", pct * 100.0f);
            printf("\033[2KDepth:    stabilizing... %6.1f %%\n", depth_mm);
        } else {
            printf("\033[2KLevel:   %6.1f %%\n", pct * 100.0f);
            printf("\033[2KDepth:   %6.1f mm\n", depth_mm);
        }
        fflush(stdout);

        usleep(interval_ms * 1000);
    }
}

int main(int argc, char *argv[]) {
    int channel       = DEFAULT_CHANNEL;
    int num_samples   = DEFAULT_NUM_SAMPLES;
    int interval_ms   = DEFAULT_INTERVAL_MS;

    if (argc > 1) num_samples = atoi(argv[1]);
    if (argc > 2) interval_ms = atoi(argv[2]);

    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if (fd == -1) {
        printf("Failed to open I2C for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return -1;
    }

    printf("ADS1015 water level sensor -- AIN%d, %d-sample average, %d ms interval\n\n",
           channel, num_samples, interval_ms);
    printf("\n\n\n\n");  // reserve lines for in-place update

    FILE *f = fopen("ADC_Max.bin", "rb");
    float cal_max_adc;
    fread(&val, sizeof(float), 1, f);
    fclose(f);
    run_live_monitor(fd, channel, num_samples, interval_ms, cal_max_adc);

    return 0;
}