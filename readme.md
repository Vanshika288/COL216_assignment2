
RISC-V Pipeline Simulator

Project Overview
This project simulates a 5-stage RISC-V pipeline with and without forwarding. It models the standard instruction execution stages:
- IF – Instruction Fetch
- ID – Instruction Decode
- EX – Execute
- MEM – Memory Access
- WB – Write Back

Features
- Supports key RISC-V instructions:
  - Arithmetic: add, addi, slli, srai, srli
  - Memory: lw, lb, lh, sw
  - Branches: beq, bne, bge, blt
  - Jumps: jal, jalr
  - More: auipc

- Implements hazard detection and inserts bubbles when required. We have put appropriate stalls and forwarded values properly wherever required.

- We have initialized all the register and memory values to 0, and we are updating the values as we proceed, since we are doing the calculations necessary in the ALU stage. We have simulated the processor completed with all the functionalities.

- Branches: We are resolving branches and jumps in the ID stage by taking care of the values i.e., we have not always assumed branch not taken, but are resolving branches according to the register and memory values. For example, in beq, we will take the branch if the 2 register values are equal, and not take the branch otherwise.

- We have printed the pipeline at the end. We can also print the register values if required as a debug measure. We have printed '/' as a delimiter where the instruction is in more than one stage.

- To run all the test cases, we have added a testbench shell script (testbench.sh) to automatically run all the test cases at once. Instructions to run testbench are mentioned in testbench.sh.


- We have 'gitignore' git feature to store all executables and object files in one place.

Directory Structure
/inputfiles
/outputfiles
/src
├── alu.cpp             # ALU implementation
├── alu.hpp             # ALU header file
├── control.cpp         # Control unit implementation
├── control.hpp         # Control unit header file
├── pipeline.cpp        # Pipeline logic (without forwarding)
├── pipeline_old.cpp    # Pipeline logic (with forwarding)
├── pipeline.hpp        # Pipeline header file
├── registers.cpp       # Register file implementation
├── registers.hpp       # Register file header file
├── main.cpp            # Main driver file
├── assembly_code.txt   # Stores assembly instructions
├── machine_code.txt    # Stores corresponding machine code
├── Makefile.txt        # Makefile

Build Instructions
1. Build Using Makefile
# To build both the versions
make

# To build the version with forwarding
make forward

# To build the version without forwarding
make noforward

2. Run the Executables
# Run the simulator with forwarding
$ ./forward ../inputfiles/filename.txt cyclecount

# Run the simulator without forwarding
$ ./noforward ../inputfiles/filename.txt cyclecount

Makefile Targets
- make forward – Build the pipeline simulator with forwarding.
- make noforward – Build the pipeline simulator without forwarding.
- make clean – Remove all generated .o files and executables.

Design Decisions
- We resolved branches in the ID stage to minimize unnecessary stalls.
- We have done abstraction, making the code modular and easy to understand. We have made separate files for ALU, control unit, and register file.
- We have used a simple array to represent the register file, which is easy to manipulate.
- For memory, we have used map, so that we are not constrained with the size of memory.
- We have assumed 32 bit architecture, and we have used 32 bit instructions and number of registers are also 32.

- Forwarding is implemented to avoid data hazards where possible.
- Stalls and bubbles are inserted only when dependencies cannot be resolved through forwarding.
- Proper care was taken to handle jump and branch instructions, with jump resolution in the ID stage.

Edge Cases/Error analysis:
- We have taken care that register x0 is never written even if attempted to.
- We have included warning that get printed to the console for the following cases :
    1. Unknown opcode
    2. Unknown alu operation 
    3. Write to some register value which is not in 0 to 31. 
    In such cases, we ignore to perform any operations like register write or alu operation.

Testing :
- We have added test cases, which test all the stall cases, forwarding cases, and branch resolution cases. We have also added test cases for jump instructions.
- We have added test cases to edit the value in registers and checked the value stored in them.

Known Issues
- The simulator implementation in software, varies from hardware realisation. There are values which are computed in code but according to hardware we need to assume that they are not yet available and add corresponding stalls.


Sources Consulted
- RISC-V ISA Specification
- COL216 Lecture Notes
- Stack Overflow for C++ syntax and troubleshooting
- ChatGPT for debugging and conceptual clarifications

Contributors
- Shivankur Gupta – IIT Delhi
- Vanshika – IIT Delhi
- Project developed as part of COL216: Computer Architecture assignment.
