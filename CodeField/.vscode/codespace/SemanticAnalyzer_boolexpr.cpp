#include "SemanticAnalyzer.h"
// 布尔表达式翻译
SemanticAnalyzer::boolExprNode SemanticAnalyzer::bool_expr()
{
    // Implementation for bool_expr
    /*
    bool_expr ⟶ bool_value bool_rest
            {
                bool_rest.inTruelist=bool_value.truelist
                bool_rest.inFalselist=bool_value.falselist
                bool_expr.truelist=bool_rest.truelist
                bool_expr.falselist=bool_rest.falselist
            }
            |equality
            {
                bool_expr.truelist=equality.truelist
                bool_expr.falselist=equality.falselist
            }
    */
     boolExprNode ret;
    //bool_value first集
    if (currentToken.lexeme == "!" || currentToken.lexeme == "(" || 
        currentToken.lexeme == "true" || currentToken.lexeme == "false") {

        boolExprNode left = bool_value();   
        ret = bool_rest(left);       

    } else {
        ret = equality();

    }
    return ret;
}

SemanticAnalyzer::boolExprNode SemanticAnalyzer::bool_value()
{
    // Implementation for bool_value
    /*
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
    */
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
}

SemanticAnalyzer::boolExprNode SemanticAnalyzer::bool_rest(boolExprNode left)
{
    // Implementation for bool_rest
    /*
    bool_rest ⟶ || M bool_value bool_rest
           | && M bool_value bool_rest
           | ℇ
    */
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

SemanticAnalyzer::boolExprNode SemanticAnalyzer::equality()
{
    // Implementation for equality
    /*
    equality ⟶rel rest4
        {
            rest4.inTruelist=rel.truelist
            rest4.inFalselist=rel.falselist
            equality.truelist=rest4.truelist
            equality.falselist=rest4.falselist
        }
        boolExprNode relNode = rel();
        boolExprNode rest4Node;
        rest4Node = rest4(relNode);
        boolExprNode equalityNode;
        equalityNode.truelist = rest4Node.truelist;
        equalityNode.falselist = rest4Node.falselist;
        return equalityNode;
    */
    boolExprNode ret;
    boolExprNode relNode = rel();
    ret = rest4(relNode);
    return ret;
}

SemanticAnalyzer::boolExprNode SemanticAnalyzer::rest4(boolExprNode relNode)
{
    // Implementation for rest4
    /*
    rest4 ⟶ ==rel rest4 |
         !=rel rest4 |
          ℇ
        {
            rest4.truelist=rest4.inTruelist
            rest4.falselist=rest4.inFalselist
        }
    */
    /*
    if (currentToken.lexeme == "==" || currentToken.lexeme == "!=")
    {
        string op = currentToken.lexeme;
        match(op);
        boolExprNode relNode = rel();
        boolExprNode rest4Node = rest4(relNode);
        if(op == "==")
        {
            backpatch(rest4Node.truelist, relNode.truelist);
            rest4Node.truelist = rest4Node.truelist;
            rest4Node.falselist = merge(relNode.falselist, rest4Node.falselist);
        }
        else
        {
            backpatch(rest4Node.falselist, relNode.falselist);
            rest4Node.truelist = merge(relNode.truelist, rest4Node.truelist);
            rest4Node.falselist = rest4Node.falselist;
        }
    }
    else
    {
        return relNode;
    }
     */
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
    // Implementation for rel
    /*
    rel ⟶expr rop_expr
    {
        rop_expr.inPlace=expr.place
        rel.truelist=rop_expr.truelist
        rel.falselist=rop_expr.falselist
    }
    */
    boolExprNode ret;
    string exprPlace = expr();
    ret = rop_expr(exprPlace);
    return ret;
}

SemanticAnalyzer::boolExprNode SemanticAnalyzer::rop_expr(string arg)
{
    // Implementation for rop_expr
    /*
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


    currentToken = scanner.peekToken();
    boolExprNode ropExprNode;
    ropExprNode.truelist = makelist(nextquad());
    ropExprNode.falselist = makelist(nextquad() + 1);
    string rop = currentToken.lexeme;
    if (rop == "<")
    {
        emit("j<", inPlace, expr(), "-");
    }
    else if (rop == "<=")
    {
        emit("j<=", inPlace, expr(), "-");
    }
    else if (rop == ">")
    {
        emit("j>", inPlace, expr(), "-");
    }
    else if (rop == ">=")
    {
        emit("j>=", inPlace, expr(), "-");
    }
    else
    {
        emit("jnz", inPlace, "-", "-");
    }
    emit("j", "-", "-", "-");
    return ropExprNode;
    */
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