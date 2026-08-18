#include "../../functions.h"
#include "../../pins.h"
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

/* Encoder channel 2: incremental centrifuge position */
#define ENC ENC_CENTRIFUGE_INC

/* Ticks per full*/
#define TPR 2048

/* FPGA PWM motor index */
#define FPGA_PWM_MOTOR 5

#define TICK_TOLERANCE 300

static volatile int sigint = 0;

static void intHandler(int dummy) {
  (void)dummy;
  sigint = 1;
  fpga_pwm_uptime(FPGA_PWM_MOTOR, 0);
}

static void pwm_off(void) { fpga_pwm_uptime(FPGA_PWM_MOTOR, 0); }

/**
 * Rotate the one-direction centrifuge motor until encoder channel 2 has changed by
 * approximately `degrees` (same direction convention as the legacy soft-PWM tool:
 * positive degrees → count decreases toward start - delta).
 */
static void rotate_by_degrees(int degrees, uint32_t pwm_period, uint32_t pwm_uptime) {
  uint32_t start = fpga_safetran(ENC);
  int64_t delta = (int64_t)((double)degrees * (double)TPR / 360.0 + 0.5);
  int64_t goal = (int64_t)start - delta;

  printf("start_ticks=%u goal_ticks=%lld delta=%lld\n", (unsigned)start, (long long)goal,
         (long long)delta);

  fpga_pwm_period(FPGA_PWM_MOTOR, pwm_period);
  fpga_pwm_uptime(FPGA_PWM_MOTOR, pwm_uptime);

  while (!sigint) {
    uint32_t cur = fpga_safetran(ENC);
    if ((int64_t)cur <= goal + TICK_TOLERANCE) {
      break;
    }
    usleep(1000);
  }

  pwm_off();
  uint32_t end = fpga_safetran(ENC);
  printf("end_ticks=%u\n", (unsigned)end);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s degrees [pwm_uptime [pwm_period]]\n", argv[0]);
    fprintf(stderr, "defaults: uptime=%u period=%u (FPGA hard PWM)\n", 1200U, 2000U);
    return 1;
  }

  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);

  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);

  int degs = vals[0];
  if (degs <= 0) {
    fprintf(stderr, "degrees must be positive (one-direction motor)\n");
    return 1;
  }
  uint32_t uptime = (argc >= 3) ? (uint32_t)vals[1] : 1200U;
  uint32_t period = (argc >= 4) ? (uint32_t)vals[2] : 2000U;

  if (period == 0 || uptime > period) {
    fprintf(stderr, "invalid PWM: need period>0 and uptime<=period\n");
    return 1;
  }

  rotate_by_degrees(degs, period, uptime);
  return 0;
}
