#include "SemanticAnalyzer.h"
SemanticAnalyzer::controlNode SemanticAnalyzer::stmts()
{
    /*
    stmts⟶ stmt rest0
        {
            rest0.inNextlist = stmt.nextlist;
            stmts.nextlist = rest0.nextlist;
        }
    */
    controlNode stmtNode = stmt();
    controlNode restNode = rest0(stmtNode);
    return restNode;
}

SemanticAnalyzer::controlNode SemanticAnalyzer::rest0(SemanticAnalyzer::controlNode stmtNode)
{
    /*
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
    */
   /*
   controlNode ret;
    if (currentToken.type != TokenType::END && 
        (currentToken.lexeme == "if" || currentToken.lexeme == "while" || 
         currentToken.type == TokenType::IDENTIFIED))
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
   */
    controlNode ret;
    if (currentToken.type != TokenType::END && 
        (currentToken.lexeme == "if" || currentToken.lexeme == "while" || 
         currentToken.type == TokenType::IDENTIFIED))
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
    // Implementation for stmt
    /*
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
            stmt.nextlist = merge(bool_expr.falselist, stmt.nextlist);
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
        */
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
            int m2 = M();

            controlNode stmt2 = stmt();
            
            backpatch(boolNode.truelist, m1);
            backpatch(boolNode.falselist, m2);

            // 回填N的跳转地址,stmt2已经执行,nextquad()指向下一条语句的地址
            backpatch(n, nextquad());
            ret.nextlist = merge(stmt1.nextlist, stmt2.nextlist);
        }
        else
        {
            backpatch(boolNode.truelist, m1);
            ret.nextlist = merge(boolNode.falselist, stmt1.nextlist);
        }
    }
    //while M1 (bool_expr) M2 stmt1
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
    // Implementation for M
    /*
    M ⟶ ℇ
    {
        M.quad = nextquad()
    }
    */
    return nextquad();
}

int SemanticAnalyzer::N()
{
    // Implementation for N
    /*
    N ⟶ ℇ
    {
        N.quad = makelist(nextquad())
        emit(j, -, -, 0)}
    }
    */
    int q = nextquad();
    emit("j", "-", "-", "0");
    return q;
}