#include "control.hpp"
#include <iostream>
using namespace std;
Control::Control() {
    // Initialize all control signals to 0 (safe defaults)
    regWrite = false;
    memRead = false;
    memWrite = false;
    aluSrc = false;
    memToReg = false;
    aluOp = 0;
}

// Set control signals based on opcode
void Control::setControl(uint32_t opcode, uint32_t funct3) {
    switch (opcode) {
        case 0x33:  // R-type (ADD OR AND)
            switch (funct3){
                case 0 :  // ADD
                    regWrite = true;
                    aluSrc = false;
                    memRead = false;
                    memWrite = false;
                    memToReg = false;
                    aluOp = 0;
                    break;
                case 6:  //OR
                    regWrite = true;
                    aluSrc = false;
                    memRead = false;
                    memWrite = false;
                    memToReg = false;
                    aluOp = 3;
                    break;
                case 7: //AND
                    regWrite = true;
                    aluSrc = false;
                    memRead = false;
                    memWrite = false;
                    memToReg = false;
                    aluOp = 2;
                    break;
                case 2 : //Slt
                    regWrite = true;
                    aluSrc = false;
                    memRead = false;
                    memWrite = false;
                    memToReg = false;
                    aluOp = 5;
                    break;
                default:
                    break;
            }
            
            break;

        case 0x13:  // I-type (ADDI, ANDI, ORI, etc.) 
            switch (funct3)
            {
            case 0:
                regWrite = true;
                aluSrc = true;
                memRead = false;
                memWrite = false;
                memToReg = false;
                aluOp = 0;
                /* code */
                break;
            case 1:
                regWrite = true;
                aluSrc = true;
                memRead = false;
                memWrite = false;
                memToReg = false;
                aluOp = 6;
                /* code */
                break;
            case 5:
                regWrite = true;
                aluSrc = true;
                memRead = false;
                memWrite = false;
                memToReg = false;
                aluOp = 7;
                /* code */
                break;
            default:
                break;
            }
            
            break;

        case 0x03:  // Load (LW)
            regWrite = true;
            aluSrc = true;
            memRead = true;
            memWrite = false;
            memToReg = true;
            aluOp = 0;
            break;

        case 0x23:  // Store (SW)
            regWrite = false;
            aluSrc = true;
            memRead = false;
            memWrite = true;
            memToReg = false;
            aluOp = 0;
            break;

        case 0x63:  // Branch (BEQ, BNE, etc.)
            regWrite = false;
            aluSrc = false;
            memRead = false;
            memWrite = false;
            memToReg = false;
            aluOp = 1;
            break;

        case 0x6F:  // JAL (Jump and Link)
            regWrite = true;
            aluSrc = false;
            memRead = false;
            memWrite = false;
            memToReg = false;
            aluOp = 3;
            break;
        case 0x67:  // JALR 
            regWrite = true;
            aluSrc = false;
            memRead = false;
            memWrite = false;
            memToReg = false;
            aluOp = 3;
            break;
        
        case 0x17 :
            regWrite = true;
            aluSrc = false;
            memRead = false;
            memWrite = false;
            memToReg = false;
            aluOp = 0;
            break;

        default:
            std::cerr << "Warning: Unknown opcode 0x" << std::hex << opcode << std::endl;
            regWrite = false;
            aluSrc = false;
            memRead = false;
            memWrite = false;
            memToReg = false;
            aluOp = 0;
    }
}
