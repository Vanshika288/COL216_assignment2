#ifndef ALU_HPP
#define ALU_HPP

#include <cstdint>

class ALU {
public:
    int32_t result;   // Stores result of ALU operations
    bool zero;        // Zero flag (set if result == 0)
    bool overflow;    // Overflow flag

    ALU();                                  // Constructor
    void execute(uint8_t aluOp, int32_t input1, int32_t input2); // ALU operation
    bool detectOverflow(int32_t a, int32_t b, int32_t res, uint8_t aluOp);
};

#endif // ALU_HPP
