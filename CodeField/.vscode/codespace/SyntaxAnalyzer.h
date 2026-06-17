#include "Tokenscaner.h"
#include <fstream>
#include <string>

using namespace std;

class SyntaxAnalyzer{
private:
    TokenAnalyzer &scanner;
    Token currentToken;
    ofstream outputFile;
    int productionCount;
    void printProduction(const string &production)
    {
        outputFile << "(" << productionCount++ << ")" << production << endl;
    }
public:
    SyntaxAnalyzer(TokenAnalyzer &s) : scanner(s), productionCount(1) {}
    SyntaxAnalyzer(TokenAnalyzer &s, const string &outputPath) : scanner(s), productionCount(1) {
        outputFile.open(outputPath);
    }
    ~SyntaxAnalyzer() {
        if (outputFile.is_open()) {
            outputFile.close();
        }
    }
    void parse();
    void stmts();
    void rest0();
    void stmt();
    void loc();
    void resta();
    void elist();
    void rest1();
    void bool_expr();
    void equality();
    void rest4();
    void rel();
    void rop_expr();
    void expr();
    void rest5();
    void term();
    void rest6();
    void unary();
    void factor();
    bool match(const TokenType &tokenType);
    bool match(const string &token);
    void expect(const string &token);
};