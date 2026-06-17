#include <fstream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <cctype>
#include <stdexcept>
#include <sys/stat.h>
using namespace std;

enum class TokenType
{
    KEYWORD,
    IDENTIFIED,
    NUMBER,
    STRING,
    OPERATOR,
    DELIMITED,
    END
};
/*
struct SymbolEntry
{
    string name;
    TokenType type;
    int first_occur_line;
    int first_occur_col;

    string data_type;
    int int_value;
    double double_value;
    string string_value;

    SymbolEntry() : int_value(0), double_value(0.0) {}
    SymbolEntry(const string &na, TokenType t, int fl, int fc)
        : name(na), type(t), first_occur_line(fl), first_occur_col(fc),
          int_value(0), double_value(0.0){}
};
*/
struct SymbolEntry
{
    string name;
    TokenType type;
    int line;
    int col;
    int scope; // 作用域层级

    bool isArray;  // 是否为数组
    int arraySize; // 数组大小（如果是数组）

    SymbolEntry() : scope(0), line(0), col(0), isArray(false), arraySize(0) {}
    SymbolEntry(const string &n, const TokenType &t, int s, int l)
        : name(n), type(t), scope(s), line(l), col(0), isArray(false), arraySize(0) {}
};

struct Token
{
    TokenType type;
    string lexeme;
    int line, column;
    int int_value;
    double double_value;
    string string_value;
    class SymbolEntry *symbol = nullptr;
    Token() : type(TokenType::END), line(0), column(0), int_value(0), double_value(0.0) {}
    Token(TokenType t) : type(t), line(0), column(0), int_value(0), double_value(0.0) {}
    Token(TokenType t, int ln, int col) : type(t), line(ln), column(col), int_value(0), double_value(0.0) {}
};

inline unordered_map<string, TokenType> keywordTable = {
    {"if", TokenType::KEYWORD},
    {"else", TokenType::KEYWORD},
    {"while", TokenType::KEYWORD},
    {"return", TokenType::KEYWORD},
    {"int", TokenType::KEYWORD},
    {"float", TokenType::KEYWORD}};

inline unordered_map<string, TokenType> operatorwordTable = {
    {"+", TokenType::OPERATOR},
    {"!", TokenType::OPERATOR},
    {"-", TokenType::OPERATOR},
    {"*", TokenType::OPERATOR},
    {"/", TokenType::OPERATOR},
    {"=", TokenType::OPERATOR},
    {"+=", TokenType::OPERATOR},
    {"-=", TokenType::OPERATOR},
    {"*=", TokenType::OPERATOR},
    {"/=", TokenType::OPERATOR},
    {"++", TokenType::OPERATOR},
    {"--", TokenType::OPERATOR},
    {"<", TokenType::OPERATOR},
    {">", TokenType::OPERATOR},
    {"<=", TokenType::OPERATOR},
    {">=", TokenType::OPERATOR},
    {"==", TokenType::OPERATOR},
    {"!=", TokenType::OPERATOR}};

inline unordered_map<string, TokenType> delimitedwordTable = {
    {";", TokenType::DELIMITED},
    {",", TokenType::DELIMITED},
    {"(", TokenType::DELIMITED},
    {")", TokenType::DELIMITED},
    {"{", TokenType::DELIMITED},
    {"}", TokenType::DELIMITED},
    {"[", TokenType::DELIMITED},
    {"]", TokenType::DELIMITED}};

class SymbolTable
{

public:
    unordered_map<string, SymbolEntry> table;
    SymbolEntry *insert(const string &name, const SymbolEntry &entry)
    {
        if (table.count(name))
        {
            return nullptr;
        }
        table[name] = entry;
        return &table[name];
    }
    SymbolEntry *lookup(const string &name)
    {
        if (table.count(name))
            return &table[name];
        cout << "该符号不存在" << endl;
        return nullptr;
    }
};

class TokenAnalyzer
{
private:
    string cpp_path;
    ifstream infile;
    char peek;

    Token peekedToken;
    bool hasPeeked;

    char nextChar()
    {
        char c;
        if (!infile.get(c))
            return EOF;
        if (c == '\n')
        {
            g_line++;
            g_col = 1;
        }
        else
            g_col++;
        return c;
    }

    bool fileExists(const string &path)
    {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }

public:
    SymbolTable table;
    static long long g_line;
    static int g_col;

    explicit TokenAnalyzer(const string &path) : cpp_path(path), infile(path, ios::in), hasPeeked(false), peekedToken()
    {
        g_line = 1;
        g_col = 1;
        if (!fileExists(cpp_path))
        {
            throw runtime_error("File not found");
        }
        if (!infile.is_open())
        {
            throw runtime_error("Failed to open file");
        }
        peek = nextChar();
    }

    Token getNumber();
    Token getAlpha();
    Token getDelimited();
    Token getOperator();

private:
    Token doScan()
    {
        while (isspace(peek))
            peek = nextChar();

        if (isdigit(peek))
        {
            return getNumber();
        }
        if (isalpha(peek) || peek == '_')
        {
            return getAlpha();
        }
        if (delimitedwordTable.count(string(1, peek)) != 0)
        {
            return getDelimited();
        }
        if (operatorwordTable.count(string(1, peek)) != 0)
        {
            return getOperator();
        }
        Token end_token;
        end_token.type = TokenType::END;
        return end_token;
    }

public:
    Token scan()
    {
        if (hasPeeked)
        {
            hasPeeked = false;
            return peekedToken;
        }
        return doScan();
    }
    Token peekToken()
    {
        if (!hasPeeked)
        {
            peekedToken = doScan();
            hasPeeked = true;
        }
        return peekedToken;
    }
};
