#pragma once

#include <stdint.h>

// Struktura przechowująca surowe dane z akcelerometru.
// Są to wartości bezpośrednio odczytane z rejestrów MPU6050.
struct Acceleration
{
    int16_t x;
    int16_t y;
    int16_t z;
};

// Struktura przechowująca przyspieszenie w jednostkach g.
// Gdy czujnik leży nieruchomo na stole,
// jedna z osi będzie miała wartość około 1 g.
struct AccelerationG
{
    float x;
    float y;
    float z;
};

// Klasa odpowiedzialna za obsługę czujnika MPU6050.
class MPU6050
{
public:
    MPU6050();

    // Inicjalizuje czujnik i wybudza go z trybu uśpienia.
    void init();

    // Odczytuje surowe wartości przyspieszenia X, Y, Z.
    Acceleration readAcceleration();

    // Odczytuje przyspieszenie i przelicza je na jednostki g.
    AccelerationG readAccelerationG();

    // Odczytuje rejestr WHO_AM_I w celu sprawdzenia,
    // czy czujnik odpowiada poprawnie.
    uint8_t readWhoAmI();

    // Oblicza wartość wypadkową przyspieszenia:
    // sqrt(x^2 + y^2 + z^2).
    float getAccelerationMagnitude();

private:
    // Stały adres I2C czujnika MPU6050.
    // Dla typowego modułu MPU6050 wynosi 0x68.
    // constexpr oznacza, że wartość jest znana już podczas kompilacji.
    // static oznacza, że adres jest wspólny dla wszystkich obiektów klasy.
    static constexpr uint8_t DEVICE_ADDRESS = 0x68;
};