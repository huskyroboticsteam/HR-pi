#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "../../functions.h"
#include "../../pins.h"

#define DATA_ADDR ENC_RAISE_LOWER

/* Absolute target in encoder ticks. Tune for your column. */
#define TICK_TOLERANCE 5 /* stop when within this many ticks of target */
/* Motion monitoring: if encoder speed stays below this while still far from the
 * target, the column is treated as stuck. Tune after measuring normal cruise speed. */
#define CONTROL_SAMPLE_US 5000
#define MIN_SPEED_TICKS_S_N 580
#define MIN_SPEED_TICKS_S_P 550
#define STALL_CONFIRM_SAMPLES 3
#define STALL_GRACE_US 50000 /* ignore stall until the motor has had time to ramp */
#define STALL_MIN_GAP_TICKS 12 /* do not apply stall logic when this close to target */

static int sigint = 0;
static void intHandler(int dummy) { sigint = 1; }

/** Same SPI command as Executables/resetEnc (fpga_reset_encoder). */
static void ResetENC(uint8_t encoder_channel) {
  fpga_reset_encoder(encoder_channel);
}

int get_fpga_bit(int channel, int bit) {
  uint32_t data = fpga_safetran(channel);
  return (data >> bit) & 1;
}

/** True when the top Hall sensor reports magnet present (bit reads 0; idle is 1). */
static int column_at_top_hall(void) {
  return (get_fpga_bit(HALL_CHANNEL, COLUMN_TOP_HALL_BIT) == 0);
}

static int32_t read_position_ticks(void) { return fpga_safetran(DATA_ADDR); }

/**
 * Instantaneous encoder speed from two samples (ticks/s).
 * Sign matches encoder direction: rising tick count → positive.
 */
static float column_velocity_ticks_s(int32_t ticks_before, int32_t ticks_after,
                                     long elapsed_usec) {
  if (elapsed_usec <= 0) {
    return 0.0f;
  }
  float delta_ticks = (float)(ticks_after - ticks_before);
  return delta_ticks * (1000000.0f / (float)elapsed_usec);
}

static long timeval_diff_usec(const struct timeval *a, const struct timeval *b) {
  return (long)(b->tv_sec - a->tv_sec) * 1000000L + (b->tv_usec - a->tv_usec);
}

/**
 * Returns 1 if measured speed magnitude is below the minimum (ticks/s).
 */
static int column_stall_detected(float speed_ticks_s) {
  
  return (fabsf(speed_ticks_s) < MIN_SPEED_TICKS_S_N)&&(speed_ticks_s < 0)||(fabsf(speed_ticks_s) < MIN_SPEED_TICKS_S_P)&&(speed_ticks_s > 0);
}

static int32_t ticks_remaining(int32_t ticks, int32_t target_ticks) {
  int64_t diff = (int64_t)ticks - (int64_t)target_ticks;
  if (diff < 0) {
    diff = -diff;
  }
  return (int32_t)diff;
}

static int raiseLowerTo(int32_t target_ticks, int raise_pin, int lower_pin) {
  pinMode(raise_pin, OUTPUT);
  pinMode(lower_pin, OUTPUT);

  int32_t ticks = read_position_ticks();
  int32_t distance_remaining = ticks_remaining(ticks, target_ticks);

  if ((ticks < target_ticks) && column_at_top_hall()) {
    ResetENC(ENC_COLUMN_RL_INDEX);
    ticks = read_position_ticks();
    // Unnecessary error
    // fprintf(stderr,
    //         "ERROR: Column top Hall limit active; cannot raise toward %d ticks "
    //         "(current %d ticks). Encoder reset at top.\n",
    //         target_ticks, ticks);
    digitalWrite(raise_pin, 0);
    digitalWrite(lower_pin, 0);
    printf("Column is at top hall limit.\n");
    return 0;
  }

  if (ticks > target_ticks) {
    digitalWrite(raise_pin, 0);
    digitalWrite(lower_pin, 1);
  } else {
    digitalWrite(raise_pin, 1);
    digitalWrite(lower_pin, 0);
  }

  struct timeval move_start, tv_a, tv_b;
  gettimeofday(&move_start, NULL);
  int stall_count = 0;

  while (distance_remaining > TICK_TOLERANCE) {
    if (sigint) {
      digitalWrite(raise_pin, 0);
      digitalWrite(lower_pin, 0);
      break;
    }

    int32_t ticks_start = ticks;
    gettimeofday(&tv_a, NULL);
    usleep(CONTROL_SAMPLE_US);
    ticks = read_position_ticks();
    gettimeofday(&tv_b, NULL);
    long elapsed = timeval_diff_usec(&tv_a, &tv_b);
    float speed_ticks_s = column_velocity_ticks_s(ticks_start, ticks, elapsed);
    long since_start_us = timeval_diff_usec(&move_start, &tv_b);

    distance_remaining = ticks_remaining(ticks, target_ticks);

    if ((ticks < target_ticks) && column_at_top_hall()) {
      digitalWrite(raise_pin, 0);
      digitalWrite(lower_pin, 0);
      ResetENC(ENC_COLUMN_RL_INDEX);
      ticks = read_position_ticks();
      // fprintf(stderr,
      //         "Stopped: column top Hall limit reached before target "
      //         "(target %d ticks, actual %d ticks). Encoder reset at top.\n",
      //         target_ticks, ticks);
      printf("Column is at top hall limit.\n");
      break;
    }

    if (since_start_us >= (long)STALL_GRACE_US &&
        distance_remaining > STALL_MIN_GAP_TICKS &&
        column_stall_detected(speed_ticks_s)) {
      stall_count++;
      if (stall_count >= STALL_CONFIRM_SAMPLES) {
        fprintf(stderr,
                "ERROR: Column motion stopped: encoder speed %.4f ticks/s is "
                "below %d or %d ticks/s (stall / end of travel).\n",
                speed_ticks_s, MIN_SPEED_TICKS_S_P, -MIN_SPEED_TICKS_S_N);
        printf("Column stall detected.\n");
        digitalWrite(raise_pin, 0);
        digitalWrite(lower_pin, 0);
        return -1;
      }
    } else {
      stall_count = 0;
    }
  }
  digitalWrite(raise_pin, 0);
  digitalWrite(lower_pin, 0);
  printf("Target: %d ticks\nActual: %d ticks\n", target_ticks, ticks);
  return (distance_remaining > TICK_TOLERANCE) ? -1 : 0;
}

#ifdef BUILD_RAISELOWERC_MAIN
int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);
  wiringPiSetupPinType(WPI_PIN_WPI);
  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);
  int rc = raiseLowerTo((int32_t)vals[0], COLUMN_RL_PIN2, COLUMN_RL_PIN1);
  return (rc < 0) ? 1 : 0;
}
#endif