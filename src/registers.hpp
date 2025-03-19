#ifndef REGISTERS_HPP
#define REGISTERS_HPP

#include <cstdint>
#include <array>
#include <string>

class Registers {
private:
    std::array<int32_t, 32> regFile;  // Array to store 32 registers

public:
    Registers();                      // Constructor to initialize registers
    int32_t read(uint8_t regNum);     // Read from a register
    void write(uint8_t regNum, int32_t value); // Write to a register
    void dump();                      // Debug function to print all registers
};

#endif // REGISTERS_HPP
