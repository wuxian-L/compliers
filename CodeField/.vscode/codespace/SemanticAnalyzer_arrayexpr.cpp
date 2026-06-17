#include "SemanticAnalyzer.h"


SemanticAnalyzer::arrayNode SemanticAnalyzer::loc()
{
    // Implementation for loc
    /*
    loc⟶ id resta
        {
            resta.inArray = id.place
            loc.place = resta.place
            loc.offset = resta.offset
        }
    */
    arrayNode ret;
    if (currentToken.type == TokenType::IDENTIFIED)
    {
        string idName = currentToken.lexeme;
        if (lookupSymbol(idName) == nullptr)
        {
            insertSymbol(idName, currentToken.type, 0);
        }
        //currentToken = scanner.scan(); // 消费标识符
        match(TokenType::IDENTIFIED);
        ret = resta(idName);
    }
    return ret;
}

SemanticAnalyzer::arrayNode SemanticAnalyzer::resta(std::string inArray)
{
    // Implementation for resta
    /*
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
    */
    arrayNode ret;
    ret.inArray = inArray;
    //lookupSymbol(inArray)->isArray = true;
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

SemanticAnalyzer::arrayNode SemanticAnalyzer::elist(std::string inArray)
{
    // Implementation for elist
    /*
    elist ⟶ expr rest1
    {
        rest1.inArray=elist.inArray;
        rest1.inNdim=1;
        rest1.inPlace=expr.place}
        {elist.array=rest1.array;
        elist.offset=rest1.offset}
    }
    */
    arrayNode ret;
    string exprPlace = expr();
    ret = rest1({inArray, 1, exprPlace, "", "", ""});
    return ret;
}
std::string limit(std::string array, int j)
{
    // 返回字符串 "n" + to_string(j)
    return "n" + std::to_string(j);
}
SemanticAnalyzer::arrayNode SemanticAnalyzer::rest1(SemanticAnalyzer::arrayNode elistNode)
{
    // Implementation for rest1
    /*
    rest1⟶ , expr rest11
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
    */
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