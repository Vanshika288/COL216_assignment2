#include "pipeline.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>

Pipeline::Pipeline(bool forwarding) : forwarding(forwarding) {
    registers = new RegisterFile();
    alu = new ALU();
}

Pipeline::~Pipeline() {
    delete registers;
    delete alu;
}

void Pipeline::loadInstructions(const std::string &filename) {
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

void Pipeline::runPipeline(int cycles) {
    int pc = 0;
    for (int cycle = 0; cycle < cycles; cycle++) {
        // Pipeline Stages
        writeBack();
        memory();
        execute();
        decode();
        fetch(pc);

        // Print pipeline state
        printPipelineState();
    }
}

void Pipeline::fetch(int &pc) {
    if (pc / 4 >= instructionMemory.size()) {
        ifStage.active = false;
        return;
    }

    ifStage.instruction = instructionMemory[pc / 4];
    ifStage.pc = pc;
    ifStage.active = true;
    pc += 4;
}

void Pipeline::decode() {
    if (!ifStage.active) {
        idStage.active = false;
        return;
    }

    idStage.pc = ifStage.pc;
    idStage.instruction = ifStage.instruction;

    // Decode instruction fields
    idStage.rs1 = (idStage.instruction >> 15) & 0x1F;
    idStage.rs2 = (idStage.instruction >> 20) & 0x1F;
    idStage.rd = (idStage.instruction >> 7) & 0x1F;
    idStage.funct3 = (idStage.instruction >> 12) & 0x7;
    idStage.funct7 = (idStage.instruction >> 25) & 0x7F;
    idStage.opcode = idStage.instruction & 0x7F;

    idStage.active = true;
}

void Pipeline::execute() {
    if (!idStage.active) {
        exStage.active = false;
        return;
    }

    exStage.pc = idStage.pc;
    exStage.opcode = idStage.opcode;
    exStage.rd = idStage.rd;
    exStage.active = true;

    // Execute ALU operation
    exStage.aluResult = alu->compute(idStage.opcode, idStage.funct3, idStage.funct7,
                                     registers->read(idStage.rs1), registers->read(idStage.rs2));
}

void Pipeline::memory() {
    if (!exStage.active) {
        memStage.active = false;
        return;
    }

    memStage.pc = exStage.pc;
    memStage.opcode = exStage.opcode;
    memStage.rd = exStage.rd;
    memStage.aluResult = exStage.aluResult;
    memStage.active = true;
}

void Pipeline::writeBack() {
    if (!memStage.active) return;
    
    registers->write(memStage.rd, memStage.aluResult);
}

void Pipeline::printPipelineState() {
    std::cout << std::setw(10) << "IF: " << ifStage.pc << " | ";
    std::cout << "ID: " << idStage.pc << " | ";
    std::cout << "EX: " << exStage.pc << " | ";
    std::cout << "MEM: " << memStage.pc << " | ";
    std::cout << "WB: " << (memStage.active ? std::to_string(memStage.rd) : "N/A") << std::endl;
}
