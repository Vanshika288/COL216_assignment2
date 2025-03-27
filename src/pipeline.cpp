#include "pipeline.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>

Pipeline::Pipeline(bool forwarding) : forwardingEnabled(forwarding)
{

}


void Pipeline::loadInstructions(string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        uint32_t machineCode;
        ss >> std::hex >> machineCode;
        instructionMemory.push_back(machineCode);
    }
    file.close();
}

void Pipeline::load_string_instructions(string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        exit(1);
    }
    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        instructions.push_back(line);
    }
    file.close();
}

void Pipeline::runPipeline(int cycles)
{
    for (int cycle = 0; cycle < cycles; cycle++)
    {
        // Pipeline Stages (in correct order)
        WB_stage(cycle);  // Write-Back stage
        MEM_stage(cycle); // Memory stage
        EX_stage(cycle);  // Execute stage
        ID_stage(cycle);  // Decode stage
        IF_stage(cycle);  // Fetch stage

       
    }
}

// IF_stage: Fetch stage
void Pipeline::IF_stage(int cycle)
{
    if (if_id.free_latch)
    {
        if (pc / 4 >= instructionMemory.size())
        {
            if_id.instruction = 0;
            if_id.pc = -1;
            return;
        }
        if_id.instruction = instructionMemory[pc / 4];
        if_id.pc = pc;
        pc += 4;
        instr_fetch.push_back({cycle, {(if_id.pc) / 4, 1}});
    }
    else
    {
        instr_fetch.push_back({cycle, {(if_id.pc) / 4, 0}});
        return;
    }
}

// ID_stage: Decode stage
void Pipeline::ID_stage(int cycle)
{
    uint32_t previous_opcode = id_ex.opcode;
    // jalr and jal and bne and beq logics :

    // jalr instr
    if (id_ex.opcode == 0x67 || id_ex.opcode == 0x6F)
    {
        if_id.instruction = 0;
        // id_ex.no_op = true;
        pc = id_ex.pc_new;
    }
    else if (id_ex.opcode == 0x63)
    {
        if (id_ex.pc_new != if_id.pc)
        {
            if_id.instruction = 0;
            // id_ex.no_op = true;
            pc = id_ex.pc_new;
        }
    }
    bool earlier_stall = false;
    if (!ID_stall)
    {
        if_id.free_latch = true;
        if (if_id.instruction == 0)
        {
            id_ex.no_op = true;
            id_ex.pc = 0;
            id_ex.pc_new = 0;
            id_ex.rs1 = 0;
            id_ex.rs2 = 0;
            id_ex.rd = 0;
            id_ex.imm = 0;
            id_ex.func3 = 0;
            id_ex.func7 = 0;
            id_ex.opcode = 0;
            id_ex.data1 = 0;
            id_ex.data2 = 0;
            return;
        }
        // take from previous latch

        // initialise everything to zero.
        id_ex.pc = 0;
        id_ex.pc_new = 0;
        id_ex.rs1 = 0;
        id_ex.rs2 = 0;
        id_ex.rd = 0;
        id_ex.imm = 0;
        id_ex.func3 = 0;
        id_ex.func7 = 0;
        id_ex.opcode = 0;
        id_ex.data1 = 0;
        id_ex.data2 = 0;


        // start assigning to id_ex fields
        id_ex.pc = if_id.pc;

        // Decode instruction fields
        id_ex.opcode = if_id.instruction & 0x7F; // Extract opcode

        switch (id_ex.opcode)
        {
        case 0x33: // add subt etc. // R type
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rs2 = (if_id.instruction >> 20) & 0x1F;
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.func7 = (if_id.instruction >> 25) & 0x7F;
            break;

        case 0x13: // ADDI-type instruction (I-type)

            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;  // Extract rs1 (bits 19-15)
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;    // Extract rd (bits 11-7)
            id_ex.func3 = (if_id.instruction >> 12) & 0x7; // Extract funct3 (bits 14-12)

            if (id_ex.func3 == 0x1)
            { // SLLI
                id_ex.imm = (if_id.instruction >> 20) & 0x1F; // Extract shamt (bits 24-20)
            }
            else if (id_ex.func3 == 0x5)
            {                                                   // SRLI or SRAI
                id_ex.func7 = (if_id.instruction >> 26) & 0x3F; // Extract funct6 (bits 31-26) //it's actually func6 only
                id_ex.imm = (if_id.instruction >> 20) & 0x1F;   // Extract shamt (bits 24-20)

                if (id_ex.func7 == 0x00)
                { // SRLI
                }
                else if (id_ex.func7 == 0x10)
                { // SRAI
                    // Sign-extend the shift amount if MSB (bit 4) is 1
                    if (id_ex.imm & 0x10)
                    {
                        id_ex.imm |= 0xFFFFFFE0; // Extend using 5-bit signed immediate
                    }
                }
            }
            else if (id_ex.func3 == 0x0)
            { // ADDI
                id_ex.imm = (int32_t)(if_id.instruction >> 20) & 0xFFF; // Extract 12-bit immediate
                if (if_id.instruction & 0x80000000)
                {                            // Check sign bit (bit 31)
                    id_ex.imm |= 0xFFFFF000; // Sign-extend to 32 bits
                }
            }
            break;

        case 0x03: // load word inst // I type
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.imm = (int32_t)(if_id.instruction >> 20) & 0xFFF; // Sign-extended 12-bit immediate
            if (if_id.instruction & 0x80000000)                     // Sign-extension
                id_ex.imm |= 0xFFFFF000;
            break;
        case 0x67: // Jalr instr. //I type
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.imm = (int32_t)(if_id.instruction >> 20) & 0xFFF; // Sign-extended 12-bit immediate
            if (if_id.instruction & 0x80000000)                     // Sign-extension
                id_ex.imm |= 0xFFFFF000;
            break;

        case 0x23: // store word type // S type
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rs2 = (if_id.instruction >> 20) & 0x1F;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.imm = ((if_id.instruction >> 25) << 5) | ((if_id.instruction >> 7) & 0x1F); // 7-bit + 5-bit
            if (if_id.instruction & 0x80000000)
                id_ex.imm |= 0xFFFFF000;
            break;

        case 0x63: // bne beq bge blt  // SB type
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rs2 = (if_id.instruction >> 20) & 0x1F;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.imm = ((if_id.instruction >> 31) << 12) |         // Bit 12 (sign bit)
                        (((if_id.instruction >> 7) & 1) << 11) |    // Bit 11
                        (((if_id.instruction >> 25) & 0x3F) << 5) | // Bits 10:5
                        (((if_id.instruction >> 8) & 0xF) << 1);    // Bits 4:1
            if (if_id.instruction & 0x80000000)
                id_ex.imm |= 0xFFFFE000;
            break;

        case 0x6F: // jal type   //UJ-type opcode
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.imm = ((if_id.instruction >> 31) << 20) |          // Bit 20 (sign bit)
                        (((if_id.instruction >> 12) & 0xFF) << 12) | // Bits 19:12
                        (((if_id.instruction >> 20) & 1) << 11) |    // Bit 11
                        ((if_id.instruction >> 21) & 0x3FF) << 1;    // Bits 10:1
            if (if_id.instruction & 0x80000000)
                id_ex.imm |= 0xFFE00000;
            break;
        case 0x17: // U-type (AUIPC)
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.imm = if_id.instruction & 0xFFFFF000; // Upper 20 bits
            // No sign extension required
            break;

        default:
            printf("Unknown opcode: 0x%x\n", id_ex.opcode);
            break;
        }

        // Set control signals
        controlUnit.setControl(id_ex.opcode, id_ex.func3);
        id_ex.control = controlUnit;

        
    }
    else
    {
        earlier_stall = true;
    }

    if (forwardingEnabled)
    {
        // by default they are equal to regWrite values only.
        id_ex.data1 = reg.read(id_ex.rs1);
        id_ex.data2 = reg.read(id_ex.rs2);

        if (id_ex.opcode == 0x63 || id_ex.opcode == 0x67 || id_ex.opcode == 0x6F)
        {
            if (ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1 || ex_mem.rd == id_ex.rs2))
            {
                ID_stall = true;
                id_ex.no_op = true;
                id_ex.forwardA = 0;
                id_ex.forwardB = 0;
                id_ex.mem_forward = false;
            }
            else if (mem_wb.rd != 0 && mem_wb.control.regWrite && (mem_wb.rd == id_ex.rs1 || mem_wb.rd == id_ex.rs2))
            {
                if (mem_wb.opcode == 0x03)
                {
                    ID_stall = true;
                    id_ex.no_op = true;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                }
                else
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                    if (id_ex.rs1 == mem_wb.rd)
                    {
                        id_ex.data1 = mem_wb.aluResult;
                    }
                    if (id_ex.rs2 == mem_wb.rd)
                    {
                        id_ex.data2 = mem_wb.aluResult;
                    }
                }
            }
            else
            {
                ID_stall = false;
                id_ex.no_op = false;
                id_ex.forwardA = 0;
                id_ex.forwardB = 0;
                id_ex.mem_forward = false;
            }
        }
        else
        {
            if (previous_opcode == 0x03)
            {

                if (id_ex.opcode != 0x23 && ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1 || ex_mem.rd == id_ex.rs2))
                {
                    ID_stall = true;
                    id_ex.no_op = true;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                }
                else if (id_ex.opcode == 0x23 && ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1))
                {
                    ID_stall = true;
                    id_ex.no_op = true;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                }
                else if (id_ex.opcode == 0x23 && ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs2))
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = true;
                }
                else
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                }
            }
            else
            {
                if (ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1 && ex_mem.rd == id_ex.rs2))
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 2;
                    id_ex.forwardB = 2;
                    id_ex.mem_forward = false;
                }
                else if (ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs2))
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 2;
                    id_ex.mem_forward = false;
                }
                else if (ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1))
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 2;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                }
                else
                {
                    ID_stall = false;
                    id_ex.no_op = false;
                    id_ex.forwardA = 0;
                    id_ex.forwardB = 0;
                    id_ex.mem_forward = false;
                }
            }
            if (mem_wb.control.regWrite && mem_wb.rd != 0 && !(ex_mem.control.regWrite && ex_mem.rd != 0 && ex_mem.rd == id_ex.rs1) && mem_wb.rd == id_ex.rs1)
            {
                ID_stall = false;
                id_ex.no_op = false;
                id_ex.forwardA = 1;
                id_ex.mem_forward = false;
            }
            if (mem_wb.control.regWrite && mem_wb.rd != 0 && !(ex_mem.control.regWrite && ex_mem.rd != 0 && ex_mem.rd == id_ex.rs2) && mem_wb.rd == id_ex.rs2)
            {
                ID_stall = false;
                id_ex.no_op = false;
                id_ex.forwardB = 1;
                id_ex.mem_forward = false;
            }
        }
    }
    else
    {
        if (mem_wb.rd != 0 && mem_wb.control.regWrite && (mem_wb.rd == id_ex.rs1 || mem_wb.rd == id_ex.rs2))
        {
            
            id_ex.forwardA = 0;
            id_ex.forwardB = 0;
            ID_stall = true;
            id_ex.no_op = true;
            
        }
        else if (ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1 || ex_mem.rd == id_ex.rs2))
        {
            
            id_ex.forwardA = 0;
            id_ex.forwardB = 0;
            ID_stall = true;
            id_ex.no_op = true;
        }
        else
        {
            id_ex.data1 = reg.read(id_ex.rs1);
            id_ex.data2 = reg.read(id_ex.rs2);

            id_ex.no_op = false;
            ID_stall = false;
        }
    }

    if (!ID_stall)
    {
        if (id_ex.opcode == 0x67)
        {
            // jalr type
            id_ex.pc_new = id_ex.data1 + id_ex.imm;
        }
        if (id_ex.opcode == 0x6F)
        {

            id_ex.pc_new = id_ex.pc + id_ex.imm;
        }
        if (id_ex.opcode == 0x63)
        {
            int val1 = id_ex.data1;
            int val2 = id_ex.data2;
            if (id_ex.func3 == 1)
            {
                // bne
                if (val1 != val2)
                {
                    id_ex.pc_new = id_ex.pc + id_ex.imm;
                }
                else
                {
                    id_ex.pc_new = id_ex.pc + 4;
                }
            }
            else if (id_ex.func3 == 0)
            {
                if (val1 == val2)
                {
                    id_ex.pc_new = id_ex.pc + id_ex.imm;
                }
                else
                {
                    id_ex.pc_new = id_ex.pc + 4;
                }
            }
            else if (id_ex.func3 == 4)
            {
                if (val1 < val2)
                {
                    id_ex.pc_new = id_ex.pc + id_ex.imm;
                }
                else
                {
                    id_ex.pc_new = id_ex.pc + 4;
                }
            }
            else if (id_ex.func3 == 5)
            {
                if (val1 >= val2)
                {
                    id_ex.pc_new = id_ex.pc + id_ex.imm;
                }
                else
                {
                    id_ex.pc_new = id_ex.pc + 4;
                }
            }
        }
    }

    if (!(earlier_stall))
    {
        instr_decode.push_back({cycle, {(id_ex.pc) / 4, 1}});
    }
    if (earlier_stall)
    {
        if_id.free_latch = false;
        instr_decode.push_back({cycle, {(id_ex.pc) / 4, 0}});
    }
}

// EX_stage: Execute stage
void Pipeline::EX_stage(int cycle)
{
    if (id_ex.no_op == true)
    {
        ex_mem.pc = 0;
        ex_mem.aluResult = 0;
        ex_mem.data2 = 0;
        ex_mem.no_op = true;
        ex_mem.rd = 0;
        ex_mem.opcode = 0;
        ex_mem.func3 = 0;
        return;
    }

    if (forwardingEnabled)
    {
        if (id_ex.forwardA == 2)
        {
            id_ex.data1 = ex_mem.aluResult;
        }
        else if (id_ex.forwardA == 1)
        {
            id_ex.data1 = reg.read(id_ex.rs1);
        }
        if (id_ex.forwardB == 2)
        {
            id_ex.data2 = ex_mem.aluResult;
        }
        else if (id_ex.forwardB == 1)
        {
            id_ex.data2 = reg.read(id_ex.rs2);
        }
    }

    alu.execute(id_ex.control.aluOp, id_ex.data1, id_ex.control.aluSrc ? id_ex.imm : id_ex.data2);
    if (id_ex.opcode == 0x6F || id_ex.opcode == 0x67)
    {
        alu.result = id_ex.pc + 4;
    }
    if (id_ex.opcode == 0x17)
    {
        alu.result = id_ex.pc + id_ex.imm;
    }
    ex_mem.aluResult = alu.result;
    // Pass data and control to next stage
    ex_mem.rd = id_ex.rd;
    ex_mem.data2 = id_ex.data2;
    ex_mem.control = id_ex.control;
    ex_mem.no_op = false;
    // instr_execute.push_back(cycle);
    ex_mem.pc = id_ex.pc;
    ex_mem.mem_forward = id_ex.mem_forward;
    ex_mem.opcode = id_ex.opcode;
    ex_mem.func3 = id_ex.func3;
    instr_execute.push_back({cycle, {(ex_mem.pc) / 4, 1}});
}

// MEM_stage: Memory stage
void Pipeline::MEM_stage(int cycle)
{
    if (ex_mem.no_op == true)
    {
        mem_wb.pc = 0;
        mem_wb.aluResult = 0;
        mem_wb.memData = 0;
        mem_wb.no_op = true;
        mem_wb.rd = 0;
        mem_wb.opcode = 0;
        mem_wb.func3 = 0;
        return;
    }
    if (forwardingEnabled)
    {
        if (ex_mem.mem_forward)
        {
            ex_mem.data2 = mem_wb.memData;
        }
    }
    // Memory operations
    if (ex_mem.control.memRead)
    {
        mem_wb.memData = memory[ex_mem.aluResult];
        if (mem_wb.func3 == 0)
        {
            mem_wb.memData = mem_wb.memData & 0xFF;
        }
        else if (mem_wb.func3 == 1)
        {
            mem_wb.memData = mem_wb.memData & 0xFFFF;
        }
        else
        {
        }
    }
    else if (ex_mem.control.memWrite)
    {
        memory[ex_mem.aluResult] = ex_mem.data2;
    }
    else
    {
        mem_wb.memData = 0;
    }

    // Pass data and control to next stage
    mem_wb.aluResult = ex_mem.aluResult;
    mem_wb.rd = ex_mem.rd;
    mem_wb.control = ex_mem.control;
    mem_wb.no_op = false;
    // instr_memory.push_back(cycle);
    mem_wb.pc = ex_mem.pc;
    mem_wb.opcode = ex_mem.opcode;
    mem_wb.func3 = ex_mem.func3;
    instr_memory.push_back({cycle, {(mem_wb.pc) / 4, 1}});
}

// WB_stage: Write-Back stage
void Pipeline::WB_stage(int cycle)
{
    if (mem_wb.no_op == true)
    {
        return;
    }

    // Write result back to the register file
    if (mem_wb.control.regWrite)
    {
        if (mem_wb.control.memToReg)
        {
            reg.write(mem_wb.rd, mem_wb.memData);
        }
        else
        {

            reg.write(mem_wb.rd, mem_wb.aluResult);
        }
    }
    else
    {
    }
    // instr_write.push_back(cycle);
    instr_write.push_back({cycle, {(mem_wb.pc) / 4, 1}});
}
string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t"); // Find first non-whitespace
    size_t end = s.find_last_not_of(" \t");    // Find last non-whitespace
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

void trimTrailingSemicolons(string &s)
{
    // Remove trailing "; "
    s.pop_back(); // Remove last character
    while (s.size() >= 2 && s.substr(s.size() - 2) == "; ")
    {
        s.erase(s.size() - 2); // Erase last 2 characters
    }
    // Remove last ';' if present
    if (!s.empty() && s.back() == ';')
    {
        s.pop_back();
    }
}

// Print pipeline state in deploy stage

void Pipeline :: printPipeline(int cycles){
    for (int i=0;i<instructions.size();i++){
        // cout<<trim(instructions[i])<<";";
        string s = "";
        s.append(trim(instructions[i]));
        s.push_back(';');
        // int cycle_to_be_printed = 0;

        int fetch_index = 0;
        int decode_index = 0;
        int ex_index = 0;
        int mem_index = 0;
        int write_index = 0;
        for (int j=0;j<cycles;j++){
            int instr_printed_in_curr_cycle = 0;
            if (instr_fetch[fetch_index].second.first == i){
                if (instr_fetch[fetch_index].first == j){
                    if (instr_fetch[fetch_index].second.second == 1){
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"IF";
                            s.append("IF");
                        }
                        else {
                            // cout<<"/IF";
                            s.append("/IF");
                        }

                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"-";
                            s.append("-");
                        }
                        else {
                            // cout<<"/-";
                            s.append("/-");
                        }
                    }
                    instr_printed_in_curr_cycle++;
                    fetch_index++;
                }
            }
            else {
                fetch_index++;
            }
            if (instr_decode[decode_index].second.first == i){
                if (instr_decode[decode_index].first == j){
                    if (instr_decode[decode_index].second.second == 1){
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"ID";
                            s.append("ID");
                        }
                        else {
                            // cout<<"/ID";
                            s.append("/ID");
                        }

                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"-";
                            s.append("-");
                        }
                        else {
                            // cout<<"/-";
                            s.append("/-");
                        }
                    }
                    instr_printed_in_curr_cycle++;
                    decode_index++;
                }
            }
            else {
                decode_index++;
            }
            if (instr_execute[ex_index].second.first == i){
                if (instr_execute[ex_index].first == j){
                    if (instr_execute[ex_index].second.second == 1){
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"EX";
                            s.append("EX");
                        }
                        else {
                            // cout<<"/EX";
                            s.append("/EX");
                        }

                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"-";
                            s.append("-");
                        }
                        else {
                            // cout<<"/-";
                            s.append("/-");
                        }
                    }
                    instr_printed_in_curr_cycle++;
                    ex_index++;
                }
            }
            else {
                ex_index++;
            }
            if (instr_memory[mem_index].second.first == i){
                if (instr_memory[mem_index].first == j){
                    if (instr_memory[mem_index].second.second == 1){
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"MEM";
                            s.append("MEM");
                        }
                        else {
                            // cout<<"/MEM";
                            s.append("/MEM");
                        }

                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"-";
                            s.append("-");
                        }
                        else {
                            // cout<<"/-";
                            s.append("/-");
                        }
                    }
                    instr_printed_in_curr_cycle++;
                    mem_index++;
                }
            }
            else {
                mem_index++;
            }
            if (instr_write[write_index].second.first == i){
                if (instr_write[write_index].first == j){
                    if (instr_write[write_index].second.second == 1){
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"WB";
                            s.append("WB");
                        }
                        else {
                            // cout<<"/WB";
                            s.append("/WB");
                        }

                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            // cout<<"-";
                            s.append("-");
                        }
                        else {
                            // cout<<"/-";
                            s.append("/-");
                        }
                    }
                    instr_printed_in_curr_cycle++;
                    write_index++;
                }
            }
            else {
                write_index++;
            }
            if (instr_printed_in_curr_cycle==0){
                // cout<<" ";
                s.append(" ");
            }
            // cout<<";"
            s.push_back(';');
        }
        trimTrailingSemicolons(s);
        cout<<s;
        // cout<<".";
        cout<<endl;
    }
}

// Print pipeline state in debug stage

// void Pipeline ::printPipeline(int cycles)
// {
//     for (int i = 0; i < instructions.size(); i++)
//     {
//         cout << left << setw(20) << trim(instructions[i]) << "|";
//         // int cycle_to_be_printed = 0;

//         int fetch_index = 0;
//         int decode_index = 0;
//         int ex_index = 0;
//         int mem_index = 0;
//         int write_index = 0;

//         for (int j = 0; j < cycles; j++)
//         {
//             int instr_printed_in_curr_cycle = 0;
//             if (instr_fetch[fetch_index].second.first == i)
//             {
//                 if (instr_fetch[fetch_index].first == j)
//                 {
//                     if (instr_fetch[fetch_index].second.second == 1)
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "IF  ";
//                         }
//                         else
//                         {
//                             cout << "/IF ";
//                         }
//                     }
//                     else
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "-   ";
//                         }
//                         else
//                         {
//                             cout << "/-  ";
//                         }
//                     }
//                     instr_printed_in_curr_cycle++;
//                     fetch_index++;
//                 }
//             }
//             else
//             {
//                 fetch_index++;
//             }
//             if (instr_decode[decode_index].second.first == i)
//             {
//                 if (instr_decode[decode_index].first == j)
//                 {
//                     if (instr_decode[decode_index].second.second == 1)
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "ID  ";
//                         }
//                         else
//                         {
//                             cout << "/ID ";
//                         }
//                     }
//                     else
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "-   ";
//                         }
//                         else
//                         {
//                             cout << "/-  ";
//                         }
//                     }
//                     instr_printed_in_curr_cycle++;
//                     decode_index++;
//                 }
//             }
//             else
//             {
//                 decode_index++;
//             }
//             if (instr_execute[ex_index].second.first == i)
//             {
//                 if (instr_execute[ex_index].first == j)
//                 {
//                     if (instr_execute[ex_index].second.second == 1)
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "EX  ";
//                         }
//                         else
//                         {
//                             cout << "/EX ";
//                         }
//                     }
//                     else
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "-   ";
//                         }
//                         else
//                         {
//                             cout << "/-  ";
//                         }
//                     }
//                     instr_printed_in_curr_cycle++;
//                     ex_index++;
//                 }
//             }
//             else
//             {
//                 ex_index++;
//             }
//             if (instr_memory[mem_index].second.first == i)
//             {
//                 if (instr_memory[mem_index].first == j)
//                 {
//                     if (instr_memory[mem_index].second.second == 1)
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "MEM ";
//                         }
//                         else
//                         {
//                             cout << "/MEM" << "|";
//                         }
//                     }
//                     else
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "-   ";
//                         }
//                         else
//                         {
//                             cout << "/-  ";
//                         }
//                     }
//                     instr_printed_in_curr_cycle++;
//                     mem_index++;
//                 }
//             }
//             else
//             {
//                 mem_index++;
//             }
//             if (instr_write[write_index].second.first == i)
//             {
//                 if (instr_write[write_index].first == j)
//                 {
//                     if (instr_write[write_index].second.second == 1)
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "WB  ";
//                         }
//                         else
//                         {
//                             cout << "/WB ";
//                         }
//                     }
//                     else
//                     {
//                         if (instr_printed_in_curr_cycle == 0)
//                         {
//                             cout << "-   ";
//                         }
//                         else
//                         {
//                             cout << "/-  ";
//                         }
//                     }
//                     instr_printed_in_curr_cycle++;
//                     write_index++;
//                 }
//             }
//             else
//             {
//                 write_index++;
//             }
//             if (instr_printed_in_curr_cycle == 0)
//             {
//                 cout << "    ";
//             }
//             cout << "|";
//         }
//         cout << endl;
//     }
// }

uint32_t Pipeline::signExtend(uint32_t instruction)
{
    uint32_t imm;
    uint32_t opcode = instruction & 0x7F;

    if (opcode == 0x13 || opcode == 0x03)
    {
        // I-type: Immediate from bits [31:20]
        imm = (instruction >> 20) & 0xFFF;
        if (imm & 0x800)
        {                      // Check sign bit (bit 11)
            imm |= 0xFFFFF000; // Sign-extend to 32-bit
        }
    }
    else if (opcode == 0x23)
    {
        // S-type: Immediate from bits [31:25] and [11:7]
        imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
        if (imm & 0x800)
        {
            imm |= 0xFFFFF000;
        }
    }
    else if (opcode == 0x63)
    {
        // B-type: Immediate for branching
        imm = ((instruction >> 31) << 12) |
              ((instruction >> 25) & 0x3F) << 5 |
              ((instruction >> 8) & 0xF) << 1 |
              ((instruction >> 7) & 0x1) << 11;
        if (imm & 0x1000)
        {
            imm |= 0xFFFFE000;
        }
    }
    else if (opcode == 0x6F)
    {
        // J-type: Immediate for JAL
        imm = ((instruction >> 31) << 20) |
              ((instruction >> 21) & 0x3FF) << 1 |
              ((instruction >> 20) & 0x1) << 11 |
              ((instruction >> 12) & 0xFF) << 12;
        if (imm & 0x100000)
        {
            imm |= 0xFFE00000;
        }
    }
    else
    {
        imm = 0; // Default case (for R-type or unknown)
    }
    return imm;
}

void Pipeline::dumpRegisters()
{
    reg.dump(); // Call dump from Registers class
}