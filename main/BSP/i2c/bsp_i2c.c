#include "bsp_i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdint.h>

static const char *TAG="BSP_I2C";

static i2c_master_bus_handle_t s_i2c_bus=NULL;

esp_err_t bsp_i2c_init(void)
{
    i2c_master_bus_config_t bus_config={
    .i2c_port=I2C_NUM_0,
    .sda_io_num=GPIO_NUM_8,
    .scl_io_num=GPIO_NUM_9,
    .clk_source=I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt=7,
    .flags.enable_internal_pullup=true,
    };

    esp_err_t err=i2c_new_master_bus(
        &bus_config,
        &s_i2c_bus
    );

    if (err!=ESP_OK)
    {
        /* code */
        ESP_LOGE(TAG,"i2c_new_master_bus failed:%s",esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG,"i2c_new_master_bus success");
    return ESP_OK;
}

esp_err_t bsp_i2c_probe(uint8_t address)
{
    if (s_i2c_bus==NULL)
    {
        /* code */
        ESP_LOGE(TAG,"i2c_bus is NULL");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = i2c_master_probe(
        s_i2c_bus,
        address,
        100
    );

    if (err==ESP_OK)
    {
        /* code */
        ESP_LOGI(TAG,"Device found at address 0x%02X",address);
    }
    else if (err==ESP_ERR_NOT_FOUND)
    {
        /* code */
        ESP_LOGW(TAG,"Device not found at address 0x%02X",address);
    }
    else
    {
        ESP_LOGE(TAG,"i2c_master_probe failed:%s",esp_err_to_name(err));
    }
    
    

    return err;

}

i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    return s_i2c_bus;
}