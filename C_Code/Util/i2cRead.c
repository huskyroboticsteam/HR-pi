#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../functions.h"
#define DEVICE_ID 0x39
#define COMMAND_REGISTER_BIT 0x80
#define MULTI_BYTE_BIT 0x20

static int run_i2c_read_test(int argc, char *argv[]) {
    printf("%x\n", string_to_int(argv[1]));
    int fd = wiringPiI2CSetup((uint8_t)string_to_int(argv[1]));
    if (fd == -1) {
        printf("Failed to init I2C communication.\n");
        return -1;
    }
    uint8_t result = wiringPiI2CReadReg8(fd, string_to_int(argv[2]));
    printf("Result: %x\n", result);
    return 0;
}

int main(int argc, char *argv[]) {
    return run_i2c_read_test(argc, argv);
}