#include "../functions.h"
#include "../../pins.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <math.h>
// #include <signal.h>
#define LEFTEN                                                                 \
  CENTRIFUGE_PIN // we only move to the left (and only use the left pin)
#define tpr 1018.0

int sigint = 0;
void intHandler(int dummy) { sigint = 1; }

struct PWMinput {
  int pin;
  volatile int *period;
};
// long int get_time() {
//   struct timeval tv;
//   gettimeofday(&tv, NULL);
//   return tv.tv_sec * 1E6 + tv.tv_usec;
// }
void *softPWM(void *input) {
  //   long int current_t = get_time();
  // int period = *((int *)input); // 1-100
  struct PWMinput *args = (struct PWMinput *)input;
  //   printf("Period: %i pin: %i\n", *(args->period), args->pin);
  // printf("working");
  while (1) {
    digitalWrite(args->pin, 1);
    usleep(*(args->period) * 100);
    digitalWrite(args->pin, 0);
    usleep((100 - *(args->period)) * 100);
  }
}

void rotateBy(int degrees) {
  int period = 0;
  struct PWMinput arguments;
  arguments.pin = LEFTEN;
  arguments.period = &period;
  pthread_t pwmProc;
  int current = fpga_safetran(ENC_CENTRIFUGE_ABS);
  // 2. Calculate target position (current + how many teeth we want to move by)
  int target_cf = ((degrees)*tpr) / 360;

  printf("Current: %i, Target: %i\n", current, target_cf);
  // 3. Move the centrifuge
  pinMode(LEFTEN, OUTPUT);
  period = 20;
  usleep(5000);
  period = 5;
  // period = string_to_int(argv[1]);
  //(to make it manual)
  volatile int fpga_out;
  fpga_out = fpga_safetran(ENC_CENTRIFUGE_ABS);
  int distance_remaining = abs(target_cf - fpga_out);
  pthread_create(&pwmProc, NULL, softPWM, &arguments);
  // sleep(5);
  // 4. Wait until the target position is reached
  while (distance_remaining > 10) {
    // usleep(100E3);
    if (sigint) {
      digitalWrite(LEFTEN, 0);
      break;
    }
    usleep(1E3);
    fpga_out = fpga_safetran(ENC_CENTRIFUGE_ABS);
    distance_remaining = abs(target_cf - fpga_out);
  }
  pthread_cancel(pwmProc);
  pthread_join(pwmProc, NULL);
  digitalWrite(LEFTEN, 0);
  //   usleep(100E3);
  fpga_out = fpga_safetran(ENC_CENTRIFUGE_ABS);
  printf("Final: %i\n", fpga_out);
}

int main(int argc, char *argv[]) {
  wiringPiSetupPinType(WPI_PIN_WPI);

  int degs = string_to_int(argv[1]);

  rotateBy(degs);
  return 0;
}