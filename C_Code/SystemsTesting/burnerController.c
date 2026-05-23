// Excecution:

//cd C_Code
//gcc SystemsTesting/burnerController.c functions.c -lwiringPi -o ../Executables/burnerController
//sudo ./Executables/burnerController 60 5
//      # heat to 65 celcius, maintain at 60
//      

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <time.h>

#include "../pins.h"
#include "../functions.h"

// 1-Wire sysfs root; DS18B20 devices appear as "28-XXXXXX" subdirs here.
#define BASE_DIR "/sys/bus/w1/devices/"
#define SAMPLE_INTERVAL_US 500000   // 500 ms between temp reads / control updates
#define MAX_SENSOR_FAILS 50        // abort after this many consecutive bad reads
#define SENSOR_ERROR -999.0f        // sentinel returned by read_ds18b20_temp() on failure
#define MIXER_PIN 24
#define STOP_TIME 600

// Ctrl+C flips this flag; the main loop checks it each iteration to exit cleanly.
static volatile sig_atomic_t sigint = 0;
static void intHandler(int _) { (void)_; sigint = 1; }

// Reads the DS18B20 temperature via the kernel 1-Wire sysfs interface.
// Returns degrees C, or -999.0 on any failure (no device, file unreadable, etc).
float read_ds18b20_temp(void) {
    // Cache the device path between calls so we only scan the directory once.
    static char cached_device_file[256] = {0};
    static int path_found = 0;

    // First call: find the "28-*" subdir and build the path to its "temperature" file.
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

    FILE *f = fopen(cached_device_file, "r");
    if (f == NULL) {
        // Sensor was likely unplugged — invalidate the cache so we rescan next call.
        path_found = 0;
        return -999.0;
    }

    // sysfs returns the temperature in millidegrees C as an ASCII integer.
    char temp_str[16];
    float temp_c = -999.0;

    if (fgets(temp_str, sizeof(temp_str), f) != NULL) {
        temp_c = atof(temp_str) / 1000.0;
    }

    fclose(f);
    return temp_c;
}

// Active-HIGH burner relay: pin HIGH = burner energized.
static inline void burner_on(void)  { digitalWrite(BURNER_PIN, 1); }
static inline void burner_off(void) { digitalWrite(BURNER_PIN, 0); }

int main(int argc, char *argv[]) {
    // Register Ctrl+C handler so we can always shut the burner off on exit.
    signal(SIGINT, intHandler);

    // Expect exactly two args: target temperature and how far above it to overshoot.
    if (argc != 3) {
        fprintf(stderr, "usage: %s <target_C> <overshoot_C>\n", argv[0]);
        fprintf(stderr, "  heats until temp >= target+overshoot, then off until temp <= target, repeat\n");
        return 1;
    }

    // Parse args as floats. target = lower bound, upper = target + overshoot = shut-off point.
    float vals[2];
    floatparse(2, argv + 1, vals);
    float target = vals[0];
    float overshoot = vals[1];
    float upper = target + overshoot;
    float lower = target - overshoot;

    // Without a positive overshoot the hysteresis window collapses and the burner would chatter.
    if (overshoot <= 0.0f) {
        fprintf(stderr, "overshoot must be > 0 (got %.2f)\n", overshoot);
        return 1;
    }

    // GPIO setup: use wiringPi (wPi) pin numbering, configure burner pin as output, start OFF.
    wiringPiSetupPinType(WPI_PIN_WPI);
    pinMode(BURNER_PIN, OUTPUT);
    pinMode(MIXER_PIN, OUTPUT);
    burner_off();

    // Start in the heating phase (per spec: drive temp up to `upper` first).
    int heating = 1;
    burner_on();
    digitalWrite(MIXER_PIN, 1);

    // Main bang-bang control loop. Runs until SIGINT or repeated sensor failure.
    int fails = 0;
    time_t startTime = time(NULL);
    digitalWrite(MIXER_PIN, 1);
    while (!sigint) {
        if(time(NULL) - startTime >= STOP_TIME) {
            break;
        }
        float t = read_ds18b20_temp();

        // Sensor error path: count consecutive failures and abort if it persists.
        if (t == SENSOR_ERROR) {
            if (++fails >= MAX_SENSOR_FAILS) { //burner off --> on ---> poll again ---> if temp, continue main, if not repeat
                fprintf(stderr, "\nDS18B20 read failed %d times, retrying\n", fails);

                //turns burner off for 30 seconds (quits if runtime goes over or sigint)

                burner_off(); heating = 0;
                for (int i = 0; i < 30 && !sigint && time(NULL) - startTime < STOP_TIME; i++) {
                    sleep(1);
                }
                //check for sigint or runtime elapsed
                if (sigint || time(NULL) - startTime >= STOP_TIME) break;

                //turns burner on for 30 seconds (quits if runtime goes over or sigint)
                burner_on(); heating = 1;
                for (int i = 0; i < 30 && !sigint && time(NULL) - startTime < STOP_TIME; i++) {
                    sleep(1);
                }
                //check for sigint or runtime elapsed
                if (sigint || time(NULL) - startTime >= STOP_TIME) break;
                
                //fails reset
                fails = 0;
            }
            //checks if sensor is working again.
            usleep(SAMPLE_INTERVAL_US);
            continue;
        }
        fails = 0;  // good read — reset the failure counter

        // Hysteresis: only flip state at the two thresholds. Between them, leave burner alone.
        if (heating && t >= upper)       { burner_off(); heating = 0; }  // reached upper -> stop heating
        else if (!heating && t <= lower){ burner_on(); heating = 1; }  // cooled to target -> heat again

        // Live status line; \r overwrites in place so the terminal doesn't scroll.
        printf("\rT=%.2f C  target=%.2f  upper=%.2f  burner=%s   ",
               t, target, upper, heating ? "ON " : "OFF");
        fflush(stdout);

        usleep(SAMPLE_INTERVAL_US);
    }

    // Any exit path (SIGINT or sensor abort) must leave the burner OFF.
    burner_off();
    digitalWrite(MIXER_PIN, 0);
    printf("\nburner OFF, exiting\n");
    return 0;
}
