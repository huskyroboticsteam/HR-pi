#include "../../functions.h"
#include "../../pins.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

// 882 encoder position = position 0
// 121 encoder position = position 1
// 358 encoder position = position 2
// 625 encoder position = position 3

#define SERVO_CHANNEL CENTRIFUGE_SERVO_CHANNEL      // FPGA PWM channel for the centrifuge servo
#define SERVO_PERIOD 10000                      // full duty (on)
#define SERVO_OFF 0                             // off
// #define CPR 1018                                // Counts Per Revolution of the absolute encoder
#define ENC_ABS ENC_CENTRIFUGE_ABS

static volatile int sigint = 0;

static void intHandler(int dummy) {
  (void)dummy;
  sigint = 1;
  fpga_pwm_uptime(SERVO_CHANNEL, SERVO_OFF);
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

#ifdef BUILD_ROTATESERVO_MAIN
int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);

  if (argc < 2) {
    printf("Usage: %s <position 0-3>\n", argv[0]);
    return 1;
  }

  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);

  int pos_preset = vals[0];
  int target_pos;

  switch (pos_preset) {
    case 0:
      target_pos = 882;
      break;
    case 1:
      target_pos = 121;
      break;
    case 2:
      target_pos = 358;
      break;
    case 3:
      target_pos = 625;
      break;
    default:
      printf("Invalid position. Must be between 0 and 3.\n");
      return 1;
  }

  rotateTo(target_pos);

  return 0;
}
#endif