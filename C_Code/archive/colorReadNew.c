#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../functions.h"
#include <stdio.h>
#include <stdint.h>
#define DEVICE_ID 0x39
#define COMMAND_REGISTER_BIT 0x80
#define MULTI_BYTE_BIT 0x20

static int open_color_i2c(void) {
    int fd = wiringPiI2CSetup(DEVICE_ID);
    if (fd == -1) {
        printf("Failed to init I2C communication.\n");
    }
    return fd;
}

static void configure_color_sensor_new(int fd) {
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x81, 0b00000000);
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x81, 0b00000010);
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x80, 0b00000011);
}

static int run_color_read_new(int argc, char *argv[]) {
    int vals[argc - 1];
    intparse(argc, argv, vals);
    (void)vals;
    int fd = open_color_i2c();
    if (fd == -1) {
        return -1;
    }
    configure_color_sensor_new(fd);

    while (1) {
        uint8_t status;
        do {
            status = wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x93);
        } while (!(status & 0x01));

        uint8_t clearb =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x94);
        uint8_t redb =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x96);
        uint8_t greenb =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x98);
        uint8_t blueb =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x9A);

        uint8_t cleart =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x95);
        uint8_t redt =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x97);
        uint8_t greent =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x99);
        uint8_t bluet =
            wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x9B);

        uint16_t clear = clearb | (cleart << 8);
        uint16_t red = redb | (redt << 8);
        uint16_t green = greenb | (greent << 8);
        uint16_t blue = blueb | (bluet << 8);

        if (clear > 0) {
            float r = (float)red / clear;
            float g = (float)green / clear;
            float b = (float)blue / clear;

            printf("\033[1A\033[2K\r\033[1A\033[2K\r\033[1A\033[2K\r");
            printf("Red: %.4f\nGreen:%.4f\nBlue:%.4f\n", r, g, b);
        }
    }
}

int main(int argc, char *argv[]) {
    return run_color_read_new(argc, argv);
}
