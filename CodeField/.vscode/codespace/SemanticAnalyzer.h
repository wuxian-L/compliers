#pragma once

#include "Tokenscaner.h"
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

/*
控制流语句翻译
stmts⟶ stmt rest0
        {
            rest0.inNextlist = stmt.nextlist;
            stmts.nextlist = rest0.nextlist;
        }
rest0 ⟶ M stmt rest01
        {
            backpatch(rest0.inNextlist, M.quad);
            rest0.inNextlist = stmt.nextlist;
            rest0.nextlist = rest01.nextlist;
        }
        | ℇ
        {
            rest0.nextlist = rest0.inNextlist
        }
stmt ⟶ loc = expr;
        {
        if(loc.offset == null)
            emit(=, expr.place, -, loc.place);
        else
            emit([]=, expr.place, -, loc.place[loc.offset]);
        stmt.nextlist = makeList();
        }
        | if(bool_expr) M stmt
        {
            backpatch(bool_expr.truelist, M.quad);
            S.nextlist = merge(bool_expr.falselist, stmt.nextlist);
        }
        | if(bool_expr) M1 stmt1 N else M2 stmt2
        {
            backpatch(bool_expr.truelist, m1.quad);
            backpatch(bool_expr.falselist, m2.quad);
            temp = merge(stmt1.nextlist, N.nextlist);
            s.nextList = merge(stmt2.nextlist, temp);
        }
        | while M1 (bool_expr) M2 stmt1
        {
            backpatch(stmt1.nextlist, M1.quad);
            backpatch(bool_expr.truelist, M2.quad);
            stmt.nextlist = bool_expr.falselist;
            emit(j, -, -, M1.quad);
        }
        | {stmts}
        {
            S.nextlist = stmts.nextlist;
        }
        | auto_stmt
        {
            S.nextlist = makeList();
        }
M ⟶ ℇ
    {
        M.quad = nextquad()
    }
N ⟶ ℇ
    {
        N.quad = makelist(nextquad())
        emit(j, -, -, 0)}
    }
// 类型和数组表达式翻译
loc⟶ id resta
        {
            resta.inArray = id.place
            loc.place = resta.place
            loc.offset = resta.offset
        }
resta⟶ [elist]
    {
            elist.inArray = resta.inArray
            resta.place = newtemp()
            emit(-, elist.arry, C, resta.place)
            resta.offset = newtemp()
            emit(*, w, elist.offset, resta.offset)
        }
        | ℇ
        {
            resta.place = resta.inArray
            resta.offset = null
        }
elist ⟶ expr rest1
    {
        rest1.inArray=elist.inArray;
        rest1.inNdim=1;
        rest1.inPlace=expr.place}
        {elist.array=rest1.array;
        elist.offset=rest1.offset}
    }
rest1⟶ , expr rest1
    {
        t=newtemp();
        m=rest1.inNdim+1;
        emit(‘*,’ rest1.inPlace	‘,’ limit(rest1.inarray,m)	‘,’ t);
        emit(‘+,’ t	‘,’ expr.place	‘,’ t);
        rest11.inArray=rest1.inArray;
        rest11.inNdim=m;
        rest11.inPlace=t
    }
        | ℇ
    {
        rest1.array=rest1.inArray;
        rest1.offset=rest1.inPlace
    }

布尔表达式翻译
bool_expr ⟶ bool_value bool_rest
            {}
            |equality
            {
                bool.truelist=equality.truelist
                bool.falselist=equality.falselist
            }
bool_value ⟶ !bool_value1
            {
                bool_value.truelist = bool_value1.falselist;
                bool_value.falselist = bool_value1.truelist;
            }
            | (bool_expr)
            {
                bool_value.truelist = bool_expr.truelist;
                bool_value.falselist = bool_expr.falselist;
            }
            | true
            {
                bool_value.truelist = makelist(nextquad());
                emit("j", "-", "-", "-");
            }
            | false
            {
                bool_value.falselist = makelist(nextquad());
                emit("j", "-", "-", "-");
            }
bool_rest ⟶ || M bool_value bool_rest
           | && M bool_value bool_rest
           | ℇ

equality ⟶rel rest4
        {
            rest4.inTruelist=rel.truelist
            rest4.inFalselist=rel.falselist
            equality.truelist=rest4.truelist
            equality.falselist=rest4.falselist
        }
rest4 ⟶ ==rel rest4 |
         !=rel rest4 |
          ℇ
        {
            rest4.truelist=rest4.inTruelist
            rest4.falselist=rest4.inFalselist
        }
rel ⟶expr rop_expr
    {
        rop_expr.inPlace=expr.place
        rel.truelist=rop_expr.truelist
        rel.falselist=rop_expr.falselist
    }
rop_expr ⟶ <expr
    {
        rop_expr.truelist=makelist(nextquad);
        rop_expr.falselist=makelist(nextquad+1);
        emit(‘j<,’	rop_expr.inPlace	‘,’ expr.place	‘,	-’);
        emit(‘j,	-,	-,	-’)
    }
        | <=expr
    {
        rop_expr.truelist=makelist(nextquad);
        rop_expr.falselist=makelist(nextquad+1);
        emit(‘j<=,’	rop_expr.inPlace	‘,’	expr.place	‘,	-’);
        emit(‘j,	-,	-,	-’)
    }
        | >expr
    {
        rop_expr.truelist=makelist(nextquad);
        rop_expr.falselist=makelist(nextquad+1);
        emit(‘j>,’	rop_expr.inPlace	‘,’	expr.place	‘,	-’);
        emit(‘j,	-,	-,	-’)
    }
        | >=expr
    {
        rop_expr.truelist=makelist(nextquad);
        rop_expr.falselist=makelist(nextquad+1);
        emit(‘j>=,’	rop_expr.inPlace	‘,’	expr.place	‘,	-’);
        emit(‘j,	-,	-,	-’)
    }
        | ℇ
    {
        rop_expr.truelist=makelist(nextquad);
        rop_expr.falselist=makelist(nextquad+1);
        emit(‘jnz,’ rop_expr.inPlace	‘,	-,	-’);
        emit(‘j,	-,	-,	-’)
    }

算术表达式翻译
//  term.place是term的结果临时变量，
    rest5.in继承属性，传递给rest5,
    rest5.place综合属性，是rest5的结果临时变量，最终传递给expr
    expr.place是expr的结果临时变量,整个表达式最终的结果
expr ⟶ term rest5
        {
            rest5.in = term.place
            expr.place = rest5.place
        }
rest5⟶ +term rest51 |
        {
            rest51.place = newtemp()
            emit(+, rest5.in, term.place, rest51.in)
            rest5.place = rest51.place
        }
        -term rest51 |
        {
            rest51.in = newtemp()
            emit(-, rest5.in, term.place, rest51.in)
            rest5.place = rest51.place
        }
         ℇ
        {
            rest5.in = rest5.in
        }
term ⟶ unary rest6
        {rest6.in	=	unary.place}
        {term.place	=	rest6.place}
rest6⟶ * unary rest61 |
        {rest61.in=newtemp();
        emit(‘*,’ rest6.in	‘,’ unary.place	‘,’ rest61.in)}
        {rest6.place	=	rest61 .place}
         / unary rest61 |
        {rest61.in=newtemp();
        emit(‘/,’	rest6.in	‘,’	unary.place	‘,’	rest61.in)}
        {rest6.place	=	rest61 .place}
        ℇ
        {rest6.place = rest6.in}
unary ⟶factor
        {
            unary.place	=	factor.place
        }
factor ⟶ (expr)
        {
            factor.place	=	expr.place
        }
         | loc
        {
            if(loc.offset=null)
                factor.place = loc.place
            else {
                factor.place=newtemp();
                emit(‘=[],’ loc.place	‘[’ loc.offset	‘]’ ‘,	-,’ factor.place)
                }
            }
        | num
        {
            factor.place	=	num.value
        }
*/
using namespace std;
struct Exp
{
    string place;
};
struct Rest5
{
    string in;
    string place;
};

struct Quadruple
{ // 四元式结构体
    string op;
    string arg1;
    string arg2;
    string result;
    int label = -1; // 用于标记四元式的位置，便于回填
    Quadruple(const string &o, const string &a1, const string &a2, const string &res)
        : op(o), arg1(a1), arg2(a2), result(res) {}
};
// 四元式列表,提供生成四元式和访问四元式的接口
class QuadList
{
private:
    vector<Quadruple> quads;

public:
    int nextQuad() const
    {
        return quads.size();
    }
    void emit(const string &op, const string &arg1, const string &arg2, const string &result)
    {
        quads.emplace_back(op, arg1, arg2, result);
    }
    void emit(const Quadruple &quad)
    {
        quads.push_back(quad);
    }
    Quadruple &operator[](size_t index)
    {
        return quads[index];
    }
    int size() const
    {
        return quads.size();
    }
    void backpatch(int list, int target)
    {
        while (list != -1)
        {
            int next = quads[list].label;
            quads[list].result = to_string(target);
            list = next;
        }
    }

};
struct SymbolEntry;
// 重载<<
inline ostream &operator<<(ostream &os, const TokenType &type)
{
    switch (type)
    {
    case TokenType::KEYWORD:
        os << "KEYWORD";
        break;
    case TokenType::IDENTIFIED:
        os << "IDENTIFIED";
        break;
    case TokenType::NUMBER:
        os << "NUMBER";
        break;
    case TokenType::STRING:
        os << "STRING";
        break;
    case TokenType::OPERATOR:
        os << "OPERATOR";
        break;
    case TokenType::DELIMITED:
        os << "DELIMITED";
        break;
    case TokenType::END:
        os << "END";
        break;
    }
    return os;
}

inline ostream &operator<<(ostream &os, const SymbolEntry &entry)
{
    os << entry.name << "\t" << entry.type
       << "\t" << entry.scope << "\t" << entry.line
       << "\t" << entry.col;
    return os;
}

inline ostream &operator<<(ostream &os, const Quadruple &quad)
{
    os << "(" << quad.op << ", " << quad.arg1 << ", " << quad.arg2 << ", " << quad.result << ")";
    return os;
}

class SemanticAnalyzer
{
private:
    TokenAnalyzer &scanner;
    Token currentToken;
    QuadList quadList;
    int tempCounter = 1;
    int currentScope; // 当前作用域层级,全局为0，栈深入时加1，栈出时减1
    ofstream outputFile;

    string newtemp()
    {
        string ret = "t" + to_string(tempCounter++);
        insertSymbol(ret, TokenType::IDENTIFIED, currentToken.line);
        return ret;
    }

    void emit(const string &op, const string &arg1, const string &arg2, const string &result)
    {
        quadList.emit(op, arg1, arg2, result);
        if (outputFile.is_open())
        {
            outputFile << "(" << op << ", " << arg1 << ", " << arg2 << ", " << result << ")" << endl;
        }
    }

    void emit(const Quadruple &quad)
    {
        quadList.emit(quad);
        if (outputFile.is_open())
        {
            outputFile << "(" << quad.op << ", " << quad.arg1 << ", " << quad.arg2 << ", " << quad.result << ")" << endl;
        }
    }

    void insertSymbol(const string &name, const TokenType &type, int line)
    {
        if (scanner.table.lookup(name) == nullptr)
        {
            scanner.table.insert(name, SymbolEntry(name, type, currentScope, line));
        }
        else
        {
            throw runtime_error("符号 " + name + " 已经存在");
        }
    }

    int makelist(int quad)
    {
        return quad;
    }

    int nextquad()
    {
        return quadList.nextQuad();
    }

    void backpatch(int list, int target)
    {
        quadList.backpatch(list, target);
    }

    int merge(int list1, int list2)
    {
        if (list1 == -1) return list2;
        if (list2 == -1) return list1;
        quadList[list2].label = list1;
        return list2;
    }

    SymbolEntry *lookupSymbol(const string &name)
    {
        return scanner.table.lookup(name);
    }
    void printSymbolTable()
    {
        if (outputFile.is_open())
        {
            outputFile << "\n=== 符号表 ===" << endl;
            outputFile << "名称\t类型\t作用域\t行号\t列号" << endl;
            for (const auto &pair : getSymbolTable().table)
            {
                outputFile << pair.second << endl;
            }
        }
    }

    void printQuadruples()
    {
        if (outputFile.is_open())
        {
            outputFile << "\n=== 四元式 ===" << endl;
            for (size_t i = 0; i < quadList.nextQuad(); i++)
            {
                outputFile << i << ":" << quadList[i] << endl;
            }
        }
    }
    struct CalculateNode
    {
        string in;
        string place; // 计算结果存储的临时变量
    };
    struct arrayNode
    {
        std::string inArray = "";
        int inNdim = 0;
        std::string inPlace = "";

        std::string array = "";
        std::string offset = "null";
        std::string place = "";
    };
    struct boolExprNode
    {
        int truelist;
        int falselist;
    };
    struct controlNode
    {
        int nextlist;
    };
public:
    SemanticAnalyzer(TokenAnalyzer &s) : scanner(s), tempCounter(0), currentScope(0) {}
    SemanticAnalyzer(TokenAnalyzer &s, const string &outputPath) : scanner(s), tempCounter(0), currentScope(0)
    {
        outputFile.open(outputPath);
    }
    ~SemanticAnalyzer()
    {
        if (outputFile.is_open())
        {
            outputFile.close();
        }
    }
    SymbolTable getSymbolTable() const
    {
        return scanner.table;
    }

    void analyze()
    {
        if (outputFile.is_open())
        {
            outputFile << "=== 语义分析开始 ===" << endl;
        }
        currentToken = scanner.scan();// 初始化currentToken
        stmts();
        printSymbolTable();
        printQuadruples();
        if (outputFile.is_open())
        {
            outputFile << "\n=== 语义分析完成 ===" << endl;
        }
    }

private:
    bool match(const TokenType &tokenType)
    {
        bool ret = currentToken.type == tokenType;
        currentToken = scanner.scan();
        return ret;
    }

    bool match(const string &token)
    {
        bool result = currentToken.lexeme == token;
        currentToken = scanner.scan();
        return result;
    }

    void expect(const string &token)
    {
        if (currentToken.lexeme != token)
            throw runtime_error("语法错误: 期望 " + token);
        currentToken = scanner.scan();
    }
    // 控制流语句翻译
    controlNode stmts();
    controlNode rest0(controlNode stmtNode);
    controlNode stmt();
    int M();
    int N();
    // 类型和数组表达式翻译
    arrayNode loc();
    arrayNode resta(string inArray);
    arrayNode elist(string inArray);
    arrayNode rest1(arrayNode elistNode);
    // 布尔表达式翻译
    boolExprNode bool_expr();
    boolExprNode bool_value();
    boolExprNode bool_rest(boolExprNode left);
    boolExprNode equality();
    boolExprNode rest4(boolExprNode relNode);
    boolExprNode rel();
    boolExprNode rop_expr(string arg);
    // 算术表达式翻译
    string expr();
    string rest5(string inPlace);
    string term();
    string rest6(string inPlace);
    string unary();
    string factor();
};