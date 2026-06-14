#pragma once

#include <stdint.h>

struct Acceleration
{
    int16_t x;
    int16_t y;
    int16_t z;
};

struct AccelerationG
{
    float x;
    float y;
    float z;
};

class MPU6050
{
public:
    MPU6050();

    void init();

    Acceleration readAcceleration();

    AccelerationG readAccelerationG();

    uint8_t readWhoAmI();

    float getAccelerationMagnitude();

private:
    static constexpr uint8_t DEVICE_ADDRESS = 0x68; //wartosc znana podczas kompilacji wartosc nie konkretnego obiektu tylko calej klasy klasy
};