#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "pipeline.hpp"

void printUsage() {
    std::cerr << "Usage: ./noforward <inputfile.txt> <cyclecount>\n";
    std::cerr << "Usage: ./forward <inputfile.txt> <cyclecount>\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage();
    }

    std::string filename = argv[1];
    int cycleCount;
    
    // Parse cycle count
    try {
        cycleCount = std::stoi(argv[2]);
    } catch (std::invalid_argument& e) {
        std::cerr << "Error: Invalid cycle count provided.\n";
        printUsage();
    }

    // Open the input file
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    // std::vector<std::string> instructions;
    // std::string line;
    
    // // Read instructions from the file
    // while (getline(inputFile, line)) {
    //     if (!line.empty()) {
    //         instructions.push_back(line);
    //     }
    // }

    // inputFile.close();

    // Instantiate Pipeline
    bool forwardingEnabled = (std::string(argv[0]).find("forward") != std::string::npos);
    Pipeline pipeline(forwardingEnabled);

    // Load instructions into the pipeline
    // pipeline.loadInstructions(instructions);
    pipeline.loadInstructions("inst.txt");
    pipeline.load_string_instructions("stringinst.txt");
    // Run the pipeline for the given number of cycles
    // for (int i = 0; i < cycleCount; ++i) {
    //     pipeline.runPipeline(i + 1);
    // }

    pipeline.runPipeline(cycleCount);
    // Print the final state of registers
    std::cout << "\nFinal state of registers:\n";
    pipeline.dumpRegisters();
    // for (int i=0;i<pipeline) 
    pipeline.printPipeline();
    return 0;
}
