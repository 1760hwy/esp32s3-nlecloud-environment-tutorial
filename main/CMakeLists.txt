if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/app_secrets.h")
    message(FATAL_ERROR
        "Copy main/app_secrets.example.h to main/app_secrets.h and fill your own credentials. See README.md.")
endif()

idf_component_register(
    SRCS
        "main.c"
        "BSP/i2c/bsp_i2c.c"
        "BSP/ath20/ath20.c"
        "BSP/ss1306/ssd1306.c"
        "BSP/bh1750/zh_bh1750.c"
        "BSP/network/app_wifi.c"
        "BSP/network/app_mqtt.c"

    INCLUDE_DIRS
        "."
        "BSP/i2c/include"
        "BSP/ath20/include"
        "BSP/ss1306/include"
        "BSP/bh1750/include"
        "BSP/network/include"

    PRIV_REQUIRES
        esp_driver_i2c
        esp_type_utils
        esp_wifi
        esp_netif
        esp_event
        nvs_flash

    REQUIRES driver
)