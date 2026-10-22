/**
 * @file zh_bh1750.h
 *
 * @brief Driver for BH1750 digital ambient light sensor over I2C.
 *
 * This module provides an abstraction layer for the BH1750 ambient light
 * intensity sensor, which communicates via the I2C bus. The sensor converts
 * ambient light to a digital value with high resolution, providing a direct
 * lux reading without the need for external calibration.
 *
 * Key features:
 * - I2C interface with support for standard and fast modes
 * - Automatic lux calculation from raw sensor data
 * - Error statistics tracking for diagnostics
 * - Thread-safe initialization and deinitialization
 *
 * @note Supported I2C addresses: 0x23 (default), 0x5C (ADDR pin high).
 * @warning Ensure the I2C bus is initialized before calling zh_bh1750_init().
 */

#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ZH_BH1750_INIT_CONFIG_DEFAULT() \
    {                                   \
        .i2c_frequency = 400000,        \
        .i2c_address = 0x23}

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle representing a BH1750 sensor instance.
     *
     * This handle is allocated by zh_bh1750_init() and must be passed to
     * all subsequent API functions. It is freed by zh_bh1750_deinit().
     */
    typedef struct _zh_bh1750_handle_t zh_bh1750_handle_t;

    /**
     * @brief Configuration structure for BH1750 sensor initialization.
     *
     * This structure defines the I2C bus parameters required to communicate
     * with the BH1750 sensor. The I2C address defaults to 0x23 when the ADDR
     * pin is connected to GND, or 0x5C when the ADDR pin is connected to VCC.
     *
     * @note The I2C bus handle must be initialized before passing it here.
     * @warning I2C frequency must not exceed 400000 Hz (Fast Mode).
     */
    typedef struct
    {
        i2c_master_bus_handle_t i2c_handle; /*!< I2C bus handle for sensor communication */
        uint8_t i2c_address;                /*!< Sensor I2C address (0x23 or 0x5C) */
        uint32_t i2c_frequency;             /*!< I2C clock frequency in Hz (max 400000) */
    } zh_bh1750_init_config_t;

    /**
     * @brief Structure containing sensor error statistics.
     */
    typedef struct
    {
        uint32_t i2c_driver_error; /*!< Counter of I2C driver errors */
    } zh_bh1750_stats_t;

    /**
     * @brief Initialize the BH1750 sensor.
     *
     * Allocates a handle, validates the provided configuration, and registers
     * the sensor as an I2C device. The I2C bus must already be initialized.
     *
     * @param[in] config Pointer to the initialization configuration structure
     * @param[out] handle Pointer to receive the allocated sensor handle
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if config or handle is NULL, or parameters are invalid
     * @return ESP_ERR_NO_MEM if memory allocation for the handle fails
     * @return ESP_FAIL if I2C device registration fails
     * @return ESP_ERR_NOT_FOUND if the sensor does not respond on the specified address
     */
    esp_err_t zh_bh1750_init(const zh_bh1750_init_config_t *config, zh_bh1750_handle_t **handle);

    /**
     * @brief Deinitialize the BH1750 sensor.
     *
     * Removes the sensor from the I2C bus, frees the allocated handle,
     * and sets the handle pointer to NULL.
     *
     * @param handle Pointer to the sensor handle (must not be NULL)
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if handle or *handle is NULL
     *
     * @note After this call, the handle is invalid and must not be used.
     */
    esp_err_t zh_bh1750_deinit(zh_bh1750_handle_t **handle);

    /**
     * @brief Read ambient light intensity from the BH1750 sensor.
     *
     * Issues a one-time H-resolution measurement command, waits for the
     * conversion period (~180 ms), and reads the raw data. The raw value
     * is converted to lux using the formula: lux = raw_value / 1.2.
     *
     * @param[in] handle Pointer to the sensor handle (must not be NULL)
     * @param[out] data Pointer to receive the light intensity in lux (must not be NULL)
     *
     * @return ESP_OK on success
     * @return ESP_ERR_INVALID_ARG if handle or data is NULL
     * @return ESP_FAIL if any I2C transmission or reception fails
     *
     * @note This is a blocking call that delays for approximately 180 ms.
     * @warning Ensure sufficient stack space for the vTaskDelay call.
     */
    esp_err_t zh_bh1750_read(zh_bh1750_handle_t **handle, float *data);

    /**
     * @brief Get pointer to sensor error statistics.
     *
     * Returns read-only pointer to global statistics structure.
     *
     * @return Pointer to statistics structure (valid until reset)
     */
    const zh_bh1750_stats_t *zh_bh1750_get_stats(void);

    /**
     * @brief Reset all error statistics to zero.
     *
     * Clears all error counters tracked in the global statistics structure.
     */
    void zh_bh1750_reset_stats(void);

#ifdef __cplusplus
}
#endif
