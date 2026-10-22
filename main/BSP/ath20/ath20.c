#include "ath20.h"
#include "bsp_i2c.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define AHT20_DEVICE_ADDR 0x38
#define AHT20_CMD_MEASUREMENT 0xAC
#define AHT20_CMD_READ_STATUS 0x71
#define AHT20_CMD_INIT 0xBE
#define AHT20_CMD_RST 0xBA

#define AHT20_DELAY_MEASUREMENT 80
#define AHT20_DELAY_INIT 10
#define AHT20_TIMEOUT_MS 100
#define AHT20_POLL_DELAY_MS 20
#define AHT20_POLL_ATTEMPTS 10
#define GET_BIT(byte, index) (((byte) >> (index)) & 1U)

//////////////////////////////////////////////// GLOBAL_VARIABLE ////////////////////////////////////////////////

static const char *TAG = "AHT20";
static i2c_master_dev_handle_t s_aht20_device;
static bool s_initialized;
static uint8_t data[7];
static uint8_t mode;
static uint8_t cal;

//////////////////////////////////////////////// PRIVATE_DECLARATION ////////////////////////////////////////////////

/* Round up so every wait lasts at least the requested sensor settling time. */
static void m_AHT20_delay(uint32_t milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds + portTICK_PERIOD_MS - 1));
}

static esp_err_t m_AHT20_command_init(void)
{
    const uint8_t command[] = {AHT20_CMD_INIT, 0x08, 0x00};
    return i2c_master_transmit(
        s_aht20_device, command, sizeof(command), AHT20_TIMEOUT_MS);
}

static esp_err_t m_AHT20_command_reset(void)
{
    const uint8_t command = AHT20_CMD_RST;
    return i2c_master_transmit(
        s_aht20_device, &command, sizeof(command), AHT20_TIMEOUT_MS);
}

static esp_err_t m_AHT20_command_read_status(void)
{
    const uint8_t command = AHT20_CMD_READ_STATUS;
    return i2c_master_transmit(
        s_aht20_device, &command, sizeof(command), AHT20_TIMEOUT_MS);
}

static esp_err_t m_AHT20_command_measure(void)
{
    const uint8_t command[] = {AHT20_CMD_MEASUREMENT, 0x33, 0x00};
    return i2c_master_transmit(
        s_aht20_device, command, sizeof(command), AHT20_TIMEOUT_MS);
}

static esp_err_t m_AHT20_decode_status(AHT20_data_t *pAHT20_data)
{
    pAHT20_data->is_busy = GET_BIT(data[0], 7);
    cal = GET_BIT(data[0], 3);

    if (GET_BIT(data[0], 6))
    {
        mode = 2;
    }
    else if (GET_BIT(data[0], 5))
    {
        mode = 1;
    }
    else
    {
        mode = 0;
    }

    ESP_LOGD(TAG, "STATUS: BUSY=%u; CAL=%u; MODE=%u; HEX=0x%02x",
             pAHT20_data->is_busy, cal, mode, data[0]);
    return ESP_OK;
}

static esp_err_t m_AHT20_get_result(AHT20_data_t *pAHT20_data, size_t len)
{
    if (pAHT20_data == NULL || len == 0 || len > sizeof(data))
    {
        return ESP_ERR_INVALID_ARG;
    }

    return i2c_master_receive(s_aht20_device, data, len, AHT20_TIMEOUT_MS);
}

static esp_err_t m_AHT20_read_status(AHT20_data_t *pAHT20_data)
{
    ESP_RETURN_ON_ERROR(m_AHT20_command_read_status(), TAG, "Status command failed");
    ESP_RETURN_ON_ERROR(m_AHT20_get_result(pAHT20_data, 1), TAG, "Status read failed");
    return m_AHT20_decode_status(pAHT20_data);
}

/* CRC-8 over status and both 20-bit values: initial 0xff, polynomial 0x31. */
static uint8_t m_AHT20_crc(void)
{
    uint8_t crc = 0xFF;

    for (size_t i = 0; i < sizeof(data) - 1; ++i)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31)
                               : (uint8_t)(crc << 1);
        }
    }

    return crc;
}

//////////////////////////////////////////////// PUBLIC_DECLARATION ////////////////////////////////////////////////

esp_err_t AHT20_measure(AHT20_data_t *pAHT20_data)
{
    if (pAHT20_data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_initialized)
    {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_RETURN_ON_ERROR(m_AHT20_command_measure(), TAG, "Measurement command failed");
    m_AHT20_delay(AHT20_DELAY_MEASUREMENT);

    for (unsigned attempt = 0; attempt < AHT20_POLL_ATTEMPTS; ++attempt)
    {
        ESP_RETURN_ON_ERROR(m_AHT20_get_result(pAHT20_data, sizeof(data)),
                            TAG, "Measurement read failed");
        m_AHT20_decode_status(pAHT20_data);
        if (!pAHT20_data->is_busy)
        {
            break;
        }
        if (attempt + 1 == AHT20_POLL_ATTEMPTS)
        {
            return ESP_ERR_TIMEOUT;
        }
        m_AHT20_delay(AHT20_POLL_DELAY_MS);
    }

    if (m_AHT20_crc() != data[6])
    {
        return ESP_ERR_INVALID_CRC;
    }
    if (!cal)
    {
        s_initialized = false;
        return ESP_ERR_INVALID_STATE;
    }

    const uint32_t rh_raw = ((uint32_t)data[1] << 12)
                            | ((uint32_t)data[2] << 4)
                            | ((uint32_t)data[3] >> 4);
    const uint32_t temp_raw = ((uint32_t)(data[3] & 0x0F) << 16)
                              | ((uint32_t)data[4] << 8)
                              | (uint32_t)data[5];
    const float rh = (float)rh_raw * (100.0f / 1048576.0f);
    const float temp = (float)temp_raw * (200.0f / 1048576.0f) - 50.0f;

    if (rh > 100.0f || rh < 0.0f || temp > 85.0f || temp < -40.0f)
    {
        ESP_LOGW(TAG, "Measurement out of range: RH=%f, T=%f", rh, temp);
        return ESP_ERR_INVALID_RESPONSE;
    }

    /* Publish only a complete, validated sample. Preserve the last sample on error. */
    pAHT20_data->r_humidity = rh;
    pAHT20_data->temperature = temp;
    ESP_LOGD(TAG, "Temperature: %f, Relative Humidity: %f", temp, rh);
    return ESP_OK;
}

esp_err_t AHT20_begin(AHT20_data_t *pAHT20_data)
{
    if (pAHT20_data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    if (bus == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }

    s_initialized = false;
    cal = 0;
    if (s_aht20_device == NULL)
    {
        const i2c_device_config_t device_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = AHT20_DEVICE_ADDR,
            .scl_speed_hz = 100000,
        };
        ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus, &device_config, &s_aht20_device),
                            TAG, "Adding AHT20 device failed");
    }

    m_AHT20_delay(40);
    ESP_RETURN_ON_ERROR(m_AHT20_command_reset(), TAG, "Reset command failed");
    m_AHT20_delay(40);
    ESP_RETURN_ON_ERROR(m_AHT20_read_status(pAHT20_data), TAG, "Initial status failed");
    if (cal && !pAHT20_data->is_busy)
    {
        s_initialized = true;
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(m_AHT20_command_init(), TAG, "Calibration command failed");
    m_AHT20_delay(AHT20_DELAY_INIT);
    for (unsigned attempt = 0; attempt < AHT20_POLL_ATTEMPTS; ++attempt)
    {
        m_AHT20_delay(AHT20_POLL_DELAY_MS);
        ESP_RETURN_ON_ERROR(m_AHT20_read_status(pAHT20_data), TAG, "Calibration status failed");
        if (cal && !pAHT20_data->is_busy)
        {
            s_initialized = true;
            return ESP_OK;
        }
    }

    return ESP_ERR_TIMEOUT;
}
