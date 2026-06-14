#include "i2c_bus.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#include <stdio.h>

#define I2C_MASTER_PORT I2C_NUM_0
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_FREQ_HZ 100000

I2C_Bus::I2C_Bus()
{
}

I2C_Bus& I2C_Bus::get_instance()
{
    static I2C_Bus instance;
    return instance;
}

void I2C_Bus::init()
{
    i2c_config_t config = {};

    config.mode = I2C_MODE_MASTER;
    config.sda_io_num = I2C_MASTER_SDA_IO;
    config.scl_io_num = I2C_MASTER_SCL_IO;
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    config.master.clk_speed = I2C_MASTER_FREQ_HZ;

    esp_err_t config_result = i2c_param_config(I2C_MASTER_PORT, &config);

    if (config_result != ESP_OK)
    {
        printf("[I2C_Bus] Blad konfiguracji I2C: %s\n", esp_err_to_name(config_result));
        return;
    }

    esp_err_t driver_result = i2c_driver_install(
        I2C_MASTER_PORT,
        config.mode,
        0,
        0,
        0
    );

    if (driver_result != ESP_OK)
    {
        printf("[I2C_Bus] Blad instalacji drivera I2C: %s\n", esp_err_to_name(driver_result));
        return;
    }

    printf("[I2C_Bus] Magistrala I2C uruchomiona na SDA=GPIO21, SCL=GPIO22.\n");
}

void I2C_Bus::write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buffer[2] = {reg_addr, data};

    esp_err_t result = i2c_master_write_to_device(
        I2C_MASTER_PORT,
        dev_addr,
        write_buffer,
        sizeof(write_buffer),
        1000 / portTICK_PERIOD_MS
    );

    if (result != ESP_OK)
    {
        printf("[I2C_Bus] Blad zapisu I2C: %s\n", esp_err_to_name(result));
        return;
    }

    printf("[I2C_Bus] Zapis OK: device=0x%02X, reg=0x%02X, data=0x%02X\n",
           dev_addr, reg_addr, data);
}

uint8_t I2C_Bus::read_register(uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t data = 0;

    esp_err_t result = i2c_master_write_read_device(
        I2C_MASTER_PORT,
        dev_addr,
        &reg_addr,
        1,
        &data,
        1,
        1000 / portTICK_PERIOD_MS
    );

    if (result != ESP_OK)
    {
        printf("[I2C_Bus] Blad odczytu I2C: %s\n", esp_err_to_name(result));
        return 0;
    }

    printf("[I2C_Bus] Odczyt OK: device=0x%02X, reg=0x%02X, data=0x%02X\n",
           dev_addr, reg_addr, data);

    return data;
}

void I2C_Bus::read_registers(
    uint8_t dev_addr,
    uint8_t start_reg,
    uint8_t* buffer,
    size_t length)
{
    esp_err_t result = i2c_master_write_read_device(
        I2C_MASTER_PORT,
        dev_addr,
        &start_reg,
        1,
        buffer,
        length,
        1000 / portTICK_PERIOD_MS
    );

    if (result != ESP_OK)
    {
        printf("[I2C_Bus] Blad odczytu wielu bajtow: %s\n",
               esp_err_to_name(result));
    }
}