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

    I2C_Bus& bus = I2C_Bus::get_instance();

    bus.init();

    bus.write_register(DEVICE_ADDRESS, 0x6B, 0x00);

    printf("[MPU6050] Czujnik wybudzony.\n");
}

Acceleration MPU6050::readAcceleration()
{
    uint8_t data[6];

    I2C_Bus& bus = I2C_Bus::get_instance();

    bus.read_registers(DEVICE_ADDRESS, 0x3B, data, 6);

    Acceleration acceleration;

    acceleration.x = (int16_t)((data[0] << 8) | data[1]);
    acceleration.y = (int16_t)((data[2] << 8) | data[3]);
    acceleration.z = (int16_t)((data[4] << 8) | data[5]);

    // printf("[MPU6050] Accel X=%d, Y=%d, Z=%d\n",
    //        acceleration.x,
    //        acceleration.y,
    //        acceleration.z);

    return acceleration;
}

AccelerationG MPU6050::readAccelerationG()
{
    Acceleration raw = readAcceleration();

    AccelerationG accel_g;

    accel_g.x = raw.x / 16384.0f;
    accel_g.y = raw.y / 16384.0f;
    accel_g.z = raw.z / 16384.0f;

    // printf("[MPU6050] Accel[g] X=%.3f Y=%.3f Z=%.3f\n",
    //        accel_g.x,
    //        accel_g.y,
    //        accel_g.z);

    return accel_g;
}

float MPU6050::getAccelerationMagnitude()
{
    AccelerationG accel = readAccelerationG();

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

    uint8_t who_am_i = bus.read_register(DEVICE_ADDRESS, 0x75);

    printf("[MPU6050] WHO_AM_I = 0x%02X\n", who_am_i);

    return who_am_i;
}