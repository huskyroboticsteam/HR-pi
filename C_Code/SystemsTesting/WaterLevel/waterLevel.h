#ifndef WATERLEVEL_H
#define WATERLEVEL_H

#include "waterLevelADS1015.h"

#define STABLE_DELTA_THRESHOLD  7.0f
#define STABLE_CONFIRM_CYCLES   3

float pct_to_mm(float pct);

float waterLevel_read_mm(int fd, int channel, int num_samples, float cal_max_adc);

float waterLevel_read_stable_mm(int fd, int channel, int num_samples,
                                int interval_ms, float cal_max_adc);

#endif
