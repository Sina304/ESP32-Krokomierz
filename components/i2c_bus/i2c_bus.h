#pragma once // Zabezpieczenie przed wielokrotnym wklejeniem tego pliku (gdyby dwa różne moduły zrobiły #include "i2c_bus.h")
#include <stdint.h>

class I2C_Bus {
private:
    // 1. PRYWATNY KONSTRUKTOR
    // Skoro konstruktor jest prywatny, to nie da się tak po prostu stworzyć obiektu w mainie
    I2C_Bus();

    // 2. USUNIĘCIE KOPIOWANIA
    // C++ domyślnie pozwala na kopiowanie obiektów (I2C_Bus kopia = oryginal). 
    // Słówko "= delete" zabrania tego. Zabezpiecza przed przypadkowym sklonowaniem naszego Singletona.
    I2C_Bus(const I2C_Bus&) = delete;
    I2C_Bus& operator=(const I2C_Bus&) = delete;

public:
    // 3. JEDYNA BRAMA WEJŚCIOWA
    // Skoro nie można użyć konstruktora, trzeba statycznej metody (niezależnej od obiektu), 
    // która "wypożyczy" nam tę jedyną, poprawną instancję magistrali.
    static I2C_Bus& get_instance();

    // 4. ZWYKŁE METODY
    // Funkcje, które będą wykonywać fizyczną pracę na sprzęcie.
    void init();
    void write_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
    uint8_t read_register(uint8_t dev_addr, uint8_t reg_addr);
    void read_registers(
    uint8_t dev_addr,
    uint8_t start_reg,
    uint8_t* buffer,
    size_t length
    );
};