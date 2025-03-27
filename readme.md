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
  - Arithmetic: add, addi
  - Memory: lw, sw
  - Branches: beq, bne, bge, blt
  - Jumps: jal, jalr
  - More: aupic

- Implements hazard detection and inserts bubbles when required. We have put appropriate stalls and forwarded values properly wherever required.

- We have initialised all the register values to 0, and we are updating the register values as we proceed, since we are doing the calculations necessary in the ALU stage. We have simulated the processor completed with all the functionalities.

- Branches: We are resolving branches in the ID stage by taking care of the register values i.e, we have not always assumed branch not taken, but are resolving branches according to the register values. For ex, in beq, we will take the branch if the 2 register values are equal, and not take the branch otherwise.

- We have printed the pipeline in the end. We can also print the register values if required as a debug meeasure.

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
$./forward ../inputfiles/filename.txt cyclecount

# Run the simulator without forwarding
$./noforward ../inputfiles/filename.txt cyclecount


Makefile Targets
- make forward – Build the pipeline simulator with forwarding.
- make noforward – Build the pipeline simulator without forwarding.
- make clean – Remove all generated .o files and executables.

Contributors
- Shivankur Gupta – IIT Delhi
- Vanshika - IIT Delhi
- Project developed as part of COL216: Computer Architecture assignment.

