#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
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
#define DR_1600SPS         (0x04 << 5)  // Consider dropping to increase precision
#define COMP_DISABLE       0x03         // disable comparator output

// ADC full-scale for the 12-bit result (signed, positive half = 2047)
#define ADC_MAX_COUNT      2047
#define VOLTAGE_REF        1.024f       // must match PGA_4_096V

// ── Temperature compensation ──────────────────────────────────────────────────
// slope = (ADC_at_T2 - ADC_at_T1) / (T2 - T1)  [ADC counts per °C]
// Set TEMP_REF_C to the temperature at which the CAL[] table was collected.
#define TEMP_REF_C    20.5f   // reference temperature used during depth calibration
#define TEMP_SLOPE     -2.3166f   // ADC counts per °C  <-- SET THIS FROM YOUR DATA
// ─────────────────────────────────────────────────────────────────────────────

// ── Calibration lookup table ──────────────────────────────────────────────────
// Each entry is {semi-stable ADC average, water depth in mm}.
// Readings were taken after the initial insertion spike settled but before
// electrolysis drift began. Add more rows or extend the range as needed.
static const struct { float adc; float mm; } CAL[] = {
    { 141.4f,  0.0f },
    { 697.0f, 5.0f },
    { 1008.4f, 10.0f },
    { 1161.2f, 15.0f },
    { 1281.2f, 20.0f },
    { 1353.6f,  25.0f },
    { 1404.2f,  30.0f },
    { 1444.0f,  35.0f },
};
#define CAL_POINTS (int)(sizeof(CAL) / sizeof(CAL[0]))
#define CAL_MAX_MM  35.0f

// ADC counts/cycle below which the reading is considered settled.
// Raise this if it declares "stable" too early; lower it if it takes too long.
#define STABLE_DELTA_THRESHOLD  5.0f
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

static float read_ds18b20_temp(void) {
    static char cached_device_file[256] = {0};
    static int path_found = 0;

    if (!path_found) {
        DIR *dir = opendir("/sys/bus/w1/devices/");
        struct dirent *direntp;
        if (dir == NULL) return -999.0f;
        while ((direntp = readdir(dir)) != NULL) {
            if (strncmp(direntp->d_name, "28-", 3) == 0) {
                snprintf(cached_device_file, sizeof(cached_device_file),
                         "/sys/bus/w1/devices/%s/temperature", direntp->d_name);
                path_found = 1;
                break;
            }
        }
        closedir(dir);
        if (!path_found) return -999.0f;
    }

    FILE *f = fopen(cached_device_file, "r");
    if (f == NULL) { path_found = 0; return -999.0f; }
    char buf[16];
    float temp_c = -999.0f;
    if (fgets(buf, sizeof(buf), f) != NULL)
        temp_c = atof(buf) / 1000.0f;
    fclose(f);
    return temp_c;
}

// Perform one single-shot conversion on the given channel (0-3).
// Returns the signed 12-bit ADC result, or -32768 on error.
static int16_t ads1015_read(int fd, int channel) {
    static const uint16_t mux_table[] = {
        MUX_AIN0_GND, MUX_AIN1_GND, MUX_AIN2_GND, MUX_AIN3_GND
    };
    uint16_t mux = (channel >= 0 && channel <= 3) ? mux_table[channel] : MUX_AIN0_GND;
    uint16_t config = OS_START | mux | PGA_4_096V | MODE_SINGLE |
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
static float adc_to_mm(float adc) {
    if (adc <= CAL[0].adc) return CAL[0].mm;
    if (adc >= CAL[CAL_POINTS - 1].adc) return CAL[CAL_POINTS - 1].mm;
    for (int i = 1; i < CAL_POINTS; i++) {
        if (adc <= CAL[i].adc) {
            float t = (adc - CAL[i-1].adc) / (CAL[i].adc - CAL[i-1].adc);
            return CAL[i-1].mm + t * (CAL[i].mm - CAL[i-1].mm);
        }
    }
    return CAL[CAL_POINTS - 1].mm;
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
    printf("\n\n\n\n\n");  // reserve lines for in-place update

    float prev_avg   = -1.0f;
    int stable_count = 0;

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
            printf("Error: all %d reads failed -- check wiring\n", num_samples);
            usleep(interval_ms * 1000);
            continue;
        }

        float avg_raw = (float)sum / valid;
        float voltage = (avg_raw / ADC_MAX_COUNT) * VOLTAGE_REF;

        float signed_delta = (prev_avg < 0.0f) ? 9999.0f : (avg_raw - prev_avg);
        float delta        = signed_delta < 0.0f ? -signed_delta : signed_delta;
        prev_avg = avg_raw;

        if (delta > STABLE_DELTA_THRESHOLD)
            stable_count = 0;
        else if (stable_count < STABLE_CONFIRM_CYCLES)
            stable_count++;

        float temp_c        = read_ds18b20_temp();
        float adc_for_depth = (temp_c != -999.0f)
                              ? avg_raw + TEMP_SLOPE * (temp_c - TEMP_REF_C)
                              : avg_raw;
        float depth_mm = adc_to_mm(adc_for_depth);
        float pct      = (depth_mm / CAL_MAX_MM) * 100.0f;

        // Erase the 5 data lines and reprint in-place
        printf("\033[5A");
        printf("\033[2KRaw ADC:  %6.1f  (delta %+.1f)\n", avg_raw, signed_delta);
        printf("\033[2KVoltage:  %.4f V\n", voltage);
        if (temp_c == -999.0f)
            printf("\033[2KTemp:     sensor error (no compensation)\n");
        else
            printf("\033[2KTemp:     %.2f C\n", temp_c);
        if (stable_count < STABLE_CONFIRM_CYCLES) {
            printf("\033[2KLevel:    stabilizing..., %6.1f %%\n", pct);
            printf("\033[2KDepth:    stabilizing..., %6.1f mm\n", depth_mm);
        } else {
            printf("\033[2KLevel:   %6.1f %%\n", pct);
            printf("\033[2KDepth:   %6.1f mm\n", depth_mm);
        }
        fflush(stdout);

        usleep(interval_ms * 1000);
    }

    return 0;
}
