✅ Usage:
Compile the executables:
# Compile both versions
make

Run Non-Forwarding Version:
./noforward ../inputfiles/strlen.txt 20

Run Forwarding Version:
./forward ../inputfiles/strlen.txt 20

Clean the project:
make clean

______________________________________________________________________________________________________________________

Directory Structure:

📦 riscv-simulator
├── 📂 src
│   ├── main.cpp
│   ├── pipeline.cpp
│   ├── pipeline.hpp
│   ├── control.cpp
│   ├── control.hpp
│   ├── registers.cpp
│   ├── registers.hpp
│   ├── alu.cpp
│   ├── alu.hpp
│   ├── Makefile
└── 📂 inputfiles
    ├── strlen.txt
    ├── stringcopy.txt
    └── strncpy.txt

_________________________________________________________________________________

🎯 Usage
To build both executables:
make
To build only noforward:
make noforward
To build only forward:
make forward
To clean up:
make clean


....................................................................................

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
└── temp.txt            # Temporary file (ignored by Git)

Installation and Build Instructions
1. Clone the Repository
git clone https://github.com/your-username/RISCV-Pipeline.git
cd RISCV-Pipeline/src

2. Build Using Makefile
# To build the version with forwarding
make forward

# To build the version without forwarding
make noforward

3. Run the Executables
# Run the simulator with forwarding
./forward

# Run the simulator without forwarding
./noforward

Usage Instructions
1. Place the assembly instructions in assembly_code.txt in the following format:
addi x10, x0, 4
addi x6, x0, 10
sw x6, 8(x10)
lw x5, 8(x10)
add x7, x5, x5

2. Run the respective executable to generate the machine code in machine_code.txt.

3. Pipeline Visualization:
- The pipeline stages are displayed cycle-by-cycle for easy verification.

Test Cases
Example Test Case:
addi x10, x0, 4
addi x6, x0, 10
sw x6, 8(x10)
lw x5, 8(x10)
beq x5, x6, 8
addi x5, x5, 1
jal x0, -16

Machine Code Output:
00400513
00a00313
00652423
00852283
00b28263
00128293
ff1ff06f

Pipeline Execution Diagram
Instruction       | IF | ID | EX | MEM | WB
------------------|----|----|----|-----|---
addi x10, x0, 4   | 1  | 2  | 3  | 4   | 5
addi x6, x0, 10   |    | 3  | 4  | 5   | 6
sw x6, 8(x10)     |    |    | 5  | 6   | -
lw x5, 8(x10)     |    |    |    | 7   | 8
beq x5, x6, 8     |    |    |    |     | -
addi x5, x5, 1    |    |    |    |     | -
jal x0, -16       |    |    |    |     | -

Makefile Targets
- make forward – Build the pipeline simulator with forwarding.
- make noforward – Build the pipeline simulator without forwarding.
- make clean – Remove all generated .o files and executables.

Contributors
- Shivankur Gupta – IIT Delhi
- Project developed as part of COL216: Computer Architecture assignment.

Contact
If you encounter any issues or have any suggestions, feel free to open an issue or contact me via email.
