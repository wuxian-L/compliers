#include "SyntaxAnalyzer.h"
void SyntaxAnalyzer::parse()
{
    stmts();
}

void SyntaxAnalyzer::stmts()
{
    printProduction("stmts -> stmt rest0");
    stmt();
    rest0();
}

void SyntaxAnalyzer::rest0()
{
    currentToken = scanner.peekToken();
    if (currentToken.type == TokenType::KEYWORD || currentToken.type == TokenType::IDENTIFIED || currentToken.lexeme == "if" || currentToken.lexeme == "while")
    {
        printProduction("rest0 -> stmt rest0");
        stmt();
        rest0();
    }
    else
    {
        printProduction("rest0 -> ε");
    }
}

void SyntaxAnalyzer::stmt()
{
    currentToken = scanner.peekToken();
    if (currentToken.type == TokenType::IDENTIFIED)
    {
        printProduction("stmt -> loc = expr ;");
        loc();
        expect("=");
        expr();
        expect(";");
    }
    else if (currentToken.lexeme == "if")
    {
        printProduction("stmt -> if ( bool ) stmt else stmt");
        match("if");
        expect("(");
        bool_expr();
        expect(")");
        stmt();
        expect("else");
        stmt();
    }
    else if (currentToken.lexeme == "while")
    {
        printProduction("stmt -> while ( bool ) stmt");
        match("while");
        expect("(");
        bool_expr();
        expect(")");
        stmt();
    }
    else
    {
        throw runtime_error("语法错误");
    }
}

void SyntaxAnalyzer::loc()
{
    currentToken = scanner.peekToken();
    if (currentToken.type == TokenType::IDENTIFIED)
    {
        printProduction("loc -> id resta");
        match(TokenType::IDENTIFIED);
        resta();
    }
    else
    {
        throw runtime_error("语法错误");
    }
}

void SyntaxAnalyzer::resta()
{
    currentToken = scanner.peekToken();
    if (currentToken.lexeme == "[")
    {
        printProduction("resta -> [ elist ]");
        match("[");
        elist();
        expect("]");
    }
    else
    {
        printProduction("resta -> ε");
    }
}

void SyntaxAnalyzer::elist()
{
    printProduction("elist -> expr rest1");
    expr();
    rest1();
}

void SyntaxAnalyzer::rest1()
{
    currentToken = scanner.peekToken();
    if (currentToken.lexeme == ",")
    {
        printProduction("rest1 -> , expr rest1");
        match(",");
        expr();
        rest1();
    }
    else
    {
        printProduction("rest1 -> ε");
    }
}

void SyntaxAnalyzer::bool_expr()
{
    printProduction("bool -> equality");
    equality();
}

void SyntaxAnalyzer::equality()
{
    printProduction("equality -> rel rest4");
    rel();
    rest4();
}

void SyntaxAnalyzer::rest4()
{
    currentToken = scanner.peekToken();
    if (currentToken.lexeme == "==" || currentToken.lexeme == "!=")
    {
        printProduction("rest4 -> " + currentToken.lexeme + " rel rest4");
        match(currentToken.lexeme);
        rel();
        rest4();
    }
    else
    {
        printProduction("rest4 -> ε");
    }
}

void SyntaxAnalyzer::rel()
{
    printProduction("rel -> expr rop_expr");
    expr();
    rop_expr();
}

void SyntaxAnalyzer::rop_expr()
{
    currentToken = scanner.peekToken();
    if (currentToken.lexeme == "<" || currentToken.lexeme == "<=" || currentToken.lexeme == ">" || currentToken.lexeme == ">=")
    {
        printProduction("rop_expr -> " + currentToken.lexeme + " expr");
        match(currentToken.lexeme);
        expr();
    }
    else
    {
        printProduction("rop_expr -> ε");
    }
}

void SyntaxAnalyzer::expr()
{
    printProduction("expr -> term rest5");
    term();
    rest5();
}

void SyntaxAnalyzer::rest5()
{
    currentToken = scanner.peekToken();
    if (currentToken.lexeme == "+" || currentToken.lexeme == "-")
    {
        printProduction("rest5 -> " + currentToken.lexeme + " term rest5");
        match(currentToken.lexeme);
        term();
        rest5();
    }
    else
    {
        printProduction("rest5 -> ε");
    }
}

void SyntaxAnalyzer::term()
{
    printProduction("term -> unary rest6");
    unary();
    rest6();
}

void SyntaxAnalyzer::rest6()
{
    currentToken = scanner.peekToken();
    if (currentToken.lexeme == "*" || currentToken.lexeme == "/")
    {
        printProduction("rest6 -> " + currentToken.lexeme + " unary rest6");
        match(currentToken.lexeme);
        unary();
        rest6();
    }
    else
    {
        printProduction("rest6 -> ε");
    }
}

void SyntaxAnalyzer::unary()
{
    printProduction("unary -> factor");
    factor();
}

void SyntaxAnalyzer::factor()
{
    currentToken = scanner.peekToken();
    if (currentToken.type == TokenType::NUMBER)
    {
        printProduction("factor -> num");
        match(TokenType::NUMBER);
    }
    else if (currentToken.lexeme == "(")
    {
        printProduction("factor -> ( expr )");
        match("(");
        expr();
        expect(")");
    }
    else if (currentToken.type == TokenType::IDENTIFIED)
    {
        printProduction("factor -> loc");
        loc();
    }
    else
    {
        throw runtime_error("语法错误");
    }
}

bool SyntaxAnalyzer::match(const TokenType &tokenType)
{
    currentToken = scanner.scan();
    return currentToken.type == tokenType;
}

bool SyntaxAnalyzer::match(const string &token)
{
    currentToken = scanner.scan();
    return currentToken.lexeme == token;
}

void SyntaxAnalyzer::expect(const string &token)
{
    if (!match(token))
    {
        throw runtime_error("语法错误: 期望 " + token);
    }
}
/*
int main() {
    string inputFile = "c:/Users/10497/Documents/Compliers/TestSet/SyntaxAnalyzerTest.txt";
    string outputFile = "c:/Users/10497/Documents/Compliers/TestSet/SyntaxAnalyzerTestOutput.txt";
    
    try {
        TokenAnalyzer analyzer(inputFile);
        SyntaxAnalyzer syntax(analyzer, outputFile);
        
        syntax.parse();
        
        cout << "语法分析完成，结果已写入: " << outputFile << endl;
    } catch (const exception& e) {
        cout << "错误: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
//C:\Users\10497\Documents\Compliers\codefield\.vscode\codespace\SyntaxAnalyzer.exe
*/