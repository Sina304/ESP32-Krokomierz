#pragma once

#include <stdint.h>
#include <stddef.h>

// Klasa obsługująca magistralę I2C.
// Została zrobiona jako Singleton, ponieważ w projekcie
// chcemy mieć jedną wspólną magistralę I2C dla wszystkich urządzeń.
class I2C_Bus
{
private:
    // Prywatny konstruktor blokuje tworzenie obiektów poza klasą.
    // Dzięki temu nie da się przypadkowo utworzyć drugiej magistrali I2C.
    I2C_Bus();

    // Zablokowanie kopiowania obiektu.
    // Nie chcemy sytuacji, w której ktoś skopiuje magistralę I2C.
    I2C_Bus(const I2C_Bus&) = delete;
    I2C_Bus& operator=(const I2C_Bus&) = delete;

public:
    // Zwraca jedyną instancję klasy I2C_Bus.
    static I2C_Bus& get_instance();

    // Inicjalizuje magistralę I2C na pinach SDA=GPIO21 i SCL=GPIO22.
    void init();

    // Zapisuje jeden bajt danych do wybranego rejestru urządzenia I2C.
    void write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);

    // Odczytuje jeden bajt z wybranego rejestru urządzenia I2C.
    uint8_t read_register(uint8_t dev_addr, uint8_t reg_addr);

    // Odczytuje wiele kolejnych bajtów zaczynając od wskazanego rejestru.
    void read_registers(
        uint8_t dev_addr,   // adres urządzenia I2C
        uint8_t start_reg,  // pierwszy odczytywany rejestr
        uint8_t* buffer,    // bufor na odebrane dane
        size_t length       // liczba bajtów do odczytu
    );
};