#include "i2c_bus.h"

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#include <stdio.h>

// Numer kontrolera I2C w ESP32.
// ESP32 ma więcej niż jeden kontroler I2C, tutaj używamy I2C_NUM_0.
#define I2C_MASTER_PORT I2C_NUM_0

// Piny ESP32 używane jako linie I2C.
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_SCL_IO GPIO_NUM_22

// Prędkość transmisji I2C: 100 kHz, czyli standard mode.
#define I2C_MASTER_FREQ_HZ 100000

I2C_Bus::I2C_Bus()
{
}

I2C_Bus& I2C_Bus::get_instance()
{
    // Obiekt static zostanie utworzony tylko raz,
    // przy pierwszym wywołaniu get_instance().
    static I2C_Bus instance;

    return instance;
}

void I2C_Bus::init()
{
    // Struktura konfiguracyjna drivera I2C.
    // Nawiasy {} zerują wszystkie pola.
    i2c_config_t config = {};

    // ESP32 pracuje jako master, czyli urządzenie sterujące magistralą.
    config.mode = I2C_MODE_MASTER;

    // Wybór pinów SDA i SCL.
    config.sda_io_num = I2C_MASTER_SDA_IO;
    config.scl_io_num = I2C_MASTER_SCL_IO;

    // Włączenie wewnętrznych rezystorów podciągających.
    config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    config.scl_pullup_en = GPIO_PULLUP_ENABLE;

    // Ustawienie częstotliwości zegara I2C.
    config.master.clk_speed = I2C_MASTER_FREQ_HZ;

    // Przekazanie konfiguracji do ESP-IDF.
    esp_err_t config_result = i2c_param_config(I2C_MASTER_PORT, &config);

    if (config_result != ESP_OK)
    {
        printf("[I2C_Bus] Blad konfiguracji I2C: %s\n", esp_err_to_name(config_result));
        return;
    }

    // Instalacja drivera I2C.
    esp_err_t driver_result = i2c_driver_install(
        I2C_MASTER_PORT, // numer kontrolera I2C
        config.mode,     // master/slave
        0,               // RX buffer (nieużywany w master)
        0,               // TX buffer (nieużywany w master)
        0                // flagi przerwań
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
    // Do zapisu wysyłamy dwa bajty:
    // 1. adres rejestru,
    // 2. wartość, którą chcemy do niego wpisać.
    uint8_t write_buffer[2] = {reg_addr, data};

    esp_err_t result = i2c_master_write_to_device(
        I2C_MASTER_PORT,          // kontroler I2C używany przez ESP32
        dev_addr,                 // adres urządzenia I2C (np. MPU6050 = 0x68)
        write_buffer,             // dane do wysłania (rejestr + wartość)
        sizeof(write_buffer),     // liczba wysyłanych bajtów
        1000 / portTICK_PERIOD_MS // maksymalny czas oczekiwania
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

    // Najpierw wysyłamy adres rejestru, który chcemy odczytać,
    // a potem odczytujemy z niego jeden bajt.
    esp_err_t result = i2c_master_write_read_device(
        I2C_MASTER_PORT,          // kontroler I2C
        dev_addr,                 // adres urządzenia
        &reg_addr,                // adres rejestru
        1,                        // liczba bajtów wysyłanych
        &data,                    // bufor odbiorczy
        1,                        // liczba bajtów do odczytu
        1000 / portTICK_PERIOD_MS // timeout
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
    /* Najpierw wysyłamy adres pierwszego rejestru,
     od którego chcemy rozpocząć odczyt,
     a następnie odbieramy kilka kolejnych bajtów.
     Przykład użycia:

    MPU6050:
    start_reg = 0x3B
    length = 6

    Odczytane zostaną:
    0x3B -> Accel_X_H
    0x3C -> Accel_X_L
    0x3D -> Accel_Y_H
    0x3E -> Accel_Y_L
    0x3F -> Accel_Z_H
    0x40 -> Accel_Z_L
    */
    esp_err_t result = i2c_master_write_read_device(
        I2C_MASTER_PORT,          // kontroler I2C
        dev_addr,                 // adres urządzenia
        &start_reg,               // adres pierwszego rejestru
        1,                        // wysyłamy 1 bajt (adres rejestru)
        buffer,                   // bufor na odebrane dane
        length,                   // liczba bajtów do odczytu
        1000 / portTICK_PERIOD_MS // timeout
    );

    if (result != ESP_OK)
    {
        printf("[I2C_Bus] Blad odczytu wielu bajtow: %s\n",
               esp_err_to_name(result));
    }
}