#include <stdio.h>
#include "bsp_i2c.h"
#include "ath20.h"
#include "esp_event_base.h"
#include "zh_bh1750.h"
#include "ssd1306.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "app_wifi.h"
#include "app_mqtt.h"
#include "mqtt_client.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_ERROR_CHECK(app_wifi_init());

    ESP_ERROR_CHECK(bsp_i2c_init());

    ESP_ERROR_CHECK(app_mqtt_init());

    ssd1306_handle_t oled = NULL;

    ssd1306_config_t oled_config =
        I2C_SSD1306_128x64_CONFIG_DEFAULT;

    oled_config.i2c_address = 0x3C;

    esp_err_t oled_err = ssd1306_init(
        bsp_i2c_get_handle(),
        &oled_config,
        &oled
    );

    if (oled_err == ESP_OK)
    {
        oled_err = ssd1306_clear_display(oled, false);
    }

    if (oled_err == ESP_OK)
    {
        oled_err = ssd1306_display_text(
            oled,
            0,
            "Hello ESP32-S3",
            false
        );
    }

    zh_bh1750_handle_t *bh1750=NULL;

    zh_bh1750_init_config_t config=ZH_BH1750_INIT_CONFIG_DEFAULT();

    config.i2c_handle=bsp_i2c_get_handle();
    config.i2c_frequency=100000;

    ESP_ERROR_CHECK(
        zh_bh1750_init(&config,&bh1750)
    );

    AHT20_data_t aht20_data={0};

    ESP_ERROR_CHECK(AHT20_begin(&aht20_data));

    while (1)
    {
        /* code */
        esp_err_t err=AHT20_measure(&aht20_data);

        char oled_text[17];

        float light_lux=0.0f;

        if (err==ESP_OK)
        {
            /* code */
            ESP_LOGI(TAG, "Temperature: %.2f C",
                     aht20_data.temperature);

            ESP_LOGI(TAG, "Humidity: %.2f %%",
                     aht20_data.r_humidity);

            snprintf(oled_text, sizeof(oled_text),
            "TEMP: %.2f C",
            aht20_data.temperature);

            ssd1306_display_text(oled,2,oled_text,false);

            snprintf(oled_text, sizeof(oled_text),
            "HUMI: %.2f %%",
            aht20_data.r_humidity);

            ssd1306_display_text(oled,4,oled_text,false);
        }
        else
        {
            ESP_LOGE(TAG, "AHT20 read failed: %s",
                     esp_err_to_name(err));
        }

        esp_err_t err1=zh_bh1750_read(
            &bh1750,
            &light_lux
        );

        if (err1 == ESP_OK)
        {
            
            ESP_LOGI("BH1750", "Light: %.2f lux", light_lux);
            snprintf(oled_text, sizeof(oled_text),
            "LIGHT: %.0f lux",
            light_lux);

            ssd1306_display_text(oled, 6, oled_text, false);

        }
        else
        {
            ESP_LOGE("BH1750", "Read failed: %s",
                    esp_err_to_name(err1));
        }


        if (err==ESP_OK && err1==ESP_OK)
        {
            /* code */
            esp_err_t mqtt_err=app_mqtt_publish_environment(
                aht20_data.temperature,
                aht20_data.r_humidity,
                light_lux
            );

            if (mqtt_err != ESP_OK)
            {
                ESP_LOGW(TAG, "MQTT upload skipped/failed: %s",
                        esp_err_to_name(mqtt_err));
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}