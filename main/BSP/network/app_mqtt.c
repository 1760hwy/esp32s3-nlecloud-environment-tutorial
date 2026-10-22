#include "app_mqtt.h"
#include "esp_err.h"
#include "esp_event_base.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_event.h"
#include <stdio.h>
#include <stdbool.h>

#include "app_secrets.h"

#define MQTT_PUB_TOPIC \
    "/sys/" APP_MQTT_DEVICE_ID "/" APP_MQTT_CLIENT_ID "/sensor/datas"

static const char *TAG="APP_MQTT";
static bool s_mqtt_connected=false;



static esp_mqtt_client_handle_t s_mqtt_client=NULL;

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data
)
{
    (void)handler_args;
    (void)base;
    (void)event_data;

    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        /* code */
        s_mqtt_connected=true;
        ESP_LOGI(TAG, "MQTT connected");
        break;
    case MQTT_EVENT_DISCONNECTED:
        /* code */
        s_mqtt_connected=false;
        ESP_LOGI(TAG, "MQTT disconnected");
        break;
    case MQTT_EVENT_ERROR:
        /* code */
        ESP_LOGI(TAG, "MQTT error");
        break;
    
    default:
        break;
    }
}

esp_err_t app_mqtt_init(void)
{
    if (s_mqtt_client!=NULL)
    {
        /* code */
        return ESP_ERR_INVALID_STATE;
    }

    esp_mqtt_client_config_t mqtt_cfg={
        .broker.address.uri=APP_MQTT_URI,
        .credentials.client_id=APP_MQTT_CLIENT_ID,
        .credentials.username=APP_MQTT_DEVICE_ID,
        .credentials.authentication.password=APP_MQTT_SECRET_KEY
    };

    s_mqtt_client=esp_mqtt_client_init(&mqtt_cfg);

    if (s_mqtt_client==NULL)
    {
        /* code */
        ESP_LOGE(TAG, "MQTT client init failed");
        return ESP_FAIL;
    }

    esp_err_t err=esp_mqtt_client_register_event(
        s_mqtt_client, 
        ESP_EVENT_ANY_ID,
        mqtt_event_handler, 
        NULL
    );

    if (err==ESP_OK)
    {
        /* code */
        err=esp_mqtt_client_start(s_mqtt_client);
    }

    if (err!=ESP_OK)
    {
        /* code */
        ESP_LOGE(TAG, "MQTT client start failed:%s",esp_err_to_name(err));

        esp_mqtt_client_destroy(s_mqtt_client);

        s_mqtt_client=NULL;

        return err;
    }
    
    

    return ESP_OK;
}


esp_err_t app_mqtt_publish_environment(
    float temperature,
    float humidity,
    float light
)
{
    if (s_mqtt_client==NULL || !s_mqtt_connected)
    {
        /* code */
        return ESP_ERR_INVALID_STATE;
    }

    char payload[192];

    int len = snprintf(
        payload,
        sizeof(payload),
        "{\"datatype\":1,\"datas\":{"
        "\"Temperature\":%.2f,"
        "\"Humidity\":%.2f,"
        "\"Light\":%.2f"
        "}}",
        temperature,
        humidity,
        light
    );

    if (len < 0 || len >= (int)sizeof(payload))
    {
        return ESP_ERR_INVALID_SIZE;
    }

    int msg_id=esp_mqtt_client_publish(
        s_mqtt_client,
        MQTT_PUB_TOPIC,
        payload,
        len,
        1,
        0
    );

    if (msg_id < 0)
    {
        ESP_LOGE(TAG, "MQTT publish failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "MQTT published, msg_id=%d",
             msg_id);

    return ESP_OK;
    
}
