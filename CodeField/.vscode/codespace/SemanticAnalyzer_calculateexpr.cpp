#include "SemanticAnalyzer.h"
struct CalculateNode;
string SemanticAnalyzer::expr()
{
    // Implementation for expr
    /*
    expr ⟶ term rest5
        {
            rest5.in = term.place
            expr.place = rest5.place
        }

        string term_place = term(); // term.place
        // 传参rest5.in = term.place
        // expr.place = rest5.place
        string exprNode_place = rest5(term_place);
        return exprNode_place;
    */
    string termPlace = term();
    return rest5(termPlace);
}

string SemanticAnalyzer::rest5(string inPlace)
{
    // Implementation for rest5
    /*
    rest5⟶ +term rest51 |
        {
            rest51.in = newtemp()
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
            rest5.place = rest51.in
        }
    */
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
    // Implementation for term
    /*
    term ⟶ unary rest6
        {rest6.in	=	unary.place}
        {term.place	=	rest6.place}
    */
    // string unary_place = unary();
    // CalculateNode rest6Node;
    // rest6Node.in = unary_place;
    // string term_place = rest6(rest6Node.in);
    string unaryPlace = unary();
    return rest6(unaryPlace);
}

string SemanticAnalyzer::rest6(string inPlace)
{
    // Implementation for rest6
    /*
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
    */
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
    // Implementation for unary
    /*
    unary ⟶factor
        {
            unary.place	=	factor.place
        }
    */
    return factor();
}

string SemanticAnalyzer::factor()
{
    // Implementation for factor
    /*
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
            factor.place	=	num.value(num.lexeme)
        }
    */
    if (currentToken.lexeme == "(")
    {
        match("()");
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
        currentToken = scanner.scan(); // 消费数字
        return numValue;
    }
    return "";
}
/*
int main()
{
    string inputfile = "C:/Users/10497/Documents/Compliers/TestSet/calculateTest.txt";
    string outputfile = "C:/Users/10497/Documents/Compliers/TestSet/calculateTestOutput.txt";

    try
    {
        TokenAnalyzer scanner(inputfile);
        SemanticAnalyzer analyzer(scanner, outputfile);
        analyzer.analyze();
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
    }
}
*/