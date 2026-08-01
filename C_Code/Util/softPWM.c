#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../functions.h"
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
// #define LEFTEN 4//4
// #define RIGHTEN 5//5
struct PWMinput {
    int pin;
    int* period;
};
int sigint=0;
static void intHandler(int dummy) { sigint = 1; }
void* softPWM(void* input){
    // int period = *((int *)input); // 1-100
    struct PWMinput* args = (struct PWMinput*)input;
    printf("Period: %i pin: %i\n", *((args)->period), (args)->pin);
    while (1){
        digitalWrite(args->pin, 1);
        usleep(*((args)->period)*100);
        digitalWrite((args)->pin, 0);
        usleep((100-*((args)->period))*100);
    }
}

static int run_soft_pwm_cli(int argc, char *argv[]) {
    wiringPiSetupPinType(WPI_PIN_WPI);
    int vals[argc - 1];
    intparse(argc - 1, argv + 1, vals);
    int period = 0;
    struct PWMinput arguments;
    arguments.pin = vals[0];
    arguments.period = &period;
    pthread_t pwmProc;
    pinMode(vals[0], OUTPUT);
    period = vals[1];

    pthread_create(&pwmProc, NULL, softPWM, &arguments);
    for (int i=0; i<vals[2]; i++){
        usleep(10000);
        if (sigint){
            break;
        }
    }
    // usleep(1000 * vals[2]);

    pthread_cancel(pwmProc);
    pthread_join(pwmProc, NULL);
    digitalWrite(vals[0], 0);
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, intHandler);
    return run_soft_pwm_cli(argc, argv);
}