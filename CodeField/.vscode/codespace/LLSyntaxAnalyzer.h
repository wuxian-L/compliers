#include "Tokenscaner.h"
#include <fstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <algorithm>

/*
LL(1)语法分析器 - 基于预测分析表的自顶向下语法分析

文法规则定义：
stmts ⟶ stmt rest0                    语句序列
rest0 ⟶ stmt rest0 | ε                语句序列的递归部分（可空）
stmt ⟶ loc = expr;                    赋值语句
      | if(bool) stmt else stmt        if-else语句
      | while(bool) stmt               while循环语句
loc ⟶ id resta                        位置（标识符或数组元素）
resta ⟶ [elist] | ε                   数组下标（可空）
elist ⟶ expr rest1                    表达式列表
rest1 ⟶ , expr rest1 | ε              表达式列表的递归部分（可空）
bool ⟶ equality                       布尔表达式
equality ⟶ rel rest4                  相等性表达式
rest4 ⟶ == rel rest4 | != rel rest4 | ε 相等运算符（可空）
rel ⟶ expr rop_expr                   关系表达式
rop_expr ⟶ < expr | <= expr | > expr | >= expr | ε 关系运算符（可空）
expr ⟶ term rest5                     算术表达式
rest5 ⟶ + term rest5 | - term rest5 | ε 加法运算符（可空）
term ⟶ unary rest6                    项
rest6 ⟶ * unary rest6 | / unary rest6 | ε 乘法运算符（可空）
unary ⟶ factor                        一元表达式
factor ⟶ (expr) | loc | num           因子（括号表达式、位置或数字）
*/

using Production = std::vector<std::string>;

class LLSyntaxAnalyzer
{
private:
    TokenAnalyzer &scanner;
    Token currentToken;
    
    // 非终结符集合
    std::unordered_set<std::string> nonTerminals = {
        "stmts", "rest0", "stmt", "loc", "resta", "elist", "rest1", 
        "bool", "equality", "rest4", "rel", "rop_expr", "expr", 
        "rest5", "term", "rest6", "unary", "factor"
    };
    
    // 终结符集合
    std::unordered_set<std::string> terminals = {
        "if", "else", "while", "id", "num", "=", "==", "!=", "<", "<=", 
        ">", ">=", "+", "-", "*", "/", "(", ")", "[", "]", ",", ";", "$"
    };
    
    // 产生式集合 - 修复后的数据结构：每个产生式右部是符号列表
    std::unordered_map<std::string, std::vector<Production>> productionMap = {
        {"stmts", { {"stmt", "rest0"} }},
        {"rest0", { {"stmt", "rest0"}, {} }},  // {} 表示 ε
        {"stmt", { 
            {"loc", "=", "expr", ";"}, 
            {"if", "(", "bool", ")", "stmt", "else", "stmt"}, 
            {"while", "(", "bool", ")", "stmt"} 
        }},
        {"loc", { {"id", "resta"} }},
        {"resta", { {"[", "elist", "]"}, {} }},
        {"elist", { {"expr", "rest1"} }},
        {"rest1", { {",", "expr", "rest1"}, {} }},
        {"bool", { {"equality"} }},
        {"equality", { {"rel", "rest4"} }},
        {"rest4", { {"==", "rel", "rest4"}, {"!=", "rel", "rest4"}, {} }},
        {"rel", { {"expr", "rop_expr"} }},
        {"rop_expr", { {"<", "expr"}, {"<=", "expr"}, {">", "expr"}, {">=", "expr"}, {} }},
        {"expr", { {"term", "rest5"} }},
        {"rest5", { {"+", "term", "rest5"}, {"-", "term", "rest5"}, {} }},
        {"term", { {"unary", "rest6"} }},
        {"rest6", { {"*", "unary", "rest6"}, {"/", "unary", "rest6"}, {} }},
        {"unary", { {"factor"} }},
        {"factor", { {"(", "expr", ")"}, {"loc"}, {"num"} }}
    };
    
    std::unordered_map<std::string, std::unordered_set<std::string>> firstSet;  // FIRST集
    std::unordered_map<std::string, std::unordered_set<std::string>> followSet; // FOLLOW集
    std::unordered_map<std::string, std::unordered_map<std::string, Production>> parseTable; // 预测分析表
    std::ofstream outputFile;
    //添加注解，提高可读性
    /**
     * @brief 判断符号是否为终结符
     * @param symbol 待判断的符号
     * @return 如果是终结符返回true，否则返回false
     */
    bool isTerminal(const std::string &symbol) {
        return terminals.count(symbol) > 0;
    }
    
    /**
     * @brief 判断符号是否为非终结符
     * @param symbol 待判断的符号
     * @return 如果是非终结符返回true，否则返回false
     */
    bool isNonTerminal(const std::string &symbol) {
        return nonTerminals.count(symbol) > 0;
    }
    
    /**
     * @brief 判断产生式右部是否为空（ε）
     * @param production 产生式右部
     * @return 如果为空产生式返回true，否则返回false
     */
    bool isEpsilon(const Production &production) {
        return production.empty();
    }
    
    /**
     * @brief 计算单个符号串的FIRST集
     * 
     * FIRST(α)定义：从α推导出的所有串的首终结符集合
     * 
     * 计算规则：
     * 1. 如果α = ε，则FIRST(α) = {ε}
     * 2. 如果α = X1X2...Xn：
     *    - 将FIRST(X1)中除ε外的所有符号加入FIRST(α)
     *    - 如果ε ∈ FIRST(X1)，将FIRST(X2)中除ε外的所有符号加入FIRST(α)
     *    - 依此类推，直到某个Xi不能推导出ε
     *    - 如果所有Xi都能推导出ε，则ε ∈ FIRST(α)
     * 
     * @param symbols 符号序列
     * @return 该符号序列的FIRST集
     */
    std::unordered_set<std::string> computeFirstOfString(const std::vector<std::string> &symbols) {
        std::unordered_set<std::string> result;
        
        if (symbols.empty()) {
            result.insert("ε");  // ε的FIRST集就是{ε}
            return result;
        }
        
        bool allCanEpsilon = true;
        for (const std::string &symbol : symbols) {
            if (isTerminal(symbol)) {
                // 终结符的FIRST集就是它自己
                result.insert(symbol);
                allCanEpsilon = false;
                break;
            } else {
                // 非终结符，获取其FIRST集
                const auto &first = firstSet[symbol];
                for (const std::string &s : first) {
                    if (s != "ε") {
                        result.insert(s);
                    }
                }
                // 检查该非终结符是否能推导出ε
                if (first.count("ε") == 0) {
                    allCanEpsilon = false;
                    break;
                }
            }
        }
        
        // 如果所有符号都能推导出ε，则ε也在FIRST集中
        if (allCanEpsilon) {
            result.insert("ε");
        }
        
        return result;
    }
    
    /**
     * @brief 计算所有非终结符的FIRST集
     * 
     * FIRST(A)定义：从A推导出的所有串的首终结符集合
     * 
     * 迭代算法：
     * 1. 初始化：对于每个非终结符A，FIRST(A) = {}
     * 2. 反复应用以下规则直到不再变化：
     *    a. 如果有产生式A → α，则将FIRST(α)中除ε外的所有符号加入FIRST(A)
     *    b. 如果对于产生式A → α，ε ∈ FIRST(α)，则将ε加入FIRST(A)
     * 
     * @return 所有非终结符的FIRST集映射
     */
    std::unordered_map<std::string, std::unordered_set<std::string>> computeFirstSet() {
        // 直接使用成员变量 firstSet，而不是创建局部变量
        for (const std::string &nt : nonTerminals) {
            firstSet[nt] = std::unordered_set<std::string>();
        }
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto &entry : productionMap) {
                const std::string &nonTerminal = entry.first;
                const std::vector<Production> &productions = entry.second;
                
                for (const Production &prod : productions) {
                    // 计算当前产生式右部的FIRST集
                    std::unordered_set<std::string> firstOfProd = computeFirstOfString(prod);
                    
                    // 将FIRST(α)中的符号加入FIRST(A)
                    for (const std::string &symbol : firstOfProd) {
                        if (firstSet[nonTerminal].count(symbol) == 0) {
                            firstSet[nonTerminal].insert(symbol);
                            changed = true;
                        }
                    }
                }
            }
        }
        
        // 打印FIRST集
        outputFile << "=== FIRST集 ===" << std::endl;
        for (const auto &entry : firstSet) {
            outputFile << "FIRST(" << entry.first << ") = { ";
            for (const std::string &symbol : entry.second) {
                outputFile << symbol << " ";
            }
            outputFile << "}" << std::endl;
        }
        outputFile << std::endl;
        
        return firstSet;
    }
    
    /**
     * @brief 计算所有非终结符的FOLLOW集
     * 
     * FOLLOW(A)定义：在某些句型中紧跟在A右边的终结符的集合
     * 
     * 计算规则：
     * 1. 对于开始符号S，将$加入FOLLOW(S)
     * 2. 如果有产生式A → αBβ，则将FIRST(β)中除ε外的所有符号加入FOLLOW(B)
     * 3. 如果有产生式A → αB或A → αBβ且ε ∈ FIRST(β)，则将FOLLOW(A)中的所有符号加入FOLLOW(B)
     * 
     * @return 所有非终结符的FOLLOW集映射
     */
    std::unordered_map<std::string, std::unordered_set<std::string>> computeFollowSet() {
        // 直接使用成员变量 followSet，而不是创建局部变量
        for (const std::string &nt : nonTerminals) {
            followSet[nt] = std::unordered_set<std::string>();
        }
        
        // 规则1：开始符号stmts的FOLLOW集包含$
        followSet["stmts"].insert("$");
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto &entry : productionMap) {
                const std::string &A = entry.first;  // 产生式左部非终结符
                const std::vector<Production> &productions = entry.second;
                
                for (const Production &prod : productions) {
                    // 遍历产生式右部的每个符号
                    for (size_t i = 0; i < prod.size(); ++i) {
                        const std::string &B = prod[i];  // 当前符号B
                        
                        if (!isNonTerminal(B)) {
                            continue;  // 只处理非终结符
                        }
                        
                        // 计算β：B后面的符号序列
                        std::vector<std::string> beta(prod.begin() + i + 1, prod.end());
                        std::unordered_set<std::string> firstBeta = computeFirstOfString(beta);
                        
                        // 规则2：将FIRST(β)中除ε外的所有符号加入FOLLOW(B)
                        for (const std::string &symbol : firstBeta) {
                            if (symbol != "ε" && followSet[B].count(symbol) == 0) {
                                followSet[B].insert(symbol);
                                changed = true;
                            }
                        }
                        
                        // 规则3：如果ε ∈ FIRST(β)，则将FOLLOW(A)加入FOLLOW(B)
                        if (firstBeta.count("ε") > 0) {
                            for (const std::string &symbol : followSet[A]) {
                                if (followSet[B].count(symbol) == 0) {
                                    followSet[B].insert(symbol);
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // 打印FOLLOW集
        outputFile << "=== FOLLOW集 ===" << std::endl;
        for (const auto &entry : followSet) {
            outputFile << "FOLLOW(" << entry.first << ") = { ";
            for (const std::string &symbol : entry.second) {
                outputFile << symbol << " ";
            }
            outputFile << "}" << std::endl;
        }
        outputFile << std::endl;
        
        return followSet;
    }
    
    /**
     * @brief 构建LL(1)预测分析表
     * 
     * 预测分析表M[A,a]定义：
     * - M[A,a] = A → α，当且仅当：
     *   1. a ∈ FIRST(α)，或者
     *   2. ε ∈ FIRST(α) 且 a ∈ FOLLOW(A)
     * 
     * 对于LL(1)文法，每个M[A,a]最多只有一个产生式
     * 
     * @return 预测分析表，格式为 M[非终结符][终结符] = 产生式
     */
    std::unordered_map<std::string, std::unordered_map<std::string, Production>> constructParseTable() {
        std::unordered_map<std::string, std::unordered_map<std::string, Production>> table;
        
        // 初始化表结构
        for (const std::string &nt : nonTerminals) {
            table[nt] = std::unordered_map<std::string, Production>();
        }
        
        // 填充预测分析表
        for (const auto &entry : productionMap) {
            const std::string &A = entry.first;  // 非终结符
            const std::vector<Production> &productions = entry.second;
            
            for (const Production &alpha : productions) {
                // 计算FIRST(α)
                std::unordered_set<std::string> firstAlpha = computeFirstOfString(alpha);
                
                // 情况1：对于FIRST(α)中的每个终结符a，M[A,a] = A → α
                for (const std::string &a : firstAlpha) {
                    if (a != "ε") {
                        table[A][a] = alpha;
                    }
                }
                
                // 情况2：如果ε ∈ FIRST(α)，对于FOLLOW(A)中的每个终结符b，M[A,b] = A → α
                if (firstAlpha.count("ε") > 0) {
                    for (const std::string &b : followSet[A]) {
                        table[A][b] = alpha;
                    }
                }
            }
        }
        
        // 打印预测分析表
        outputFile << "=== 预测分析表 ===" << std::endl;
        outputFile << "      ";
        for (const std::string &term : terminals) {
            outputFile << term << "  ";
        }
        outputFile << std::endl;
        
        for (const std::string &nt : nonTerminals) {
            outputFile << nt << " | ";
            for (const std::string &term : terminals) {
                if (table[nt].count(term) > 0) {
                    const Production &prod = table[nt][term];
                    outputFile << nt << "→";
                    for (const std::string &sym : prod) {
                        outputFile << sym;
                    }
                    if (prod.empty()) outputFile << "ε";
                } else {
                    outputFile << "---";
                }
                outputFile << "  ";
            }
            outputFile << std::endl;
        }
        outputFile << std::endl;
        
        return table;
    }
    
    /**
     * @brief 获取当前token对应的终结符
     * @return 当前token的终结符表示
     */
    std::string getCurrentTokenSymbol() {
        if (currentToken.type == TokenType::END) {
            return "$";
        } else if (currentToken.type == TokenType::IDENTIFIED) {
            return "id";
        } else if (currentToken.type == TokenType::NUMBER) {
            return "num";
        } else if (currentToken.type == TokenType::KEYWORD) {
            return currentToken.lexeme;
        } else {
            return currentToken.lexeme;
        }
    }
    
public:
    /**
     * @brief 构造函数
     * @param s 词法分析器引用
     */
    LLSyntaxAnalyzer(TokenAnalyzer &s) : scanner(s) {
        outputFile.open("c:/Users/10497/Documents/Compliers/TestSet/SyntaxAnalysisOutput.txt");
        if (!outputFile.is_open()) {
            throw std::runtime_error("无法打开输出文件");
        }
        
        // 计算FIRST集、FOLLOW集并构建预测分析表
        outputFile << "=== LL(1)语法分析器初始化 ===" << std::endl << std::endl;
        firstSet = computeFirstSet();
        followSet = computeFollowSet();
        parseTable = constructParseTable();
    }
    
    /**
     * @brief 执行LL(1)预测分析
     * 
     * 分析算法：
     * 1. 初始化栈，将$和开始符号stmts压入栈
     * 2. 获取第一个token
     * 3. 重复以下步骤直到栈为空：
     *    a. 如果栈顶是终结符或$：
     *       i. 如果与当前token匹配，弹出栈顶并读取下一个token
     *       ii. 否则，语法错误
     *    b. 如果栈顶是非终结符A：
     *       i. 如果M[A,a]存在，弹出A并将产生式右部逆序压栈
     *       ii. 否则，语法错误
     * 4. 如果栈为空且token为$，分析成功
     */
    void parse() {
        std::vector<std::string> stack;
        stack.push_back("$");
        stack.push_back("stmts");
        
        currentToken = scanner.scan();
        std::string a = getCurrentTokenSymbol();
        
        outputFile << "=== 分析过程 ===" << std::endl;
        outputFile << "栈\t\t输入\t\t动作" << std::endl;
        outputFile << "----------------------------------------" << std::endl;
        
        while (!stack.empty()) {
            std::string X = stack.back();
            stack.pop_back();
            
            // 打印当前状态
            outputFile << "[";
            for (size_t i = 0; i < stack.size(); ++i) {
                if (i > 0) outputFile << " ";
                outputFile << stack[i];
            }
            outputFile << "]\t\t" << a << "\t\t";
            
            if (X == "$") {
                if (a == "$") {
                    outputFile << "分析成功" << std::endl;
                    break;
                } else {
                    throw std::runtime_error("语法错误：多余的输入");
                }
            } else if (isTerminal(X)) {
                if (X == a) {
                    outputFile << "匹配 " << X << std::endl;
                    currentToken = scanner.scan();
                    a = getCurrentTokenSymbol();
                } else {
                    throw std::runtime_error("语法错误：期望 " + X + "，得到 " + a);
                }
            } else if (isNonTerminal(X)) {
                if (parseTable[X].count(a) > 0) {
                    Production prod = parseTable[X][a];
                    outputFile << X << " → ";
                    for (const std::string &sym : prod) {
                        outputFile << sym << " ";
                    }
                    if (prod.empty()) outputFile << "ε";
                    outputFile << std::endl;
                    
                    // 将产生式右部逆序压栈（不包括ε）
                    if (!prod.empty()) {
                        for (auto it = prod.rbegin(); it != prod.rend(); ++it) {
                            stack.push_back(*it);
                        }
                    }
                } else {
                    throw std::runtime_error("语法错误：无法处理 " + X + " 面对 " + a);
                }
            }
        }
        
        outputFile << std::endl << "=== 分析完成 ===" << std::endl;
        outputFile.close();
    }
};