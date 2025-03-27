#ifndef CONTROL_HPP
#define CONTROL_HPP

#include <cstdint>

class Control {
public:
    bool regWrite;    // Write to register
    bool memRead;     // Read from memory
    bool memWrite;    // Write to memory
    bool aluSrc;      // Select immediate or register value as ALU source
    bool memToReg;    // Select between ALU result or memory read
    uint8_t aluOp;    // ALU operation control

    Control();                 // Constructor to initialize signals
    void setControl(uint32_t opcode,uint32_t funct3);  // Set control signals based on opcode
};

#endif // CONTROL_HPP
