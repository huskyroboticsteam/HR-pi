// #include <stdlib.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
// #include <stdio.h>
#include "../../functions.h"
#include "../../pins.h"
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
// #include <signal.h>
#define CPR 663.0 / 6 // Counts per revolution
#define DATA_ADDR_ROTATE ENC_COLUMN_ROTATE

// -100 is deposit position
// 0 is home
// Note: to allow for inclusion of this file and raise_lower_column.c in the same file,
// the sigint variable and handler are duplicated here with different names. There's no
// difference in behavior, just to avoid a redefinition error.
int sigint2 = 0;
void intHandler2(int dummy) { sigint2 = 1; }

// Gets the encoder value
void enc_dec(int32_t *amount, float *degrees) {
    *amount = fpga_safetran(DATA_ADDR_ROTATE);
//   value;
//   *dir = value & (1 << 17);
    *degrees = (*amount * 360.0) / CPR;
}

int rotateTo(float target, int left, int right) {
  pinMode(left, OUTPUT);
  pinMode(right, OUTPUT);
  // int dir;
  int32_t amount;
  float degrees;
  enc_dec(&amount, &degrees);
  float distance_remaining = fabsf((degrees) - (target));
  if (degrees > target) {
    digitalWrite(left, 0);
    digitalWrite(right, 1);
  } else {
    digitalWrite(left, 1);
    digitalWrite(right, 0);
  }

  while ((distance_remaining) > 0.5) {
    if (sigint2) {
      digitalWrite(left, 0);
      digitalWrite(right, 0);
      break;
    }
    usleep(10000);
    enc_dec(&amount, &degrees);
    distance_remaining = fabsf((degrees) - (target));
  }
  digitalWrite(left, 0);
  digitalWrite(right, 0);
  printf("Target: %f\nActual: %f\n", target, degrees);
  return 0;
}
#ifdef BUILD_ROTATETO_MAIN
int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler2);
  wiringPiSetupPinType(WPI_PIN_WPI);
  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);
  rotateTo(vals[0], H1A_3, H1A_4);
  return 0;
}
#endif