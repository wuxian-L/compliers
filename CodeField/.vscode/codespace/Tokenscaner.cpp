#include "Tokenscaner.h"

long long TokenAnalyzer::g_line = 1;
int TokenAnalyzer::g_col = 1;

Token TokenAnalyzer::getNumber()
{
    string lex;
    double val = 0;
    float bitright = 1;
    bool Esign = false;
    int Enum = 0;

    Token retToken;
    retToken.type = TokenType::NUMBER;

    int state = 1;
    while (1)
    {
        switch (state)
        {
        case 1:
            val = val * 10 + (int)(peek - 48);
            lex += peek;
            peek = nextChar();
            if (peek == '.')
                state = 2;
            else if (isdigit(peek))
                state = 1;
            else if (peek == 'E')
                state = 4;
            else
                state = 8;
            break;
        case 2:
            lex += peek;
            peek = nextChar();
            if (isdigit(peek))
                state = 3;
            else
            {
                throw logic_error("数字格式错误");
                return Token{};
            }
            break;
        case 3:
            bitright *= 0.1;
            val = val + bitright * (int)(peek - 48);
            lex += peek;
            peek = nextChar();
            if (peek == 'E')
                state = 4;
            else if (isdigit(peek))
                state = 3;
            else
                state = 9;
            break;
        case 4:
            lex += peek;
            peek = nextChar();
            if (peek == '+' || peek == '-')
                state = 5;
            else if (isdigit(peek))
                state = 6;
            else
            {
                throw logic_error("数字格式错误");
                return Token{};
            }
            break;
        case 5:
            if (peek == '-')
                Esign = true;
            lex += peek;
            peek = nextChar();
            if (isdigit(peek))
                state = 6;
            else
            {
                throw logic_error("数字格式错误");
                return Token{};
            }
            break;
        case 6:
            Enum = Enum * 10 + (int)(peek - 48);
            lex += peek;
            peek = nextChar();
            if (isdigit(peek))
                state = 6;
            else
                state = 7;
            break;
        case 7:
            retToken.lexeme = lex;
            for (int i = 0; i < Enum; i++)
            {
                if (Esign)
                {
                    val /= 10;
                }
                else
                {
                    val *= 10;
                }
            }
            retToken.double_value = val;
            retToken.line = g_line;
            retToken.column = g_col;
            retToken.symbol = nullptr;
            return retToken;
            break;
        case 8:
            retToken.lexeme = lex;
            retToken.double_value = val;
            retToken.line = g_line;
            retToken.column = g_col;
            retToken.symbol = nullptr;
            return retToken;
            break;
        case 9:
            retToken.lexeme = lex;
            retToken.double_value = val;
            retToken.line = g_line;
            retToken.column = g_col;
            retToken.symbol = nullptr;
            return retToken;
            break;
        }
    }
}

Token TokenAnalyzer::getAlpha()
{
    string lex;
    lex += peek;

    while (isalnum(peek = nextChar()) || peek == '_')
    {
        lex += peek;
    }

    Token tok;
    tok.lexeme = lex;
    tok.line = g_line;
    tok.column = g_col;

    if (keywordTable.count(lex))
    {
        tok.type = TokenType::KEYWORD;
        tok.symbol = nullptr;
    }
    else
    {
        tok.type = TokenType::IDENTIFIED;
        tok.symbol = table.insert(lex, SymbolEntry(lex, tok.type, g_line, g_col));
    }
    return tok;
}

Token TokenAnalyzer::getDelimited()
{
    string lex;
    lex += peek;
    peek = nextChar();
    Token tok;
    tok.lexeme = lex;
    tok.line = g_line;
    tok.column = g_col;
    tok.symbol = nullptr;
    tok.type = TokenType::DELIMITED;
    return tok;
}

Token TokenAnalyzer::getOperator()
{
    int state = 0;
    string lex;
    Token tok;
    tok.type = TokenType::OPERATOR;
    while (1)
    {
        switch (state)
        {
        case 0:
            lex += peek;
            peek = nextChar();
            if (peek == '!' || peek == '*' || peek == '/' || peek == '=' || peek == '<' || peek == '>')
                state = 1;
            else if (peek == '+')
                state = 2;
            else if (peek == '-')
                state = 3;
            else
            {
                tok.lexeme = lex;
                tok.string_value = lex;
                tok.line = g_line;
                tok.column = g_col;
                tok.symbol = nullptr;
                return tok;
            }
            break;
        case 1:
            lex += peek;
            peek = nextChar();
            if (peek == '=')
                state = 4;
            else
            {
                tok.lexeme = lex;
                tok.string_value = lex;
                tok.line = g_line;
                tok.column = g_col;
                tok.symbol = nullptr;
                return tok;
            }
            break;
        case 2:
            lex += peek;
            peek = nextChar();
            if (peek == '+' || peek == '=')
                state = 4;
            else
            {
                tok.lexeme = lex;
                tok.string_value = lex;
                tok.line = g_line;
                tok.column = g_col;
                tok.symbol = nullptr;
                return tok;
            }
            break;
        case 3:
            lex += peek;
            peek = nextChar();
            if (peek == '-' || peek == '=')
                state = 4;
            else
            {
                tok.lexeme = lex;
                tok.string_value = lex;
                tok.line = g_line;
                tok.column = g_col;
                tok.symbol = nullptr;
                return tok;
            }
            break;
        case 4:
            lex += peek;
            peek = nextChar();
            tok.lexeme = lex;
            tok.string_value = lex;
            tok.line = g_line;
            tok.column = g_col;
            tok.symbol = nullptr;
            return tok;
            break;
        }
    }
    return tok;
}
/*
int main() {
    string inputFile = "c:/Users/10497/Documents/Compliers/TestSet/TokenAnalyzerTest.txt";
    string outputFile = "c:/Users/10497/Documents/Compliers/TestSet/TokenAnalyzerTestOutput.txt";
    
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
*/