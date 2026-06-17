#include "LLSyntaxAnalyzer.h"

int main() {
    std::string inputFile = "c:/Users/10497/Documents/Compliers/TestSet/SyntaxAnalyzerTest.txt";
    
    try {
        TokenAnalyzer analyzer(inputFile);
        LLSyntaxAnalyzer syntax(analyzer);
        
        std::cout << "LL(1)语法分析器初始化完成" << std::endl;
        std::cout << "开始分析..." << std::endl;
        
        syntax.parse();
        
        std::cout << "分析完成！结果已写入 SyntaxAnalysisOutput.txt" << std::endl;
    } catch (const std::exception &e) {
        std::cout << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}