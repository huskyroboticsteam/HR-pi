#include "../../functions.h"
#include "../../pins.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define LEFTEN                                                                 \
  CENTRIFUGE_PIN // we only move to the left (and only use the left pin)

int sigint = 0;
void intHandler(int dummy) { sigint = 1; }

void rotateTo(int target_cf) {
  int current = fpga_safetran(ENC_CENTRIFUGE_ABS);

  printf("Current: %i, Target: %i\n", current, target_cf);

  // Turn on the centrifuge motor
  pinMode(LEFTEN, OUTPUT);
  digitalWrite(LEFTEN, 1);

  int distance_remaining = abs(target_cf - current);

  // Wait until the target position is reached, 100 units before start slowing down
  while (distance_remaining > 100) {
    if (sigint) {
      digitalWrite(LEFTEN, 0);
      break;
    }
    usleep(1E3);
    current = fpga_safetran(ENC_CENTRIFUGE_ABS);
    distance_remaining = abs(target_cf - current);
  }

  // Turn off the centrifuge motor
  digitalWrite(LEFTEN, 0);

  current = fpga_safetran(ENC_CENTRIFUGE_ABS);
  printf("Final: %i\n", current);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);

  if (argc < 2) {
    printf("Usage: %s <target_encoder_position>\n", argv[0]);
    return 1;
  }

  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);

  int target_pos = vals[0];

  rotateTo(target_pos);

  return 0;
}
