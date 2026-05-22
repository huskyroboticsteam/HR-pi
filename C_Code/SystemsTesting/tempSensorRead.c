#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>

#define AHT_ADDR 0x38 /* 7-bit I2C address; datasheet shows 0x70/0x71 as 8-bit R/W forms */

int fd;

int aht_init(void) {
    fd = wiringPiI2CSetup(AHT_ADDR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open I2C device\n");
        return -1;
    }

    usleep(100000); /* datasheet 6.2.1: wait >=100ms after power-on */

    uint8_t status;
    if (read(fd, &status, 1) != 1) {
        perror("Failed to read status byte");
        return -1;
    }
    printf("Init status: 0x%02x\n", status);

    if ((status & 0x18) != 0x18) {
        fprintf(stderr, "Sensor not calibrated (status=0x%02x)\n", status);
        return -1;
    }
    return 0;
}

int aht_read(float *temperature_c, float *humidity_rh) {
    usleep(10000); /* datasheet 6.2.2: wait >=10ms between status check and trigger */

    uint8_t cmd[3] = {0xAC, 0x33, 0x00}; /* datasheet 6.2.2: measurement trigger */
    if (write(fd, cmd, 3) != 3) {
        perror("Failed to send trigger command");
        return -1;
    }

    usleep(80000); /* datasheet 6.2.3: wait >=80ms for measurement */

    uint8_t data[7];
    if (read(fd, data, 7) != 7) {
        perror("Failed to read data");
        return -1;
    }

    if (data[0] & 0x80) { /* datasheet 6.2.3: status bit 7 set means still busy */
        fprintf(stderr, "Sensor still busy\n");
        return -1;
    }

    /* datasheet 7: two 20-bit values packed into data[1..5].
     * data[3] is split: upper nibble = humidity tail, lower nibble = temperature head. */
    uint32_t raw_h = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    uint32_t raw_t = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

    *humidity_rh   = ((float)raw_h / 1048576.0f) * 100.0f;
    *temperature_c = ((float)raw_t / 1048576.0f) * 200.0f - 50.0f;
    return 0;
}

static int run_temp_reading(void) {
    float temperature, humidity;

    if (aht_read(&temperature, &humidity) == 0) {
        printf("Temperature: %.2f C\n", temperature);
        printf("Humidity: %.2f %%RH\n", humidity);
        return 0;
    }
    printf("Reading Error\n");
    return -1;
}

#ifdef BUILD_TEMPREAD_MAIN
int main(void) {
    if (aht_init() < 0) {
        return -1;
    }

    run_temp_reading();
    close(fd);
    return 0;
}
#endif