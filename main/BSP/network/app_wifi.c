#include <stdlib.h>
#include <string.h>

#include "app_wifi.h"

#include "esp_check.h"
#include "esp_event_base.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "app_secrets.h"

#define WIFI_SSID APP_WIFI_SSID
#define WIFI_PASSWORD APP_WIFI_PASSWORD

static const char *TAG = "APP_WIFI";

static bool s_connected=false;
static bool s_initialized=false;

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (event_base==WIFI_EVENT)
    {
        /* code */
        if (event_id==WIFI_EVENT_STA_START) 
        {
            esp_wifi_connect();
        }
        else if (event_id==WIFI_EVENT_STA_DISCONNECTED) 
        {
            s_connected=false;

            ESP_LOGW(TAG, "WiFi disconnected");

            esp_err_t err=esp_wifi_connect();

            if (err!=ESP_OK)
            {
                /* code */
                ESP_LOGE(TAG, "Reconnect request failed:%s",esp_err_to_name(err));
            }
        }
    }
    else if (event_base==IP_EVENT &&
             event_id==IP_EVENT_STA_GOT_IP)
    {
        /* code */
        ip_event_got_ip_t *event=
        (ip_event_got_ip_t *)event_data;

        s_connected=true;

        ESP_LOGI(TAG, "WiFi Got IP: " IPSTR,
                 IP2STR(&event->ip_info.ip));
    }
}

bool app_wifi_is_connected(void)
{
    return s_connected;
}

esp_err_t app_wifi_init(void)
{
    if (s_initialized)
    {
        /* code */
        return ESP_OK;
    }

    esp_err_t err = nvs_flash_init();

    if (err!=ESP_OK)
    {
        /* code */
        ESP_LOGE(TAG, "NVS init failed: %s",
                 esp_err_to_name(err));
        return err;
    }

    // 2. TCP/IP
    ESP_RETURN_ON_ERROR(esp_netif_init(),
                        TAG, "Netif init failed");

    // 3. 默认事件循环
    ESP_RETURN_ON_ERROR(esp_event_loop_create_default(),
                        TAG, "Event loop failed");

    // 4. 创建 STA 接口
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    if (netif == NULL)
    {
        return ESP_FAIL;
    }

    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();

    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg),
                        TAG, "WiFi init failed");

    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL
        ),
    TAG,"WIFI event handler register failed");

    ESP_RETURN_ON_ERROR(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL),
        TAG, "IP event register failed");

    wifi_config_t wifi_config={0};

    strlcpy((char *)wifi_config.sta.ssid,
            WIFI_SSID,
            sizeof(wifi_config.sta.ssid));

    strlcpy((char *)wifi_config.sta.password,
            WIFI_PASSWORD,
            sizeof(wifi_config.sta.password));

    ESP_RETURN_ON_ERROR(
        esp_wifi_set_storage(WIFI_STORAGE_RAM),
        TAG, "WiFi storage failed");

    ESP_RETURN_ON_ERROR(
        esp_wifi_set_mode(WIFI_MODE_STA),
        TAG, "WiFi mode failed");

    ESP_RETURN_ON_ERROR(
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config),
        TAG, "WiFi config failed");

    ESP_RETURN_ON_ERROR(
        esp_wifi_start(),
        TAG, "WiFi start failed");

    s_initialized=true;

    ESP_LOGI(TAG, "WiFi STA initialized");
    return ESP_OK;
    
}