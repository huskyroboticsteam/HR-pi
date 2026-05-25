#ifndef WATERLEVELADS1015_H
#define WATERLEVELADS1015_H

#include <stdint.h>

#define ADS1015_ADDR    0x48
#define REG_CONVERSION  0x00
#define REG_CONFIG      0x01

#define OS_START        (1 << 15)
#define MUX_AIN0_GND    (0x04 << 12)
#define PGA_4_096V      (0x02 << 9)
#define MODE_SINGLE     (1 << 8)
#define DR_1600SPS      (0x01 << 5)
#define COMP_DISABLE    0x03

#define ADC_MAX_COUNT   2047
#define VOLTAGE_REF     2.048f

#define DEFAULT_CHANNEL       0
#define DEFAULT_NUM_SAMPLES   50
#define DEFAULT_INTERVAL_MS   100

#define MAX_STABLE_THRESHOLD    5.0f
#define MIN_ADC_MAX_COUNTS      500
#define MAX_STABLE_CONFIRM_CYCLES 10

int16_t ads1015_read(int fd, int channel);
int ads1015_detect_change(int fd, int channel, int num_samples, int interval_ms, float threshold, int confirm_cycles);
#endif