#include "registers.hpp"
#include <iostream>
#include <iomanip>

Registers::Registers() {
    regFile.fill(0);  // Initialize all registers to 0
}

// Read a value from a register
int32_t Registers::read(uint8_t regNum) {
    if (regNum == 0) {
        return 0;  // x0 is always 0
    } else if (regNum < 32) {
        return regFile[regNum];
    } else {
        std::cerr << "Error: Invalid register number " << (int)regNum << std::endl;
        return 0;
    }
}

// Write a value to a register
void Registers::write(uint8_t regNum, int32_t value) {
    if (regNum != 0 && regNum < 32) {
        regFile[regNum] = value;
    } else if (regNum == 0) {
        // std::cerr << "Warning: Attempt to modify x0 ignored." << std::endl;
    } else {
        std::cerr << "Error: Invalid register number " << (int)regNum << std::endl;
    }
}

// Debug function to print the current state of all registers
void Registers::dump() {
    for (int i = 0; i < 32; i++) {
        std::cout << "x" << std::setw(2) << i << ": " << std::setw(8) << regFile[i];
        if ((i + 1) % 4 == 0) {
            std::cout << std::endl;
        } else {
            std::cout << "\t";
        }
    }
}

