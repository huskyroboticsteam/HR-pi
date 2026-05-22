#include "../../functions.h"
#include "../../pins.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <signal.h>
#define LEFTEN 4 // we only move to the left (and only use the left pin)
#define period_duty 10

struct PWMinput {
  int pin;
  int *period;
};
long int get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1E6 + tv.tv_usec;
}
void *softPWM(void *input) {
  long int current_t = get_time();
  // int period = *((int *)input); // 1-100
  struct PWMinput *args = (struct PWMinput *)input;
  printf("Period: %i pin: %i\n", *(args->period), args->pin);
  while (1) {
    digitalWrite(args->pin, 1);
    usleep(*(args->period) * 100);
    digitalWrite(args->pin, 0);
    usleep((100 - *(args->period)) * 100);
  }
}

// rotates a full revolution
void move() {
  int period = 0;
  struct PWMinput arguments;
  arguments.pin = LEFTEN;
  arguments.period = &period;
  pthread_t pwmProc;

  int start_cf = fpga_safetran(ENC_CENTRIFUGE_ABS);

  pinMode(LEFTEN, OUTPUT);
  period = 100;
  pthread_create(&pwmProc, NULL, softPWM, &arguments);
  uint32_t fpga_out;
  sleep(5);
  period = period_duty;
  sleep(3);
  while (1) {
    fpga_out = fpga_safetran(ENC_CENTRIFUGE_ABS);
    if (fpga_out == start_cf) {
      break;
    }
  }
  pthread_cancel(pwmProc);
  pthread_join(pwmProc, NULL);
  digitalWrite(LEFTEN, 0);
}

int main(int argc, char *argv[]) {
  wiringPiSetupPinType(WPI_PIN_WPI);
  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);
}
