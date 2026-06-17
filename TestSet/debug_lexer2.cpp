#include "Tokenscaner.h"
#include <iostream>

using namespace std;

int main() {
    string inputFile = "C:/Users/10497/Documents/Compliers/TestSet/SyntaxAnalyzerTest.txt";
    
    cout << "输入文件: " << inputFile << endl;
    
    try {
        TokenAnalyzer analyzer(inputFile);
        cout << "词法分析器初始化成功" << endl;
        
        // 检查peek字符
        cout << "第一个字符: '" << analyzer.peek << "' (ASCII: " << (int)analyzer.peek << ")" << endl;
        
        Token token;
        
        cout << "词法分析结果：" << endl;
        int count = 0;
        while ((token = analyzer.scan()).type != TokenType::END) {
            cout << "Token " << count++ << ": '" << token.lexeme << "' Type: ";
            switch (token.type) {
                case TokenType::KEYWORD: cout << "KEYWORD"; break;
                case TokenType::IDENTIFIED: cout << "IDENTIFIED"; break;
                case TokenType::NUMBER: cout << "NUMBER"; break;
                case TokenType::OPERATOR: cout << "OPERATOR"; break;
                case TokenType::DELIMITED: cout << "DELIMITED"; break;
                default: cout << "UNKNOWN";
            }
            cout << endl;
        }
        cout << "扫描完成，共 " << count << " 个token" << endl;
    } catch (const exception& e) {
        cout << "错误: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}