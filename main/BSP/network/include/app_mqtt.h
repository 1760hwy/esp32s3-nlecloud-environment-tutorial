#pragma once

#include "esp_err.h"

esp_err_t app_mqtt_init(void);

esp_err_t app_mqtt_publish_environment(
    float temperature,
    float humidity,
    float light
);