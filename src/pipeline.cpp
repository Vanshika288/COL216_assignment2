#include "pipeline.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>

Pipeline::Pipeline(bool forwarding) : forwardingEnabled(forwarding) {
    // reg = new Registers();
    // alu = new ALU();

    // No need to allocate with `new` since objects are already initialized
}

// REMOVE THIS - NO NEED FOR A DESTRUCTOR
// Pipeline::~Pipeline() {
//     delete registers;
//     delete alu;
// }

void Pipeline::loadInstructions(string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        uint32_t machineCode;
        ss >> std::hex >> machineCode;
        instructionMemory.push_back(machineCode);
    }
    file.close();
}

// void Pipeline::runPipeline(int cycles) {
//     int pc = 0;
//     for (int cycle = 0; cycle < cycles; cycle++) {
//         // Pipeline Stages
//         writeBack();
//         memory();
//         execute();
//         decode();
//         fetch(pc);

//         // Print pipeline state
//         printPipelineState();
//     }
// }

void Pipeline::runPipeline(int cycles) {
    for (int cycle = 0; cycle < cycles; cycle++) {
        // Pipeline Stages (in correct order)
        WB_stage();    // Write-Back stage
        MEM_stage();   // Memory stage
        EX_stage();    // Execute stage
        ID_stage();    // Decode stage
        IF_stage();    // Fetch stage

        // Print the pipeline state
        printPipeline(cycle + 1);
    }
}

// IF_stage: Fetch stage
void Pipeline::IF_stage() {
    if (pc / 4 >= instructionMemory.size()) {
        if_id.instruction = 0;
        if_id.pc = 0;
        return;
    }

    if_id.instruction = instructionMemory[pc / 4];
    if_id.pc = pc;
    pc += 4;
}

// ID_stage: Decode stage
void Pipeline::ID_stage() {
    if (if_id.instruction == 0) {
        id_ex.pc = 0;
        return;
    }

    id_ex.pc = if_id.pc;

    // Decode instruction fields
    id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
    id_ex.rs2 = (if_id.instruction >> 20) & 0x1F;
    id_ex.rd = (if_id.instruction >> 7) & 0x1F;
    id_ex.func3 = (if_id.instruction >> 12) & 0x7;
    id_ex.func7 = (if_id.instruction >> 25) & 0x7F;
    id_ex.opcode = if_id.instruction & 0x7F;

    // Load data from registers
    id_ex.data1 = reg.read(id_ex.rs1);
    id_ex.data2 = reg.read(id_ex.rs2);

    // Sign-extend immediate (depends on opcode)
    id_ex.imm = signExtend(if_id.instruction);

    // Set control signals
    id_ex.control = controlUnit.decode(id_ex.opcode);
}

// EX_stage: Execute stage
void Pipeline::EX_stage() {
    if (id_ex.pc == 0) {
        ex_mem.rd = 0;
        return;
    }

    // Perform ALU operation
    if (id_ex.control.aluOp) {
        ex_mem.aluResult = alu.compute(id_ex.opcode, id_ex.func3, id_ex.func7, id_ex.data1, id_ex.data2, id_ex.imm);
    } else {
        ex_mem.aluResult = 0;
    }

    // Pass data and control to next stage
    ex_mem.rd = id_ex.rd;
    ex_mem.data2 = id_ex.data2;
    ex_mem.control = id_ex.control;
}

// MEM_stage: Memory stage
void Pipeline::MEM_stage() {
    if (ex_mem.rd == 0) {
        mem_wb.rd = 0;
        return;
    }

    // Memory operations
    if (ex_mem.control.memRead) {
        mem_wb.memData = memory[ex_mem.aluResult];
    } else if (ex_mem.control.memWrite) {
        memory[ex_mem.aluResult] = ex_mem.data2;
    } else {
        mem_wb.memData = 0;
    }

    // Pass data and control to next stage
    mem_wb.aluResult = ex_mem.aluResult;
    mem_wb.rd = ex_mem.rd;
    mem_wb.control = ex_mem.control;
}

// WB_stage: Write-Back stage
void Pipeline::WB_stage() {
    if (mem_wb.rd == 0) return;

    // Write result back to the register file
    if (mem_wb.control.regWrite) {
        if (mem_wb.control.memToReg) {
            reg.write(mem_wb.rd, mem_wb.memData);
        } else {
            reg.write(mem_wb.rd, mem_wb.aluResult);
        }
    }
}

// Print pipeline state
void Pipeline::printPipeline(int cycle) {
    std::cout << "Cycle " << cycle << " -> ";
    std::cout << "IF: " << (if_id.pc ? std::to_string(if_id.pc) : "N/A") << " | ";
    std::cout << "ID: " << (id_ex.pc ? std::to_string(id_ex.pc) : "N/A") << " | ";
    std::cout << "EX: " << (ex_mem.rd ? std::to_string(ex_mem.aluResult) : "N/A") << " | ";
    std::cout << "MEM: " << (mem_wb.rd ? std::to_string(mem_wb.aluResult) : "N/A") << " | ";
    std::cout << "WB: " << (mem_wb.rd ? std::to_string(mem_wb.rd) : "N/A") << "\n";
}



// void Pipeline::fetch(int &pc) {
//     if (pc / 4 >= instructionMemory.size()) {
//         ifStage.active = false;
//         return;
//     }

//     ifStage.instruction = instructionMemory[pc / 4];
//     ifStage.pc = pc;
//     ifStage.active = true;
//     pc += 4;
// }

// void Pipeline::decode() {
//     if (!ifStage.active) {
//         idStage.active = false;
//         return;
//     }

//     idStage.pc = ifStage.pc;
//     idStage.instruction = ifStage.instruction;

//     // Decode instruction fields
//     idStage.rs1 = (idStage.instruction >> 15) & 0x1F;
//     idStage.rs2 = (idStage.instruction >> 20) & 0x1F;
//     idStage.rd = (idStage.instruction >> 7) & 0x1F;
//     idStage.funct3 = (idStage.instruction >> 12) & 0x7;
//     idStage.funct7 = (idStage.instruction >> 25) & 0x7F;
//     idStage.opcode = idStage.instruction & 0x7F;

//     idStage.active = true;
// }

// void Pipeline::execute() {
//     if (!idStage.active) {
//         exStage.active = false;
//         return;
//     }

//     exStage.pc = idStage.pc;
//     exStage.opcode = idStage.opcode;
//     exStage.rd = idStage.rd;
//     exStage.active = true;

//     // Execute ALU operation
//     exStage.aluResult = alu->compute(idStage.opcode, idStage.funct3, idStage.funct7,
//                                      registers->read(idStage.rs1), registers->read(idStage.rs2));
// }

// void Pipeline::memory() {
//     if (!exStage.active) {
//         memStage.active = false;
//         return;
//     }

//     memStage.pc = exStage.pc;
//     memStage.opcode = exStage.opcode;
//     memStage.rd = exStage.rd;
//     memStage.aluResult = exStage.aluResult;
//     memStage.active = true;
// }

// void Pipeline::writeBack() {
//     if (!memStage.active) return;
    
//     registers->write(memStage.rd, memStage.aluResult);
// }

// void Pipeline::printPipelineState() {
//     std::cout << std::setw(10) << "IF: " << ifStage.pc << " | ";
//     std::cout << "ID: " << idStage.pc << " | ";
//     std::cout << "EX: " << exStage.pc << " | ";
//     std::cout << "MEM: " << memStage.pc << " | ";
//     std::cout << "WB: " << (memStage.active ? std::to_string(memStage.rd) : "N/A") << std::endl;
// }
