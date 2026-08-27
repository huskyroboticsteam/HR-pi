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
    int period = 0;
    struct PWMinput arguments;
    arguments.pin = string_to_int(argv[1]);
    arguments.period = &period;
    pthread_t pwmProc;
    pinMode(string_to_int(argv[1]), OUTPUT);
    period = string_to_int(argv[2]);

    pthread_create(&pwmProc, NULL, softPWM, &arguments);
    for (int i=0; i<string_to_int(argv[3]); i++){
        usleep(10000);
        if (sigint){
            break;
        }
    }
    // usleep(1000 * string_to_int(argv[3]));

    pthread_cancel(pwmProc);
    pthread_join(pwmProc, NULL);
    digitalWrite(string_to_int(argv[1]), 0);
    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, intHandler);
    return run_soft_pwm_cli(argc, argv);
}