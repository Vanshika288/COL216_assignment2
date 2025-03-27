#include "alu.hpp"
#include <iostream>
#include <limits>

ALU::ALU() {
    result = 0;
    zero = false;
    overflow = false;
}

// Perform ALU operation based on aluOp
void ALU::execute(uint8_t aluOp, int32_t input1, int32_t input2) {
    switch (aluOp) {
        case 0:  // ADD
            result = input1 + input2;
            overflow = detectOverflow(input1, input2, result, aluOp);
            break;

        case 1:  // SUBTRACT
            result = input1 - input2;
            overflow = detectOverflow(input1, -input2, result, aluOp);
            break;

        case 2:  // AND
            result = input1 & input2;
            overflow = false;
            break;

        case 3:  // OR
            result = input1 | input2;
            overflow = false;
            break;

        case 4:  // XOR
            result = input1 ^ input2;
            overflow = false;
            break;

        case 5:  // SLT (Set Less Than)
            result = (input1 < input2) ? 1 : 0;
            overflow = false;
            break;

        case 6:  // SLL (Shift Left Logical) slli
            result = input1 << (input2 & 0x1F);  // Masking for 5 bits
            overflow = false;
            break;

        case 7:  // SRL (Shift Right Logical)
            result = (uint32_t)input1 >> (input2 & 0x1F);
            overflow = false;
            break;

        default:
            std::cerr << "Unknown ALU operation!" << std::endl;
            result = 0;
            overflow = false;
            break;
    }

    zero = (result == 0);  // Set zero flag if result is 0
}

// Check for overflow during addition and subtraction
bool ALU::detectOverflow(int32_t a, int32_t b, int32_t res, uint8_t aluOp) {
    if (aluOp == 0) {  // ADD
        return ((a > 0 && b > 0 && res < 0) || (a < 0 && b < 0 && res > 0));
    } else if (aluOp == 1) {  // SUBTRACT
        return ((a > 0 && b < 0 && res < 0) || (a < 0 && b > 0 && res > 0));
    }
    return false;  // No overflow for other operations
}
