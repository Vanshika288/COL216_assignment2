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

void separate_columns(string filename){
    // Open input file
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "❌ Error: Could not open input file." << endl;
        return;
    }

    // Open output files
    ofstream firstColumnFile("machine_code.txt");
    ofstream secondColumnFile("assembly_code.txt");

    if (!firstColumnFile.is_open() || !secondColumnFile.is_open()) {
        cerr << "❌ Error: Could not create output files." << endl;
        return;
    }

    string line;
    while (getline(inputFile, line)) {
        stringstream ss(line);
        string firstColumn, secondColumn;

        // Extract first column and second column
        if (ss >> firstColumn) {
            getline(ss, secondColumn);  // Get remaining part as second column
            firstColumnFile << firstColumn << endl;
            secondColumnFile << secondColumn << endl;
        }
    }

    // Close all files
    inputFile.close();
    firstColumnFile.close();
    secondColumnFile.close();
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

    // Separate Columns of the input file
    separate_columns(filename);

    // Instantiate Pipeline
    bool forwardingEnabled = (std::string(argv[0]) == "./forward");
    Pipeline pipeline(forwardingEnabled);

    // Load instructions into the pipeline
    pipeline.loadInstructions("machine_code.txt");
    pipeline.load_string_instructions("assembly_code.txt");

    // Run the pipeline for the given number of cycles
    pipeline.runPipeline(cycleCount);

    // Print the final state of registers
    // std::cout << "\nFinal state of registers:\n";
    // pipeline.dumpRegisters();
    pipeline.printPipeline(cycleCount);
    return 0;
}
