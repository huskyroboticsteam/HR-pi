#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "waterLevelADS1015.h"
#include "waterLevel.h"

static volatile int sigint = 0;
static void intHandler(int dummy) { sigint = 1; }

float pump_delta_mm(int fd, int pin, float delta_mm, float cal_max_adc) {
    int channel     = DEFAULT_CHANNEL;
    int num_samples = DEFAULT_NUM_SAMPLES;
    int interval_ms = DEFAULT_INTERVAL_MS;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);

    float start_mm = waterLevel_read_stable_mm(fd, channel, num_samples,
                                               interval_ms, cal_max_adc);
    if (delta_mm == 0.0f) return start_mm;

    float target_mm = start_mm + delta_mm;
    int pumping_up  = (delta_mm > 0.0f);

    digitalWrite(pin, 1);

    while (!sigint) {
        float current_mm = waterLevel_read_mm(fd, channel, num_samples, cal_max_adc);
        if (current_mm < 0.0f) continue;

        if (pumping_up && current_mm >= target_mm) break;
        if (!pumping_up && current_mm <= target_mm) break;
    }

    digitalWrite(pin, 0);

    return waterLevel_read_stable_mm(fd, channel, num_samples,
                                     interval_ms, cal_max_adc);
}

float pumpToTime(int pin, float time) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 1);

    for(int i = 0; i < time && !signit; i += 50) {
        usleep(50000);
    }
    
    digitalWrite(pin, 0);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <wPi_pin> <delta_mm>\n", argv[0]);
        return -1;
    }

    int pin         = atoi(argv[1]);
    float delta_mm  = atof(argv[2]);

    signal(SIGINT, intHandler);
    wiringPiSetupPinType(WPI_PIN_WPI);

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

    printf("Pumping pin %d by %+.2f mm...\n", pin, delta_mm);
    float final_mm = pump_delta_mm(fd, pin, delta_mm, cal_max_adc);
    printf("Final height: %.2f mm\n", final_mm);
    return 0;
}
