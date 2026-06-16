#include "mpu6050.h"
#include "i2c_bus.h"

#include <stdio.h>
#include <math.h>

MPU6050::MPU6050()
{
}

void MPU6050::init()
{
    printf("[MPU6050] Inicjalizacja czujnika...\n");

    // Pobranie wspólnej instancji magistrali I2C.
    I2C_Bus& bus = I2C_Bus::get_instance();

    // Uruchomienie magistrali I2C.
    bus.init();

    // Rejestr 0x6B odpowiada za zarządzanie zasilaniem.
    // Wpisanie 0x00 wybudza MPU6050 z trybu uśpienia.
    bus.write_register(DEVICE_ADDRESS, 0x6B, 0x00);

    printf("[MPU6050] Czujnik wybudzony.\n");
}

Acceleration MPU6050::readAcceleration()
{
    // MPU6050 zwraca 6 bajtów danych:
    // X_H, X_L, Y_H, Y_L, Z_H, Z_L.
    uint8_t data[6];

    I2C_Bus& bus = I2C_Bus::get_instance();

    // Rejestr 0x3B to początek danych akcelerometru.
    // Czytamy 6 kolejnych bajtów dla osi X, Y, Z.
    bus.read_registers(DEVICE_ADDRESS, 0x3B, data, 6);

    Acceleration acceleration;

    // Każda oś ma 16 bitów zapisanych w dwóch bajtach:
    // starszy bajt i młodszy bajt.
    acceleration.x = (int16_t)((data[0] << 8) | data[1]);
    acceleration.y = (int16_t)((data[2] << 8) | data[3]);
    acceleration.z = (int16_t)((data[4] << 8) | data[5]);

    return acceleration;
}

AccelerationG MPU6050::readAccelerationG()
{
    // Najpierw odczytujemy surowe dane z czujnika.
    Acceleration raw = readAcceleration();

    AccelerationG accel_g;

    // Dla domyślnego zakresu +/-2g wartość 16384 odpowiada 1g.
    accel_g.x = raw.x / 16384.0f;
    accel_g.y = raw.y / 16384.0f;
    accel_g.z = raw.z / 16384.0f;

    return accel_g;
}

float MPU6050::getAccelerationMagnitude()
{
    // Odczyt przyspieszenia w jednostkach g.
    AccelerationG accel = readAccelerationG();

    // Obliczenie wartości wypadkowej przyspieszenia.
    // Dzięki temu algorytm działa niezależnie od orientacji czujnika.
    float magnitude = sqrt(
        accel.x * accel.x +
        accel.y * accel.y +
        accel.z * accel.z
    );

    printf("%.3f\n", magnitude);

    return magnitude;
}

uint8_t MPU6050::readWhoAmI()
{
    I2C_Bus& bus = I2C_Bus::get_instance();

    // Rejestr 0x75 to WHO_AM_I.
    // Dla MPU6050 powinien zwrócić 0x68.
    uint8_t who_am_i = bus.read_register(DEVICE_ADDRESS, 0x75);

    printf("[MPU6050] WHO_AM_I = 0x%02X\n", who_am_i);

    return who_am_i;
}