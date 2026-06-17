#include "Tokenscaner.h"
#include <iostream>

int main() {
    string inputFile = "C:\\Users\\10497\\Documents\\Compliers\\TestSet\\TokenAnalyzerTest.txt";
    string outputFile = "C:\\Users\\10497\\Documents\\Compliers\\TestSet\\TokenAnalyzerTestOutput.txt";
    
    ofstream out(outputFile);
    if (!out.is_open()) {
        cout << "无法打开输出文件" << endl;
        return 1;
    }
    
    try {
        TokenAnalyzer analyzer(inputFile);
        Token token;
        
        out << "输出单词：" << endl;
        
        while ((token = analyzer.scan()).type != TokenType::END) {
            out << "< " << token.lexeme << ", ";
            
            switch (token.type) {
                case TokenType::KEYWORD:
                    out << "keyword";
                    break;
                case TokenType::IDENTIFIED:
                    out << "id";
                    break;
                case TokenType::NUMBER:
                    out << "num";
                    break;
                case TokenType::OPERATOR:
                    out << "op";
                    break;
                case TokenType::DELIMITED:
                    out << "delim";
                    break;
                default:
                    out << "-";
            }
            
            out << " >" << endl;
        }
        
        out.close();
        cout << "词法分析完成，结果已写入: " << outputFile << endl;
    } catch (const exception& e) {
        out << "错误: " << e.what() << endl;
        cout << "错误: " << e.what() << endl;
        out.close();
        return 1;
    }
    
    return 0;
}
