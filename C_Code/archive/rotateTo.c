// #include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include "../functions.h"
#include "../pins.h"
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
// #include <signal.h>
#define left H1A_3
#define right H1A_4
#define CPR 663.0 / 6 // Counts per revolution
#define DATA_ADDR 5
int sigint = 0;
void intHandler(int dummy) { sigint = 1; }

static int rotate_to_target_degrees(int target_deg) {
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);
  int dir;
  int16_t amount;
  float degrees;

  enc_dec( &amount, &degrees);
  int16_t distance_remaining = fabsf(degrees - (float)target_deg);
  if (degrees > target_deg) {
    digitalWrite(left, 0);
    digitalWrite(right, 1);
  } else {
    digitalWrite(left, 1);
    digitalWrite(right, 0);
  }

  while (distance_remaining > 5) {
    if (sigint) {
      digitalWrite(left, 0);
      digitalWrite(right, 0);
      break;
    }
    usleep(10000);
    enc_dec( &amount, &degrees);
    distance_remaining = fabsf(degrees - (float)target_deg);
  }
  digitalWrite(left, 0);
  digitalWrite(right, 0);
  printf("Target: %i\nActual: %f\n", target_deg, degrees);
  return 0;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);
  return rotate_to_target_degrees(string_to_int(argv[1]));
}