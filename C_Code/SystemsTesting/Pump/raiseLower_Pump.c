#include "../../functions.h"
#include "../../pins.h"
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>



/* HALL_CHANNEL (pins.h): FPGA ch. 7; bits 0–2 are Hall (1 = idle, 0 = detected). */
#define HALL_BIT HALL_CHANNEL /* 0–2: which sensor to watch */
#define RAISED FLUIDS_HALL_TOP   /* Hall active → bit 0 */
#define LOWERED FLUIDS_HALL_BOTTOM  /* idle → bit 1 */
#ifdef BUILD_RAISELOWERP_MAIN
static int sigint = 0;
static void intHandler(int dummy) { sigint = 1; }
#endif
int get_fpga_bit(int channel, int bit) {
  uint32_t data = fpga_safetran(channel);
  return (data >> bit) & 1;
}

void pump(int should_raise) { // 1 is up, 0 is lowered
  int active_pin = should_raise ? RAISE_PUMP_PIN : LOWER_PUMP_PIN;

  // The target state corresponds to the defined RAISED and LOWERED states
  // Change the defines at the top of the file if the bit mapping is inverted
  int target_state = should_raise ? RAISED : LOWERED;

  // If we're already at the target state, don't do anything to prevent grinding
  if (get_fpga_bit(HALL_CHANNEL, target_state) == 0) {
    return;
  }

  pinMode(RAISE_PUMP_PIN, OUTPUT);
  pinMode(LOWER_PUMP_PIN, OUTPUT);

  digitalWrite(RAISE_PUMP_PIN, 0);
  digitalWrite(LOWER_PUMP_PIN, 0);

  digitalWrite(active_pin, 1);

  while (get_fpga_bit(HALL_CHANNEL, target_state) != 0) {
    #ifdef BUILD_RAISELOWERP_MAIN
    if (sigint) {
      break;
    }
    #endif
    usleep(10000);
  }

  digitalWrite(RAISE_PUMP_PIN, 0);
  digitalWrite(LOWER_PUMP_PIN, 0);
}

#ifdef BUILD_RAISELOWERP_MAIN

int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);

  wiringPiSetupPinType(WPI_PIN_WPI);
  int vals[argc - 1];
  intparse(argc - 1, argv + 1, vals);
  pump(vals[0]);
  return 0;
}
#endif