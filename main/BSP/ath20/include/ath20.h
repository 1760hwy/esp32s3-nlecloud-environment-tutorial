#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"

/// @brief Storage for the last successful measurement and latest read busy flag.
typedef struct AHT20
{
    float r_humidity;
    float temperature;
    uint8_t is_busy;
} AHT20_data_t;

/* Single-device driver. Call from one task, or serialize begin/measure externally.
 * The BSP owns the bus and must initialize it before AHT20_begin().
 * Keep that bus alive for the driver's lifetime. Repeated begin reuses the device.
 */
esp_err_t AHT20_begin(AHT20_data_t *pAHT20_data);

/* Call after a successful begin. On error, humidity and temperature are unchanged.
 * Returns the I2C error, ESP_ERR_TIMEOUT, ESP_ERR_INVALID_CRC,
 * ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_RESPONSE or ESP_ERR_INVALID_ARG.
 */
esp_err_t AHT20_measure(AHT20_data_t *pAHT20_data);
