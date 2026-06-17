#include "Tokenscaner.h"
#include <iostream>

using namespace std;

int main() {
    string inputFile = "C:/Users/10497/Documents/Compliers/TestSet/calculateTest.txt";
    
    try {
        TokenAnalyzer analyzer(inputFile);
        Token token;
        
        cout << "词法分析结果：" << endl;
        while ((token = analyzer.scan()).type != TokenType::END) {
            cout << "Token: '" << token.lexeme << "' Type: ";
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
    } catch (const exception& e) {
        cout << "错误: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}