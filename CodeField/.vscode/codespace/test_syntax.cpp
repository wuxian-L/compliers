#include "SyntaxAnalyzer.h"
#include <iostream>

int main() {
    string inputFile = "C:\\Users\\10497\\Documents\\Compliers\\TestSet\\SyntaxAnalyzerTest.txt";
    string outputFile = "C:\\Users\\10497\\Documents\\Compliers\\TestSet\\SyntaxAnalyzerTestOutput.txt";
    
    try {
        TokenAnalyzer scanner(inputFile);
        SyntaxAnalyzer analyzer(scanner, outputFile);
        analyzer.parse();
        cout << "语法分析完成，结果已写入: " << outputFile << endl;
    } catch (const exception& e) {
        cout << "错误: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
