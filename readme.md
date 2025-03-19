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
