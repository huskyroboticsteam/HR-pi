//
// Time of Flight sensor test program
//
// Copyright (c) 2017 Larry Bank
// email: bitbank@pobox.com
// Project started 7/29/2017
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tof.h" // time of flight sensor library

#define ZERO_NUM_SAMPLES 25

static int init (void) {
	int i;
	int model;
	int revision;
	i = tofInit(1, 0x29, 0);
	if (i != 1) {
		return -1;
	}

	printf("VL53L0X device successfully opened.\n");
	i = tofGetModel(&model, &revision);
	if (i != 1) {
		return -1;
	}
	printf("Model ID - %d\n", model);
	printf("Revision ID - %d\n", revision);

	setMeasurementTimingBudget(200000);
	printf("High Accuracy Enabled (200ms) \n");

	return i;
}

static int run_tof_test(int zero) {
	int iDistance;
	while (1) // read values 20 times a second for 1 minute
	{
		iDistance = zero - tofReadDistance();
		if (iDistance < 4096) {
			printf("\033[1A\033[2K\r");
			printf("Distance = %dmm\n", iDistance);
		}
		usleep(200000);
	}

	return 0;
}

int zeroCalibration() {
    long sum = 0;
	int samples = 0;
    for (int i = 0; i < ZERO_NUM_SAMPLES; i++) {
        int reading = tofReadDistance();
        if (reading < 0 || reading > 1000) {
            fprintf(stderr, "Read error during calibration\n");
        } else {
			printf("Calibrating: %d \n", reading);
        	sum += reading;
			samples++;
		}
        usleep(200000); // 200ms between samples
    }
    int measured = sum / samples;
    printf("Calibration: measured=%dmm\n", measured);
    return measured;
}

#ifdef BUILD_TOFTEST_MAIN
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	init();
	usleep(500000);
	int zero = zeroCalibration();
	printf("Calibrated\n");
	return run_tof_test(zero);
}
#endif