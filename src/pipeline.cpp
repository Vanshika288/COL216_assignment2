#include "pipeline.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>

Pipeline::Pipeline(bool forwarding) : forwardingEnabled(forwarding)
{
    // reg = new Registers();
    // alu = new ALU();

    // No need to allocate with `new` since objects are already initialized
}

// REMOVE THIS - NO NEED FOR A DESTRUCTOR
// Pipeline::~Pipeline() {
//     delete registers;
//     delete alu;
// }

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

        // Print the pipeline state
        // printPipeline(cycle + 1);
    }
}

// IF_stage: Fetch stage
void Pipeline::IF_stage(int cycle)
{
    cout << "if_stage ";
    if (if_id.free_latch)
    {
        cout << "latch is empty and free" << endl;
        if (pc / 4 >= instructionMemory.size())
        {
            if_id.instruction = 0;
            if_id.pc = 0;
            return;
        }
        cout << "inst.fetch occurs" << endl;
        if_id.instruction = instructionMemory[pc / 4];
        cout << if_id.instruction << " " << pc << endl;
        if_id.pc = pc;
        pc += 4;
        // instr_fetch.push_back(cycle);
        table[(if_id.pc) / 4].push_back({"IF", cycle});
        instr_fetch.push_back({cycle, {(if_id.pc) / 4,1}});
    }
    else
    {
        instr_fetch.push_back({cycle,{(if_id.pc) / 4,0}});
        return;
    }
}

// ID_stage: Decode stage
void Pipeline::ID_stage(int cycle)
{
    cout << "id_stage" << endl;
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
        cout << "id_Stage no stall" << endl;
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
        cout << "fetches from prev latch" << endl;
        // take from previous latch
        id_ex.pc = if_id.pc;

        // Decode instruction fields
        id_ex.opcode = if_id.instruction & 0x7F; // Extract opcode

        switch (id_ex.opcode)
        {
        case 0x33: // add subt etc. // R type
            cout << "it's add instr" << endl;
            cout << "intruction" << if_id.instruction << endl;
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rs2 = (if_id.instruction >> 20) & 0x1F;
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            cout << "register values of add instr : " << id_ex.rs1 << " " << id_ex.rs2 << " " << id_ex.rd << endl;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.func7 = (if_id.instruction >> 25) & 0x7F;
            break;

        case 0x13: // addi type inst. //I type
            cout << "addi instr. recognised " << endl;
            id_ex.rs1 = (if_id.instruction >> 15) & 0x1F;
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.func3 = (if_id.instruction >> 12) & 0x7;
            id_ex.imm = (int32_t)(if_id.instruction >> 20) & 0xFFF; // Sign-extended 12-bit immediate
            if (if_id.instruction & 0x80000000)                     // Sign-extension
                id_ex.imm |= 0xFFFFF000;
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
                        ((if_id.instruction >> 8) & 0xF);           // Bits 4:1
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
            cout << "AUIPC instruction recognised" << endl;
            id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            id_ex.imm = if_id.instruction & 0xFFFFF000; // Upper 20 bits
            // No sign extension required
            cout << "AUIPC rd: " << id_ex.rd << ", imm: " << id_ex.imm << endl;
            break;
            // case U_TYPE_OPCODE:  // Replace with actual U-type opcode //for lui type
            //     id_ex.rd = (if_id.instruction >> 7) & 0x1F;
            //     id_ex.imm = if_id.instruction & 0xFFFFF000;  // Upper 20 bits
            //     break;

        default:
            printf("Unknown opcode: 0x%x\n", id_ex.opcode);
            break;
        }

        // Load data from registers
        // id_ex.data1 = reg.read(id_ex.rs1);
        // id_ex.data2 = reg.read(id_ex.rs2);

        // Sign-extend immediate (depends on opcode)
        if (id_ex.opcode != 0x17){
            id_ex.imm = signExtend(if_id.instruction);
        }
        // cout << id_ex.data1 << " " << id_ex.data2 << " " << id_ex.imm << endl;
        // Set control signals
        controlUnit.setControl(id_ex.opcode);
        id_ex.control = controlUnit;

        // if (id_ex.opcode == 0x67)
        // {
        //     // jalr type
        //     id_ex.pc_new = reg.read(id_ex.rs1) + id_ex.imm;
        // }
        // else if (id_ex.opcode == 0x6F)
        // {
        //     // jal type
        //     id_ex.pc_new = id_ex.pc + id_ex.imm;
        // }
        // else if (id_ex.opcode == 0x63)
        // {
        //     int val1 = reg.read(id_ex.rs1);
        //     int val2 = reg.read(id_ex.rs2);
        //     if (id_ex.func3 == 1)
        //     {
        //         // bne
        //         if (val1 != val2)
        //         {
        //             id_ex.pc_new = id_ex.pc + id_ex.imm;
        //         }
        //         else
        //         {
        //             id_ex.pc_new = id_ex.pc + 4;
        //         }
        //     }
        //     else if (id_ex.func3 == 0)
        //     {
        //         if (val1 == val2)
        //         {
        //             id_ex.pc_new = id_ex.pc + id_ex.imm;
        //         }
        //         else
        //         {
        //             id_ex.pc_new = id_ex.pc + 4;
        //         }
        //     }
        // }
    }
    else
    {
        cout << "does not fetch from prev latch" << endl;
        earlier_stall = true;
    }
    cout << "print" << ex_mem.rd << " " << mem_wb.rd << endl;

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
            else {
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
            // if (forwardingEnabled)
            // {
            //     if (mem_wb.rd == id_ex.rs1)
            //     {
            //         id_ex.forwardA = 1;
            //     }
            //     if (mem_wb.rd == id_ex.rs2)
            //     {
            //         id_ex.forwardB = 1;
            //     }
            // }
            // else
            //{
            id_ex.forwardA = 0;
            id_ex.forwardB = 0;
            ID_stall = true;
            id_ex.no_op = true;
            //}
        }
        else if (ex_mem.rd != 0 && ex_mem.control.regWrite && (ex_mem.rd == id_ex.rs1 || ex_mem.rd == id_ex.rs2))
        {
            // if (forwardingEnabled)
            // {
            //     if (ex_mem.rd == id_ex.rs1)
            //     {
            //         if (previous_opcode == 0x03){
            //             ID_stall = true;
            //             id_ex.no_op = true;
            //         }
            //         else {
            //             id_ex.forwardA = 2;
            //         }
            //     }
            //     else {
            //         id_ex.forwardA = 0;
            //     }
            //     if (ex_mem.rd == id_ex.rs2)
            //     {
            //         if (previous_opcode == 0x03){
            //             if (id_ex.opcode == 0x23){
            //                 ex_mem.mem_forward = true;  // for store word
            //             }
            //             else {
            //                 ID_stall = true;
            //                 id_ex.no_op = true;
            //             }
            //         }
            //         else {
            //             id_ex.forwardB = 2;
            //         }
            //     }
            //     else {
            //         id_ex.forwardB = 0;
            //     }
            // }
            // else
            //{
            id_ex.forwardA = 0;
            id_ex.forwardB = 0;
            ID_stall = true;
            id_ex.no_op = true;
            //}
        }
        else
        {
            cout << "no stalling" << endl;
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
            else if (id_ex.func3 == 4){
                if (val1 < val2){
                    id_ex.pc_new = id_ex.pc + id_ex.imm;
                }
                else {
                    id_ex.pc_new = id_ex.pc + 4;
                }
            }
            else if (id_ex.func3 == 5){
                if (val1 >= val2){
                    id_ex.pc_new = id_ex.pc + id_ex.imm;
                }
                else {
                    id_ex.pc_new = id_ex.pc + 4;
                }
            }
        }
    }

    if (!(earlier_stall))
    {
        // instr_decode.push_back(cycle);
        table[(id_ex.pc) / 4].push_back({"ID", cycle});
        instr_decode.push_back({cycle, {(id_ex.pc) / 4,1}});
    }
    if (earlier_stall)
    {
        cout << "earlier stall and free _latch set to false" << endl;
        if_id.free_latch = false;
        instr_decode.push_back({cycle, {(id_ex.pc) / 4,0}});
    }
}

// EX_stage: Execute stage
void Pipeline::EX_stage(int cycle)
{
    cout << "ex_stage ";
    if (id_ex.no_op == true)
    {
        ex_mem.pc = 0;
        ex_mem.aluResult = 0;
        ex_mem.data2 = 0;
        ex_mem.no_op = true;
        ex_mem.rd = 0;
        return;
    }

    // // Perform ALU operation
    // if (id_ex.control.aluOp) {
    //     ex_mem.aluResult = alu.compute(id_ex.opcode, id_ex.func3, id_ex.func7, id_ex.data1, id_ex.data2, id_ex.imm);
    // } else {
    //     ex_mem.aluResult = 0;
    // }

    // Perform ALU operation based on control signals

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

    cout << "some execution occurs" << endl;
    alu.execute(id_ex.control.aluOp, id_ex.data1, id_ex.control.aluSrc ? id_ex.imm : id_ex.data2);
    if (id_ex.opcode == 0x6F || id_ex.opcode == 0x67)
    {
        alu.result = id_ex.pc + 4;
    }
    if (id_ex.opcode == 0x17) {
        alu.result = id_ex.pc + id_ex.imm;
    }
    ex_mem.aluResult = alu.result;
    cout << ex_mem.aluResult << endl;
    // Pass data and control to next stage
    ex_mem.rd = id_ex.rd;
    cout << "when it returns zero as alu result : reg value " << ex_mem.rd << endl;
    ex_mem.data2 = id_ex.data2;
    ex_mem.control = id_ex.control;
    ex_mem.no_op = false;
    // instr_execute.push_back(cycle);
    ex_mem.pc = id_ex.pc;
    ex_mem.mem_forward = id_ex.mem_forward;
    ex_mem.opcode = id_ex.opcode;
    table[(ex_mem.pc) / 4].push_back({"EX", cycle});
    instr_execute.push_back({cycle, {(ex_mem.pc) / 4,1}});
}

// MEM_stage: Memory stage
void Pipeline::MEM_stage(int cycle)
{
    cout << "mem_stage " << endl;
    if (ex_mem.no_op == true)
    {
        mem_wb.pc = 0;
        mem_wb.aluResult = 0;
        mem_wb.memData = 0;
        mem_wb.no_op = true;
        mem_wb.rd = 0;
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
        cout << "to be read from mem" << endl;
        mem_wb.memData = memory[ex_mem.aluResult];
    }
    else if (ex_mem.control.memWrite)
    {
        cout << "to be written in mem" << endl;
        memory[ex_mem.aluResult] = ex_mem.data2;
    }
    else
    {
        cout << "no mem op" << endl;
        mem_wb.memData = 0;
    }

    // Pass data and control to next stage
    mem_wb.aluResult = ex_mem.aluResult;
    mem_wb.rd = ex_mem.rd;
    mem_wb.control = ex_mem.control;
    cout << mem_wb.aluResult << " " << mem_wb.rd << " " << endl;
    mem_wb.no_op = false;
    // instr_memory.push_back(cycle);
    mem_wb.pc = ex_mem.pc;
    mem_wb.opcode = ex_mem.opcode;
    table[(mem_wb.pc) / 4].push_back({"DM", cycle});
    instr_memory.push_back({cycle, {(mem_wb.pc) / 4,1}}); 
}

// WB_stage: Write-Back stage
void Pipeline::WB_stage(int cycle)
{
    cout << "wb_stage " << endl;
    if (mem_wb.no_op == true)
    {
        return;
    }

    // Write result back to the register file
    if (mem_wb.control.regWrite)
    {
        cout << "to be written" << endl;
        if (mem_wb.control.memToReg)
        {
            cout << "write_here from mem" << endl;
            reg.write(mem_wb.rd, mem_wb.memData);
        }
        else
        {
            cout << "write_here from reg" << endl;

            reg.write(mem_wb.rd, mem_wb.aluResult);
        }
    }
    else
    {
        cout << "not to be written" << endl;
    }
    // instr_write.push_back(cycle);
    table[(mem_wb.pc) / 4].push_back({"WB", cycle});
    instr_write.push_back({cycle, {(mem_wb.pc) / 4,1}});
}

// Print pipeline state
// void Pipeline::printPipeline(int cycle) {
//     std::cout << "Cycle " << cycle << " -> ";
//     std::cout << "IF: " << (if_id.pc ? std::to_string(if_id.pc) : "N/A") << " | ";
//     std::cout << "ID: " << (id_ex.pc ? std::to_string(id_ex.pc) : "N/A") << " | ";
//     std::cout << "EX: " << (ex_mem.rd ? std::to_string(ex_mem.aluResult) : "N/A") << " | ";
//     std::cout << "MEM: " << (mem_wb.rd ? std::to_string(mem_wb.aluResult) : "N/A") << " | ";
//     std::cout << "WB: " << (mem_wb.rd ? std::to_string(mem_wb.rd) : "N/A") << "\n";
// }

// Print function that is asked in the assignment

// void Pipeline::printPipeline(){
//     for (int i=0;i<instructions.size();i++){
//         cout<<instructions[i]<<";";
//         int s1 = instr_fetch[i];
//         int s2 = instr_decode[i];
//         int s3 = instr_execute[i];
//         int s4 = instr_memory[i];
//         int s5 = instr_write[i];

//         int cnt = 1;
//         int cycle = 0;
//         while(cnt<=5){
//             if (cnt==1){
//                 if (s1==cycle) {
//                     cout<<"IF;";
//                     cnt++;
//                 }
//                 else cout<<"-;";
//             }
//             else if (cnt==2){
//                 if (s2==cycle){
//                     cout<<"ID;";
//                     cnt++;
//                 }
//                 else cout<<"-;";
//             }
//             else if (cnt==3){
//                 if (s3==cycle){
//                     cout<<"EX;";
//                     cnt++;
//                 }
//                 else cout<<"-;";
//             }
//             else if (cnt==4){
//                 if (s4==cycle){
//                     cout<<"MEM;";
//                     cnt++;
//                 }
//                 else cout<<"-;";
//             }
//             else if (cnt==5){
//                 if (s5==cycle){
//                     cout<<"WB";
//                     cnt++;
//                 }
//                 else cout<<"-;";
//             }
//             cycle++;
//         }
//         cout<<endl;
//     }
// }

// Print function for debug stage
// void Pipeline::printPipeline() {
//     // Find the maximum cycle to align properly
//     int maxCycle = 0;
//     for (int i = 0; i < instructions.size(); i++) {
//         maxCycle = max((long long)maxCycle, instr_write[i]);
//     }

//     // Print each instruction with aligned stages
//     for (int i = 0; i < instructions.size(); i++) {
//         // Print instruction
//         cout << setw(20) << left << instructions[i] << "|";

//         int s1 = instr_fetch[i];
//         int s2 = instr_decode[i];
//         int s3 = instr_execute[i];
//         int s4 = instr_memory[i];
//         int s5 = instr_write[i];

//         // Print cycles with padding to align properly
//         for (int cycle = 0; cycle <= maxCycle; cycle++) {
//             if (cycle == s1) {
//                 cout << setw(5) << "IF";
//             }
//             else if (cycle == s2) {
//                 cout << setw(5) << "ID";
//             }
//             else if (cycle == s3) {
//                 cout << setw(5) << "EX";
//             }
//             else if (cycle == s4) {
//                 cout << setw(5) << "MEM";
//             }
//             else if (cycle == s5) {
//                 cout << setw(5) << "WB";
//             }
//             else {
//                 cout << setw(5) << " ";  // Empty cycles for padding
//             }
//         }

//         // Move to next line for next instruction
//         cout << endl;
//     }
// }

string trim(const string &s)
{
    size_t start = s.find_first_not_of(" \t"); // Find first non-whitespace
    size_t end = s.find_last_not_of(" \t");    // Find last non-whitespace
    return (start == string::npos) ? "" : s.substr(start, end - start + 1);
}

// void Pipeline::printPipeline()
// {
//     unordered_map<string, string> stageOrder = {{"IF", "ID"}, {"ID", "EX"}, {"EX", "DM"}, {"DM", "WB"}};

//     for (size_t i = 0; i < instructions.size(); i++)
//     {
//         cout << trim(instructions[i]) << ";"; // Print instruction followed by semicolon

//         if (table.find(i) == table.end())
//         {
//             cout << endl;
//             continue;
//         }

//         vector<pair<string, int>> stages = table[i];
//         int lastCycle = -1;                // Track last cycle for spacing
//         string lastPrintedStage = "start"; // Track last printed stage

//         for (const auto &[stage, cycle] : stages)
//         {
//             while (lastCycle + 1 < cycle)
//             {
//                 if (stage == "IF")
//                 {
//                     cout << " ;"; // Always print space for IF
//                 }
//                 else if (stageOrder[lastPrintedStage] == stage)
//                 {
//                     cout << "-;"; // Print stall
//                 }
//                 else
//                 {
//                     cout << " ;"; // Print flush
//                 }
//                 lastCycle++;
//             }

//             cout << stage << ";"; // Print the current stage
//             lastCycle = cycle;
//             lastPrintedStage = stage;
//         }

//         cout << endl;
//     }
// }

void Pipeline :: printPipeline(int cycles){
    for (int i=0;i<instructions.size();i++){
        cout<<trim(instructions[i])<<";";
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
                            cout<<"IF";
                        }
                        else {
                            cout<<"/IF";
                        }
                        
                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            cout<<"-";
                        }
                        else {
                            cout<<"/-";
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
                            cout<<"ID";
                        }
                        else {
                            cout<<"/ID";
                        }
                        
                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            cout<<"-";
                        }
                        else {
                            cout<<"/-";
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
                            cout<<"EX";
                        }
                        else {
                            cout<<"/EX";
                        }
                        
                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            cout<<"-";
                        }
                        else {
                            cout<<"/-";
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
                            cout<<"MEM";
                        }
                        else {
                            cout<<"/MEM";
                        }
                        
                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            cout<<"-";
                        }
                        else {
                            cout<<"/-";
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
                            cout<<"WB";
                        }
                        else {
                            cout<<"/WB";
                        }
                        
                    }
                    else {
                        if (instr_printed_in_curr_cycle == 0){
                            cout<<"-";
                        }
                        else {
                            cout<<"/-";
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
                cout<<" ";
            }
            cout<<";";
        }
        cout<<endl;
    }
}

// void Pipeline::printPipeline()
// {
//     unordered_map<string, string> stageOrder = {{"IF", "ID"}, {"ID", "EX"}, {"EX", "DM"}, {"DM", "WB"}};

//     // Determine the maximum cycle number for formatting
//     int maxCycle = 0;
//     for (const auto &entry : table)
//     {
//         for (const auto &[stage, cycle] : entry.second)
//         {
//             maxCycle = max(maxCycle, cycle);
//         }
//     }

//     for (size_t i = 0; i < instructions.size(); i++)
//     {
//         cout << left << setw(20) << trim(instructions[i]) << "|"; // Print instruction with padding

//         if (table.find(i) == table.end())
//         {
//             cout << endl;
//             continue;
//         }

//         vector<pair<string, int>> stages = table[i];
//         int lastCycle = -1;
//         string lastPrintedStage = "start";

//         for (int cycle = 0; cycle <= maxCycle; cycle++) // Iterate over all possible cycles
//         {
//             auto it = find_if(stages.begin(), stages.end(), [cycle](const pair<string, int> &p)
//                               { return p.second == cycle; });

//             if (it != stages.end())
//             {
//                 cout << setw(4) << it->first << "|"; // Print stage in proper column
//                 lastPrintedStage = it->first;
//                 lastCycle = cycle;
//             }
//             else
//             {
//                 if (lastCycle + 1 == cycle)
//                 {
//                     cout << setw(4) << "-" << "|"; // Stall
//                 }
//                 else
//                 {
//                     cout << setw(4) << " " << "|"; // Flush
//                 }
//             }
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
