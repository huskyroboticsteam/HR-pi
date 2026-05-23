#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
// #include "waterLevel.h"

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

static volatile int sigint = 0;
static void intHandler(int dummy) { sigint = 1; }

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
        if (sigint) {
            break;
        }
         
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

void pumpToTime(int fd, int pin, float time_s) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 1);
    printf("Detecting for water entering mixer... \n");
    int waterEntered = ads1015_detect_change(fd, DEFAULT_CHANNEL, DEFAULT_NUM_SAMPLES,
                       DEFAULT_INTERVAL_MS, MAX_STABLE_THRESHOLD, MAX_STABLE_CONFIRM_CYCLES);
    if (waterEntered) {
        printf("Water Detected, timer started. \n");
        for (int i = 0; i < time_s; i++) {
            sleep(1);
            if (sigint) {
                break;
            }
        }
    }
    printf("Timer finished, ending pump \n");
    digitalWrite(pin, 0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <wPi_pin> <time_s>\n", argv[0]);
        return -1;
    }
    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if (fd == -1) {
        printf("Failed to open I2C for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return -1;
    }
    int pin      = atoi(argv[1]);
    float time_s = atof(argv[2]);
    signal(SIGINT, intHandler);
    wiringPiSetup();
    wiringPiSetupPinType(WPI_PIN_WPI);
    printf("Pumping pin %d for %.1f seconds\n", pin, time_s);
    pumpToTime(fd, pin, time_s);
    printf("Done\n");
    return 0;
}