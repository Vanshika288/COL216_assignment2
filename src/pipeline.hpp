/* ================= pipeline.hpp ================= */
#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "control.hpp"
#include "registers.hpp"
#include "alu.hpp"

using namespace std;

// Pipeline register structure for each stage
struct IF_ID {
    uint32_t instruction = 0;
    uint32_t pc = 0;
    bool free_latch = false;
};

struct ID_EX {
    uint32_t pc = 0;
    uint32_t rs1 = 0, rs2 = 0, rd = 0;
    uint32_t imm = 0;
    uint32_t func3 = 0, func7 = 0;
    uint32_t opcode = 0;
    uint32_t data1 = 0, data2 = 0;
    Control control;
    bool no_op = true;
};

struct EX_MEM {
    uint32_t aluResult = 0;
    uint32_t rd = 0;
    uint32_t data2 = 0;  //in store word the register from which we take value 
    Control control;
    bool no_op = true;
};

struct MEM_WB {
    uint32_t memData = 0;
    uint32_t aluResult = 0;
    uint32_t rd = 0;
    Control control;
    bool no_op = true;
};

class Pipeline {
private:
    Registers reg;
    ALU alu;
    Control controlUnit;
    
    IF_ID if_id;
    ID_EX id_ex;
    EX_MEM ex_mem;
    MEM_WB mem_wb;

    vector<string> instructions;
    vector<uint32_t> instructionMemory;  // ✅ Stores 32-bit machine codes
    unordered_map<uint32_t, uint32_t> memory;
    uint32_t pc = 0;
    bool forwardingEnabled = false;

    uint32_t signExtend(uint32_t instruction);
    bool ID_stall = false;
    vector<long long int> instr_fetch;
    vector<long long int> instr_decode;
    vector<long long int> instr_execute;
    vector<long long int> instr_memory;
    vector<long long int> instr_write;

public:
    Pipeline(bool enableForwarding);
    void loadInstructions(string filename);
    void load_string_instructions(string filename);
    void runPipeline(int cycles);
    void IF_stage(int cycle);
    void ID_stage(int cycle);
    void EX_stage(int cycle);
    void MEM_stage(int cycle);
    void WB_stage(int cycle);
    void printPipeline();
    void dumpRegisters();
};

#endif
