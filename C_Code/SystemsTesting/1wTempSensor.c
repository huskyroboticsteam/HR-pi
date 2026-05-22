#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <wiringPi.h>

#include "../pins.h"
#include "../functions.h"

#define BASE_DIR "/sys/bus/w1/devices/"

// Finds the sensor, reads the data, and returns a single float.
float read_ds18b20_temp(void) {
    static char cached_device_file[256] = {0};
    static int path_found = 0;

    // Search for the 1-Wire folder if we haven't found it yet
    if (!path_found) {
        DIR *dir = opendir(BASE_DIR);
        struct dirent *direntp;
        
        if (dir == NULL) return -999.0; 

        while ((direntp = readdir(dir)) != NULL) {
            if (strncmp(direntp->d_name, "28-", 3) == 0) {
                snprintf(cached_device_file, sizeof(cached_device_file), "%s%s/temperature", BASE_DIR, direntp->d_name);
                path_found = 1;
                break;
            }
        }
        closedir(dir);
        if (!path_found) return -999.0; 
    }

    // Read the file and convert the text to a float
    FILE *f = fopen(cached_device_file, "r");
    if (f == NULL) {
        path_found = 0; 
        return -999.0;
    }

    char temp_str[16];
    float temp_c = -999.0;
    
    if (fgets(temp_str, sizeof(temp_str), f) != NULL) {
        temp_c = atof(temp_str) / 1000.0;
    }
    
    fclose(f);
    return temp_c;
}

#ifdef BUILD_1WTEMP_MAIN
int main(int argc, char *argv[]) {
    if (wiringPiSetup() == -1) {
        return -1;
    }

    // Grab the single temperature reading
    float current_temp = read_ds18b20_temp();

    // Print raw float value 
    if (current_temp != -999.0) {
        printf("%.2f\n", current_temp);
    } else {
        printf("-999.0\n"); 
    }
    
    fflush(stdout);
    
    return 0;
}
#endif