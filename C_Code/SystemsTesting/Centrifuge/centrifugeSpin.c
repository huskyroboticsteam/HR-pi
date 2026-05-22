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
#include <signal.h>
#include <stdlib.h>
#define SPIN_CHANNEL CENTRIFUGE_PIN // Centrifuge spin pin
#define SERVO_CHANNEL CENTRIFUGE_SERVO_CHANNEL      // FPGA PWM channel for the centrifuge servo
#define SERVO_PERIOD 10000                      // full duty (on)
#define SERVO_OFF 0                             // off
// #define CPR 1018                                // Counts Per Revolution of the absolute encoder
#define ENC_ABS ENC_CENTRIFUGE_ABS

static volatile int sigint = 0;

static void intHandler(int dummy) {
  (void)dummy;
  sigint = 1;
  digitalWrite(SPIN_CHANNEL, 0);
}

static void spin_off(void) {
  digitalWrite(SPIN_CHANNEL, 0);
}

static void pwm_off(void) { 
  fpga_pwm_uptime(SERVO_CHANNEL, SERVO_OFF); 
}

void rotateTo(int target_pos) {
  int current = fpga_safetran(ENC_ABS);

  printf("Current: %i, Target: %i\n", current, target_pos);

  // Turn on the centrifuge motor
  fpga_pwm_uptime(SERVO_CHANNEL, SERVO_PERIOD);
  int distance_remaining = abs(target_pos - current);

  // Wait until the target position is reached, 10 units before start slowing down
  while (distance_remaining > 5) {
    if (sigint) {
      break;
    }
    usleep(1E3);
    current = fpga_safetran(ENC_ABS);
    distance_remaining = abs(target_pos - current);
  }

  // Turn off the centrifuge motor
  pwm_off();

  current = fpga_safetran(ENC_ABS);
  printf("Final: %i\n", current);
}

void softPWM_ramp(int pin, int period_scale) {
  printf("Period: %i pin: %i\n", period_scale, pin);
  for(int i = 0; i <= period_scale; i++) {
    digitalWrite(pin, 1);
    usleep(i * 100);
    digitalWrite(pin, 0);
    usleep((period_scale - i) * 100);
  }
}

// duration_seconds does not account for time taken to ramp up
void spinFor(int duration_seconds) {
  int start_pos = fpga_safetran(ENC_ABS);
  printf("Spinning for %d seconds\n", duration_seconds);
  softPWM_ramp(SPIN_CHANNEL, 100);
  digitalWrite(SPIN_CHANNEL, 1);
  sleep(duration_seconds);
  spin_off();
  sleep(5); // give time for the motor to ramp down before we rotate to start_pos
  printf("Done spinning; rotating to initial position: %d\n", start_pos);
  rotateTo(start_pos);
}

int main(int argc, char* argv[]) {
  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);
  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);
  spinFor(vals[0]);
}