#include <wiringPi.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include "../functions.h"
#define DEVICE_ID 0x39
#define COMMAND_REGISTER_BIT 0x80
#define MULTI_BYTE_BIT 0x20

#ifdef BUILD_COLORREAD_MAIN
int main(int argc, char *argv[]){
    int vals[argc-1];
    intparse(argc, argv, vals);
    // printf("%x",(uint8_t)vals[0]);
    int fd = wiringPiI2CSetup(DEVICE_ID);
    if (fd == -1) {
        printf("Failed to init I2C communication.\n");
        return -1;
    }

    
    //enable 0b00000011 is another way of writing 0x03 = 3 with 0x meaning hex 
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x00, 0b00000001); 
    usleep(3000);
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x00, 0b00000011); 

    //RGBC integration time
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x01, 0b00000000); 

    //-added-
    //RGBC INTEGRATION TIME REGISTER
    //0xFF 1 cycle (default)
    //0xF6 10 cycles
    //0xD6 42 cycles
    //0xC0 64 cycles
    //0x00 256 cycles
    // Max Count = #cycles * 1024
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x01, 0b11000000); // 0xC0 65335 max count, 64 cycles
  
    //-added-
    //sets the wait long time - when asserted, wait cycles are increased by a in 
    // factor 12x from that programmed the WTIME resgister
    // 0b00000010 = 0x02
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x03, 0b11111111);
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x0D, 0b00000010); // configuration

    //-added-
    // provides RGBC gain settings and a control for managing proximity reading 
    // in event the analog circuitry becomes satruated 
    // bit 5 must be set=1
    // AGAIN 1:0 field value 00 -> 1x, 01 -> 4x, 10 -> 16x, 11 -> 60x
    // 3:2 reserved 00
    // PSAT 4 0 -> PDATA output regardless of ambient light level
    // PSAT 4 1 -> PDATA output equalt to dark current value if saturated
    // 5 reserved 1
    // PDRIVE 7:6 00 -> 100%, 01 -> 50%, 10 -> 25%, 11 -> 12.5% 
    // 00       1            1              00         10 
    // 100%  reserved  dark_if_saturated  reserved  16x_gain
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT | 0x0F, 0b00110011); // gain control register
    usleep(1000000);
    while (1) {
        int status;
        do {
            status = wiringPiI2CReadReg8(fd, COMMAND_REGISTER_BIT | 0x13);
        } while (!(status & 0x01));
        int redb = wiringPiI2CReadReg8(fd,  COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x16);
        int greenb = wiringPiI2CReadReg8(fd,  COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x18);
        int blueb = wiringPiI2CReadReg8(fd,  COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x1a);
        // printf("RED LOW: %d \n GREEN LOW: %d \n BLUE LOW: %d \n", redb, greenb, blueb);
        // usleep(2000000);
        uint8_t redt = wiringPiI2CReadReg8(fd,  COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x17);
        uint8_t greent = wiringPiI2CReadReg8(fd,  COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x19);
        uint8_t bluet = wiringPiI2CReadReg8(fd,  COMMAND_REGISTER_BIT | MULTI_BYTE_BIT | 0x1b);
        // printf("RED HIGH: %d \n GREEN HIGH: %d \n BLUE HIGH: %d \n", redt, greent, bluet);
        // usleep(2000000);
        printf("\033[1A\033[2K\r\033[1A\033[2K\r\033[1A\033[2K\r");
        printf("Red: %d\nGreen:%d\nBlue:%d\n", redb|(redt<<8), greenb|(greent<<8), blueb|(bluet<<8));
        // usleep(1000000);
    }

    
    //usleep(1000000);
    // printf("Red: %d\nGreen:%d\nBlue:%d\n", redt, greent, bluet);
    return 0;
}
#endif