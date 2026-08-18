#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>

#define SDP_ADDR 0x25 //CHANGE TO ADDR OF SENSOR!!

int fd;

int sdp_init() {
    fd = wiringPiI2CSetup(SDP_ADDR);
    if (fd < 0) {
        fprintf(stderr, "Failed to open I2C device\n");
        return -1;
    }
    return 0;
}

int sdp_read(float *pressure_pa, float *temperature_c) {
    uint8_t cmd[2] = {0x36, 0x2F};
    if (write(fd, cmd, 2) != 2) {
        perror("Failed to send trigger command");
        return -1;
    }

    usleep(50000); // 50ms

    uint8_t data[9];
    if (read(fd, data, 9) != 9) {
        perror("Failed to read data");
        return -1;
    }

    int16_t pressure_raw = (int16_t)((data[0] << 8) | data[1]);
    int16_t scale_factor = (int16_t)((data[6] << 8) | data[7]);

    if (scale_factor == 0) {
        fprintf(stderr, "Invalid scale factor\n");
        return -1;
    }

    *pressure_pa = (float) pressure_raw / (float) scale_factor;

    return 0;
}

static int run_pressure_reading(void) {
    float pressure, temperature;

    if (sdp_read(&pressure, &temperature) == 0) {
        printf("Pressure: %.4f Pa\n", pressure);
        return 0;
    }
    printf("Reading Error\n");
    return -1;
}

int main(void) {
    if (sdp_init() < 0)
        return -1;

    run_pressure_reading();
    close(fd);
    return 0;
}