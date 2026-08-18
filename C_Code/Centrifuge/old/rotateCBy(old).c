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
#include <stdlib.h>
// #include <math.h>
#include <signal.h>
#define LEFTEN CENTRIFUGE_PIN // we only move to the left (and only use the left pin)
#define tpr 2048
#define ENC ENC_CENTRIFUGE_INC

int sigint = 0;
void intHandler(int dummy) { sigint = 1; }

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

  // 1. Get current position
  int start_cf = (fpga_safetran(ENC));

  // 2. Calculate target position (current + how many teeth we want to move by)
  int target_cf = (int)(start_cf - ((degrees)*tpr)/360.0);

  printf("Current: %i Target: %i\n", start_cf, target_cf);
  // 3. Move the centrifuge
  pinMode(LEFTEN, OUTPUT);
  pthread_create(&pwmProc, NULL, softPWM, &arguments);

extern double findVelocity(int sample_time_us);

  // Spin-up logic using findVelocity
  period = 10;
  while (!sigint) {
      double velocity = findVelocity(10000); // 10ms sample time
      
      if (velocity > 500.0 || velocity < -500.0) { // If moving at a certain speed
          break;
      }
      
      int current_pos = (int)fpga_safetran(ENC);
      if (current_pos <= target_cf + 310) {
          break;
      }
      
      if (period < 100) {
          period += 5; // Incrementally increase
      }
  }
  
  period = 10; // Set to low value
  // period = vals[0]; //(to make it manual)
  // 4. Wait until the target position is reached

  int fpga_out = (int)fpga_safetran(ENC);
  // int distance_remaining = abs(target_cf - fpga_out);
  while (fpga_out <= target_cf + 300) {
    // usleep(10E3);
    if (sigint) {
      digitalWrite(LEFTEN, 0);
      break;
    }
    fpga_out = (int)fpga_safetran(ENC);
    // printf("%d", fpga_out);
    // distance_remaining = abs(target_cf - fpga_out);
  }
  pthread_cancel(pwmProc);
  pthread_join(pwmProc, NULL);
  digitalWrite(LEFTEN, 0);
  printf("Final: %i\n", fpga_out);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);
  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);

  int degs = vals[0];

  rotateBy(degs);
  return 0;
}
