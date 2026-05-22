// TO COMPILE:
// gcc colorReadTest.c ../functions.c -lwiringPi -lm -o ../../Executables/colorReadTest

// Reads RGB color values normalized to how much ambient (clear) light is present
// Based on reference Dirt RGB values, calculates the distance between the read value
// and the dirt reference to detect if a reaction has occurred
// Outputs if a reaction has occured based on a percentage threshold,
// as well as the percent difference from dirt to
// colorReadTest.csv

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <math.h>
#include "../functions.h"
#define DEVICE_ID 0x39
#define COMMAND_REGISTER_BIT 0x80
#define MULTI_BYTE_BIT 0x20
int main(int argc, char *argv[]){
    int vals[argc-1];
    //intparse(argc, argv, vals);
    // printf("%x",(uint8_t)vals[0]);
    int fd = wiringPiI2CSetup(DEVICE_ID);
    if (fd == -1) {
        printf("Failed to init I2C communication.\n");
        return -1;
    }
    // enable 0b00000011 is another way of writing 0x03 = 3 with 0x meaning hex 
    // 0x80 enables register -> command register bit, 0x03 enables AEN and PON (power on)
    wiringPiI2CWriteReg8(fd, COMMAND_REGISTER_BIT, 0b00000011); 

    //0x81: integration time.
    wiringPiI2CWriteReg8(fd, 0x81, 0b00000000);

    //-added-
    //sets the wait long time - when asserted, wait cycles are increased by a in 
    // factor 12x from that programmed the WTIME resgister
    // 0b00000010 = 0x02
    // 0x8D: configuration register 1, sets wait long time
    wiringPiI2CWriteReg8(fd, 0x8D, 0b00000010); // configuration

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
    // 0x8F: control register one
    wiringPiI2CWriteReg8(fd, 0x8F, 0b00110010); // gain control register

    // makes sure data is ready before reading
    uint8_t status;
    do{
        status = wiringPiI2CReadReg8(fd, 0x93);
    } while (!(status & 0x01));  // keeps waiting until AVALID = 1, then runs code below.
    
    int clearb = wiringPiI2CReadReg8(fd, 0x94);
    uint8_t cleart = wiringPiI2CReadReg8(fd, 0x95);
    int redb = wiringPiI2CReadReg8(fd, 0x96);
    int greenb = wiringPiI2CReadReg8(fd, 0x98);
    int blueb = wiringPiI2CReadReg8(fd, 0x9A);

    uint8_t redt = wiringPiI2CReadReg8(fd, 0x97);
    uint8_t greent = wiringPiI2CReadReg8(fd, 0x99);
    uint8_t bluet = wiringPiI2CReadReg8(fd, 0x9B);

    int raw_r = redb   | (redt   << 8);
    int raw_g = greenb | (greent << 8);
    int raw_b = blueb  | (bluet  << 8);
    int raw_c = clearb | (cleart << 8);
    printf("Raw - Red: %d  Green: %d  Blue: %d  Clear: %d\n", raw_r, raw_g, raw_b, raw_c);

    // Saturation check — any channel at 65535 means the ADC is clipped; readings unreliable.
    // Fix: lower gain (0x8F) or shorten integration time (0x81).
    if (raw_r == 65535 || raw_g == 65535 || raw_b == 65535 || raw_c == 65535) {
        printf("Warning: saturation on one or more channels — lower gain or integration time.\n");
        return -1;
    }

    if (raw_c == 0) {
        printf("Clear channel is zero, cannot normalize.\n");
        return -1;
    }

    // Dark calibration offsets — cover sensor completely, record raw values, set DARK_* below.
    // Subtract before any other math; most impactful on blue which has the weakest signal.
    // Recalibrate if gain or integration time changes.
    #define DARK_R 13.0f  // replace with covered-sensor red reading
    #define DARK_G 4.0f  // replace with covered-sensor green reading
    #define DARK_B 4.0f  // replace with covered-sensor blue reading
    #define DARK_C 17.0f  // replace with covered-sensor clear reading

    // White calibration scale factors — point sensor at white surface, record raw values.
    // Recalibrate if gain or integration time changes.
    #define WHITE_R 8521.0f
    #define WHITE_G 6187.0f
    #define WHITE_B 5560.0f
    #define WHITE_C 7900.0f

    // Pipeline: subtract dark offset → normalize by clear → apply white scale factors
    float r = (((float)raw_r - DARK_R) / (raw_c - DARK_C)) * (WHITE_C / WHITE_R);
    float g = (((float)raw_g - DARK_G) / (raw_c - DARK_C)) * (WHITE_C / WHITE_G);
    float b = (((float)raw_b - DARK_B) / (raw_c - DARK_C)) * (WHITE_C / WHITE_B);

    // Clamp to [0, 255] — values can exceed 1.0 for colors brighter than the white reference
    int rgb_r = (int)(r * 255.0f);
    int rgb_g = (int)(g * 255.0f);
    int rgb_b = (int)(b * 255.0f);
    if (rgb_r > 255) rgb_r = 255;
    if (rgb_g > 255) rgb_g = 255;
    if (rgb_b > 255) rgb_b = 255;
    if (rgb_r < 0)   rgb_r = 0;
    if (rgb_g < 0)   rgb_g = 0;
    if (rgb_b < 0)   rgb_b = 0;

    printf("Cal  - Red: %.3f  Green: %.3f  Blue: %.3f\n", r, g, b);
    printf("RGB  - Red: %d  Green: %d  Blue: %d\n", rgb_r, rgb_g, rgb_b);

    // Target color references — fill in by reading the RGB line for each target liquid
    // under final operating conditions, averaging several samples for stability.
    // Dirt RGB  - Red: 123  Green: 122  Blue: 90
    // RGB  - Red: 141  Green: 106  Blue: 89
    #define REF_R_A 141
    #define REF_G_A 106
    #define REF_B_A 89
    // Solution RGB  - Red: 105  Green: 128  Blue: 114
    // #define REF_R_B 105
    // #define REF_G_B 128
    // #define REF_B_B 114
    // Match threshold (Euclidean distance in 0–255 RGB space, max ~441).
    // Must be larger than the natural variation of each target and smaller than
    // half the distance between REF_A and REF_B. Tune with real data.
    #define MATCH_THRESHOLD 25.0f

    float dr_a = rgb_r - REF_R_A, dg_a = rgb_g - REF_G_A, db_a = rgb_b - REF_B_A;
    // float dr_b = rgb_r - REF_R_B, dg_b = rgb_g - REF_G_B, db_b = rgb_b - REF_B_B;
    float dist_a = sqrtf(dr_a * dr_a + dg_a * dg_a + db_a * db_a);
    // float dist_b = sqrtf(dr_b * dr_b + dg_b * dg_b + db_b * db_b);
    // printf("Dist - A: %.1f  B: %.1f\n", dist_a, dist_b);
    printf("Dist - A: %.1f \n", dist_a);

    float dist_a_pct = (dist_a / 441.67f) * 100.0f;
    int detected = 0;
    // detection line threshold still needs to be determined
    if (dist_a_pct > 7.0f) {
        detected = 1;
    }

    // if (dist_a < dist_b && dist_a < MATCH_THRESHOLD) {
    if (dist_a < MATCH_THRESHOLD) {
        printf("Match: target A (Dirt)\n");
    }
    // } else if (dist_b < MATCH_THRESHOLD) {
    //     printf("Match: target B (Reaction)\n");
    // } else {
    //     printf("Match: none\n");
    // }

    FILE *csv = fopen("colorReadTest.csv", "w");
    if (csv) {
        fprintf(csv, "pct_diff\n");
        fprintf(csv, "%.2f\n", dist_a_pct);
        fclose(csv);
    }

    return 0;
}
