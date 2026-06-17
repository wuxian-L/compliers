#include "SemanticAnalyzer.h"
// 控制流语句翻译
SemanticAnalyzer::controlNode SemanticAnalyzer::stmts()
{
    controlNode stmtNode = stmt();
    controlNode restNode = rest0(stmtNode);
    return restNode;
}

SemanticAnalyzer::controlNode SemanticAnalyzer::rest0(controlNode stmtNode)
{
    controlNode ret;
    if (currentToken.type != TokenType::END &&
        (currentToken.lexeme == "if" || currentToken.lexeme == "while" ||
         currentToken.type == TokenType::IDENTIFIED || currentToken.lexeme == "{"))
    {
        int m = nextquad();
        controlNode newStmt = stmt();
        backpatch(stmtNode.nextlist, m);
        controlNode rest = rest0(newStmt);
        ret.nextlist = rest.nextlist;
    }
    else
    {
        ret.nextlist = stmtNode.nextlist;
    }
    return ret;
}
SemanticAnalyzer::controlNode SemanticAnalyzer::stmt()
{
    controlNode ret;
    if (currentToken.type == TokenType::IDENTIFIED)
    {
        arrayNode locNode = loc();
        expect("=");
        string exprPlace = expr();
        expect(";");
        if (locNode.offset == "null")
        {
            emit("=", exprPlace, "-", locNode.place);
        }
        else
        {
            emit("[]=", exprPlace, "-", locNode.place + "[" + locNode.offset + "]");
        }
        ret.nextlist = -1;
    }
    else if (currentToken.lexeme == "if")
    {
        match("if");
        expect("(");
        boolExprNode boolNode = bool_expr();
        expect(")");
        int m1 = M();
        controlNode stmt1 = stmt();
        if (currentToken.lexeme == "else")
        {
            int n = N();
            match("else");
            int m2 = nextquad();

            controlNode stmt2 = stmt();
            
            backpatch(boolNode.truelist, m1);
            backpatch(boolNode.falselist, m2);

            backpatch(n, nextquad());

            int temp = merge(stmt1.nextlist, n);
            ret.nextlist = merge(temp, stmt2.nextlist);

        }
        else
        {
            backpatch(boolNode.truelist, m1);
            ret.nextlist = merge(boolNode.falselist, stmt1.nextlist);
        }
    }
    else if (currentToken.lexeme == "while")
    {
        match("while");
        int m1 = nextquad();
        expect("(");
        boolExprNode boolNode = bool_expr();
        expect(")");
        int m2 = nextquad();
        controlNode stmt1 = stmt();
        backpatch(stmt1.nextlist, m1);
        backpatch(boolNode.truelist, m2);
        emit("j", "-", "-", to_string(m1));
        ret.nextlist = boolNode.falselist;
    }
    else if(currentToken.lexeme == "{")
    {
        match("{");
        controlNode stmtsNode = stmts();
        match("}");
        ret.nextlist = stmtsNode.nextlist;
    }
    else
    {
        return {makelist(nextquad())};
    }
    return ret;
}

int SemanticAnalyzer::M()
{
    return nextquad();
}

int SemanticAnalyzer::N()
{
    int q = nextquad();
    emit("j", "-", "-", "-");
    return q;
}
// 数组 位置表达式翻译
SemanticAnalyzer::arrayNode SemanticAnalyzer::loc()
{
    arrayNode ret;
    if (currentToken.type == TokenType::IDENTIFIED)
    {
        string idName = currentToken.lexeme;
        if (lookupSymbol(idName) == nullptr)
        {
            insertSymbol(idName, currentToken.type, 0);
        }
        match(TokenType::IDENTIFIED);
        ret = resta(idName);
    }
    return ret;
}

SemanticAnalyzer::arrayNode SemanticAnalyzer::resta(string inArray)
{
    arrayNode ret;
    ret.inArray = inArray;
    if (currentToken.lexeme == "[")
    {
        match("[");
        arrayNode listNode = elist(inArray);
        expect("]");
        ret.place = newtemp();
        emit("-", listNode.array, "C", ret.place);
        ret.offset = newtemp();
        emit("*", "w", listNode.offset, ret.offset);
    }
    else
    {
        ret.place = inArray;
        ret.offset = "null";
    }
    return ret;
}

SemanticAnalyzer::arrayNode SemanticAnalyzer::elist(string inArray)
{
    arrayNode ret;
    string exprPlace = expr();
    ret = rest1({inArray, 1, exprPlace, "", "", ""});
    return ret;
}

SemanticAnalyzer::arrayNode SemanticAnalyzer::rest1(arrayNode elistNode)
{
    arrayNode ret;
    if (currentToken.lexeme == ",")
    {
        match(",");
        string exprPlace = expr();
        string t = newtemp();
        int m = elistNode.inNdim + 1;
        emit("*", elistNode.inPlace, to_string(m), t);
        emit("+", t, exprPlace, t);
        ret = rest1({elistNode.inArray, m, t, "", "", ""});
    }
    else
    {
        ret.array = elistNode.inArray;
        ret.offset = elistNode.inPlace;
    }
    return ret;
}
// 布尔表达式翻译
SemanticAnalyzer::boolExprNode SemanticAnalyzer::bool_expr()
{
    boolExprNode ret;
    if (currentToken.lexeme == "!" || currentToken.lexeme == "(" ||
        currentToken.lexeme == "true" || currentToken.lexeme == "false")
    {

        boolExprNode left = bool_value();
        ret = bool_rest(left);
    }
    else
    {
        ret = equality();
    }
    return ret;
}
SemanticAnalyzer::boolExprNode SemanticAnalyzer::bool_value()
{
    if (currentToken.lexeme == "!")
    {
        match("!");
        boolExprNode boolValue1Node = bool_value();
        boolExprNode boolValueNode;
        boolValueNode.truelist = boolValue1Node.falselist;
        boolValueNode.falselist = boolValue1Node.truelist;
        return boolValueNode;
    }
    else if (currentToken.lexeme == "(")
    {
        match("(");
        boolExprNode boolExprNode = bool_expr();
        expect(")");
        return boolExprNode;
    }
    else if (currentToken.lexeme == "true")
    {
        match("true");
        boolExprNode boolValueNode;
        boolValueNode.truelist = makelist(nextquad());
        emit("j", "-", "-", "-");
        return boolValueNode;
    }
    else if (currentToken.lexeme == "false")
    {
        match("false");
        boolExprNode boolValueNode;
        boolValueNode.falselist = makelist(nextquad());
        emit("j", "-", "-", "-");
        return boolValueNode;
    }
    else
    {
        return equality();
    }
}
SemanticAnalyzer::boolExprNode SemanticAnalyzer::equality()
{
    boolExprNode ret;
    boolExprNode relNode = rel();
    ret = rest4(relNode);
    return ret;
}
SemanticAnalyzer::boolExprNode SemanticAnalyzer::bool_rest(boolExprNode left)
{
    boolExprNode ret;
    if (currentToken.lexeme == "||")
    {
        match("||");
        int M_quad = M();
        boolExprNode boolValueNode = bool_value();
        backpatch(left.falselist, M_quad);
        boolExprNode boolRestNode = bool_rest(boolValueNode);

        backpatch(boolRestNode.truelist, M_quad);

        ret.truelist = merge(left.truelist, boolRestNode.truelist);
        ret.falselist = boolRestNode.falselist;

        return ret;
    }
    else if (currentToken.lexeme == "&&")
    {
        match("&&");
        int M_quad = M();
        boolExprNode boolValueNode = bool_value();

        backpatch(left.falselist, M_quad);
        boolExprNode boolRestNode = bool_rest(boolValueNode);
        ret.truelist = boolRestNode.truelist;
        ret.falselist = merge(left.falselist, boolRestNode.falselist);

        return ret;
    }
    else
    {
        ret.truelist = left.truelist;
        ret.falselist = left.falselist;
        return ret;
    }
}
SemanticAnalyzer::boolExprNode SemanticAnalyzer::rest4(boolExprNode relNode)
{
    boolExprNode ret = relNode;
    if (currentToken.lexeme == "==")
    {
        expect("==");
        boolExprNode relNode2 = rel();
        ret = rest4(relNode2);
    }
    else if (currentToken.lexeme == "!=")
    {
        expect("!=");
        boolExprNode relNode2 = rel();
        ret = rest4(relNode2);
    }
    return ret;
}

SemanticAnalyzer::boolExprNode SemanticAnalyzer::rel()
{
    boolExprNode ret;
    string exprPlace = expr();
    ret = rop_expr(exprPlace);
    return ret;
}

SemanticAnalyzer::boolExprNode SemanticAnalyzer::rop_expr(string arg)
{
    boolExprNode ret;
    if (currentToken.lexeme == "<")
    {
        match("<");
        string exprPlace = expr();
        ret.truelist = makelist(nextquad());
        ret.falselist = makelist(nextquad() + 1);
        emit("j<", arg, exprPlace, "-");
        emit("j", "-", "-", "-");
    }
    else if (currentToken.lexeme == "<=")
    {
        match("<=");
        string exprPlace = expr();
        ret.truelist = makelist(nextquad());
        ret.falselist = makelist(nextquad() + 1);
        emit("j<=", arg, exprPlace, "-");
        emit("j", "-", "-", "-");
    }
    else if (currentToken.lexeme == ">")
    {
        match(">");
        string exprPlace = expr();
        ret.truelist = makelist(nextquad());
        ret.falselist = makelist(nextquad() + 1);
        emit("j>", arg, exprPlace, "-");
        emit("j", "-", "-", "-");
    }
    else if (currentToken.lexeme == ">=")
    {
        match(">=");
        string exprPlace = expr();
        ret.truelist = makelist(nextquad());
        ret.falselist = makelist(nextquad() + 1);
        emit("j>=", arg, exprPlace, "-");
        emit("j", "-", "-", "-");
    }
    else
    {
        ret.truelist = makelist(nextquad());
        ret.falselist = makelist(nextquad() + 1);
        emit("jnz", arg, "-", "-");
        emit("j", "-", "-", "-");
    }
    return ret;
}
// 算术表达式翻译
string SemanticAnalyzer::expr()
{
    string termPlace = term();
    return rest5(termPlace);
}

string SemanticAnalyzer::rest5(string inPlace)
{
    if (currentToken.lexeme == "+")
    {
        match("+");
        string termPlace = term();
        string t = newtemp();
        emit("+", inPlace, termPlace, t);
        return rest5(t);
    }
    else if (currentToken.lexeme == "-")
    {
        match("-");
        string termPlace = term();
        string t = newtemp();
        emit("-", inPlace, termPlace, t);
        return rest5(t);
    }
    return inPlace;
}

string SemanticAnalyzer::term()
{
    string unaryPlace = unary();
    return rest6(unaryPlace);
}

string SemanticAnalyzer::rest6(string inPlace)
{
    if (currentToken.lexeme == "*")
    {
        match("*");
        string unaryPlace = unary();
        string t = newtemp();
        emit("*", inPlace, unaryPlace, t);
        return rest6(t);
    }
    else if (currentToken.lexeme == "/")
    {
        match("/");
        string unaryPlace = unary();
        string t = newtemp();
        emit("/", inPlace, unaryPlace, t);
        return rest6(t);
    }
    return inPlace;
}

string SemanticAnalyzer::unary()
{
    if (currentToken.lexeme == "!"){
        match("!");
        string unaryPlace = unary();
        string t = newtemp();
        emit("not", "-", unaryPlace, t);
        return t;
    }
    else if(currentToken.lexeme == "-"){
        match("-");
        string unaryPlace = unary();
        string t = newtemp();
        emit("neg", "-", unaryPlace, t);
        return t;
    }
    else{
        return factor();
    }
}

string SemanticAnalyzer::factor()
{
    if (currentToken.lexeme == "(")
    {
        match("(");
        string exprPlace = expr();
        expect(")");
        return exprPlace;
    }
    else if (currentToken.type == TokenType::IDENTIFIED)
    {
        arrayNode locNode = loc();
        if (locNode.offset == "null")
        {
            return locNode.place;
        }
        else
        {
            string t = newtemp();
            emit("=[],", locNode.place, locNode.offset, t);
            return t;
        }
    }
    else if (currentToken.type == TokenType::NUMBER)
    {
        string numValue = currentToken.lexeme;
        match(TokenType::NUMBER);
        return numValue;
    }
    return "";
}
/*
int main()
{
    string inputPath = "C:/Users/10497/Documents/Compliers/TestSet/controlTest.txt";
    string outputPath = "C:/Users/10497/Documents/Compliers/TestSet/controlTestOutput.txt";
    TokenAnalyzer scanner(inputPath);
    SemanticAnalyzer analyzer(scanner, outputPath);
    analyzer.analyze();
    return 0;
}
 */