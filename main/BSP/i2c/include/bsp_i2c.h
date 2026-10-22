#pragma once

#include "esp_err.h"
#include <stdint.h>
#include "driver/i2c_master.h"

esp_err_t bsp_i2c_init(void);
esp_err_t bsp_i2c_probe(uint8_t address);

i2c_master_bus_handle_t bsp_i2c_get_handle(void);

