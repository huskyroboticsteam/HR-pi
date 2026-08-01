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

// Non-static so a caller can set this from a unified SIGINT handler. See
// mixingChamberAutomation.c for an example.
volatile int pump_sigint = 0;
static void pump_intHandler(int dummy) { pump_sigint = 1; }

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

#ifdef BUILD_WATERLEVELREAD_MAIN
int main(int argc, char *argv[]) {
    // if (argc < 3) {
    //     printf("Usage: %s <wPi_pin> <time_s>\n", argv[0]);
    //     return -1;
    // }
    int fd = wiringPiI2CSetup(ADS1015_ADDR);
    if (fd == -1) {
        printf("Failed to open I2C for ADS1015 at 0x%02X\n", ADS1015_ADDR);
        return -1;
    }
    // int pin      = atoi(argv[1]);
    // float time_s = atof(argv[2]);
    signal(SIGINT, pump_intHandler);
    wiringPiSetup();
    wiringPiSetupPinType(WPI_PIN_WPI);
    printf("%d\n", ads1015_read(fd,DEFAULT_CHANNEL));
    return 0;
}
#endif