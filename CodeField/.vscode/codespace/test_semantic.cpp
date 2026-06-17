#include "SemanticAnalyzer.h"
#include <iostream>

int main(int argc, char* argv[]) {
    string inputPath;
    string outputPath;
    
    if (argc == 3) {
        inputPath = argv[1];
        outputPath = argv[2];
    } else {
        inputPath = "C:\\Users\\10497\\Documents\\Compliers\\TestSet\\controlTest.txt";
        outputPath = "C:\\Users\\10497\\Documents\\Compliers\\TestSet\\controlTestOutput.txt";
    }
    
    std::cout << "Input path: " << inputPath << std::endl;
    std::cout << "Output path: " << outputPath << std::endl;
    
    try {
        TokenAnalyzer scanner(inputPath);
        SemanticAnalyzer analyzer(scanner, outputPath);
        analyzer.analyze();
        std::cout << "Analysis completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
