#include <stdio.h>
#include "sdp_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"
#include <inttypes.h>

int main(void) {
    int16_t error = 0;

    sensirion_i2c_hal_init();

    sdp_stop_continuous_measurement();
    // ignore error if no measurement running

    uint32_t product_number;
    const uint8_t serial_number_size = 8;
    uint8_t serial_number_raw[serial_number_size];

    error = sdp_prepare_product_identifier();
    if (error) {
        printf("Error executing sdp_prepare_product_identifier(): %i\n", error);
    }

    error = sdp_read_product_identifier(&product_number, serial_number_raw,
                                        serial_number_size);

    if (error) {
        printf("Error executing sdp_read_product_identifier(): %i\n", error);
    } else {
        printf("Product number: 0x%08x\n", product_number);
        uint64_t serial_number = (uint64_t)serial_number_raw[0] << 56 |
                                 (uint64_t)serial_number_raw[1] << 48 |
                                 (uint64_t)serial_number_raw[2] << 40 |
                                 (uint64_t)serial_number_raw[3] << 32 |
                                 (uint64_t)serial_number_raw[4] << 24 |
                                 (uint64_t)serial_number_raw[5] << 16 |
                                 (uint64_t)serial_number_raw[6] << 8 |
                                 (uint64_t)serial_number_raw[7];
        printf("Serial Number: %" PRIu64 "\n", serial_number);
    }

    // Start Measurement

    error =
        sdp_start_continuous_measurement_with_diff_pressure_t_comp_and_averaging();
    if (error) {
        printf("Error executing "
               "sdp_start_continuous_measurement_with_diff_pressure_t_comp_"
               "and_averaging(): %i\n",
               error);
    }

    for (int i = 0; i < 10000; i++) {
        // Read Measurement every 10ms
        sensirion_i2c_hal_sleep_usec(100000);

        float differential_pressure;
        float temperature;

        error = sdp_read_measurement(&differential_pressure, &temperature);

        if (error) {
            printf("Error executing sdp_read_measurement(): %i\n", error);
            break;
        } else {
            printf("\033[1A\033[2K\r\033[1A\033[2K\r");
            printf("Differential pressure: %0.2f Pa\n", differential_pressure);
            printf("Temperature: %0.2f °C\n", temperature);
        }
    }

    error = sdp_stop_continuous_measurement();
    if (error) {
        printf("Error executing sdp_stop_continuous_measurement(): %i\n",
               error);
    }

    return 0;
}