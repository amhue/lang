#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::vector;

shared_ptr<Program> Parser::program()
{
    vector<shared_ptr<Decl>> decls {};

    while (!tokens.match(TokenType::FEOF)) {
        decls.push_back(declaration());
    }

    return make_shared<Program>(decls);
}

shared_ptr<Decl> Parser::declaration()
{
    if (tokens.match(TokenType::FUN)) {
        string name {};
        tokens.advance(1);

        // If no type provided assume i32
        string type { "i32" }; // Types defined in types.hpp

        if (tokens.match(TokenType::BASE_TYPE)) {
            type = std::move(tokens.cur().token_str);
            tokens.advance(1);
        }

        if (tokens.match(TokenType::IDENT)) {
            name = tokens.cur().token_str;
            tokens.advance(1);
        } else
            tokens.expect(TokenType::IDENT,
                "Expected an identifier but got " + tokens.cur().token_str
                    + " instead");

        vector<string> params { param_list() };
        shared_ptr<CompStmt> comp_stmt { compound() };

        return make_shared<FunDecl>(name, params, comp_stmt);
    } else if (tokens.match(TokenType::AUTO)
        || tokens.match(TokenType::EXTERN)) {
        TokenType var_type { tokens.cur().token_type };
        tokens.advance(1);

        // If no type provided assume i32
        string type { "i32" }; // Types defined in types.hpp

        if (tokens.match(TokenType::BASE_TYPE)) {
            type = std::move(tokens.cur().token_str);
            tokens.advance(1);
        }

        // Variable name
        tokens.expect(TokenType::IDENT,
            "Expected an identifier but got " + tokens.cur().token_str
                + " instead!");
        string ident { tokens.cur().token_str };
        tokens.advance(1);
        tokens.expect(TokenType::SCOLON,
            "Expected ';' but got " + tokens.cur().token_str + " instead!");

        tokens.advance(1);

        return make_shared<VarDecl>(ident, type, var_type);
    }

    throw runtime_error("ERROR: Expected a declaration got a "
        + tokens.cur().token_str + " instead!");
}

vector<string> Parser::param_list()
{
    tokens.expect(TokenType::LPAREN,
        "Expected a '(' but got a " + tokens.cur().token_str + " instead");
    tokens.advance(1);
    vector<string> params {};

    while (!tokens.match(TokenType::RPAREN)) {
        if (tokens.match(TokenType::FEOF))
            throw runtime_error("Expected ')' but reached EOF");

        tokens.expect(TokenType::IDENT,
            "Expected an parameter but got " + tokens.cur().token_str
                + " instead!");
        params.push_back(tokens.cur().token_str);
        tokens.advance(1);
    }

    tokens.advance(1);
    return params;
}

// Blocks used fo example:
// fun i32 sum(a, b)
// {
//     auto i32 a;
//     auto i32 b;
//
//     return a + b;
// }
// if (a ~= b) { return a; }

shared_ptr<CompStmt> Parser::compound()
{
    tokens.expect(TokenType::LBRACE,
        "Expected a '{' but got a " + tokens.cur().token_str + " instead!");
    tokens.advance(1);

    vector<shared_ptr<Decl>> decls {};
    vector<shared_ptr<Stmt>> stmts {};

    while (true) {
        if (tokens.match(TokenType::FEOF))
            throw runtime_error("ERROR: Expected a '}' but got "
                + tokens.peek(-1).token_str + " instead!");

        if (tokens.match(TokenType::RBRACE)) {
            tokens.advance(1);
            break;
        }

        if (tokens.match(TokenType::EXTERN) || tokens.match(TokenType::AUTO)
            || tokens.match(TokenType::BASE_TYPE))
            decls.push_back(declaration());
        else
            stmts.push_back(statement());
    }

    return make_shared<CompStmt>(decls, stmts);
}

shared_ptr<Stmt> Parser::statement()
{
    switch (tokens.cur().token_type) {
        // Return Statement "return" <expression> ";"
    case TokenType::RETURN: {
        tokens.advance(1);
        shared_ptr<Expr> expr = expression();
        tokens.expect(TokenType::SCOLON,
            "Expected a ';' but got a " + tokens.cur().token_str + " instead!");
        tokens.advance(1);
        return make_shared<RetStmt>(expr);
    }

        // If statement "if" <statement> [ "else" <statement> ]
    case TokenType::IF: {
        tokens.advance(1);
        tokens.expect(TokenType::LPAREN,
            "Expected a '(' but got a " + tokens.cur().token_str + " instead!");
        tokens.advance(1);
        shared_ptr<Expr> cond { expression() };

        tokens.expect(TokenType::RPAREN,
            "Expected a ')' but got a " + tokens.cur().token_str + " instead!");

        tokens.advance(1);
        shared_ptr<Stmt> if_stmt { statement() };

        shared_ptr<Stmt> else_stmt { nullptr };
        if (tokens.cur().token_type == TokenType::ELSE) {
            tokens.advance(1);
            else_stmt = statement();
        }

        return make_shared<IfStmt>(cond, if_stmt, else_stmt);
    }

        // Loop statement "loop" <statement>
    case TokenType::LOOP: {
        tokens.advance(1);
        tokens.expect(TokenType::LPAREN,
            "Expected a '(' but got a " + tokens.cur().token_str + " instead!");
        tokens.advance(1);
        shared_ptr<Expr> cond { expression() };

        tokens.expect(TokenType::RPAREN,
            "Expected a ')' but got a " + tokens.cur().token_str + " instead!");
        tokens.advance(1);

        return make_shared<LoopStmt>(cond, statement());
    }

        // Compound statement { ... }
    case TokenType::LBRACE: {
        return compound();
    }

        // Expression statement <expression> ";"
    default: {
        shared_ptr<Expr> expr { expression() };
        tokens.expect(TokenType::SCOLON,
            "Expected a ';' but got a " + tokens.cur().token_str + " instead!");
        tokens.advance(1);

        return make_shared<ExprStmt>(expr);
    }
    }

    throw runtime_error("ERROR: Not implemented yet!");
}

shared_ptr<Expr> Parser::expression() { return assign(); }

shared_ptr<Expr> Parser::assign()
{
    shared_ptr<Expr> expr = equality();
    if (tokens.match(TokenType::EQ)) {
        tokens.advance(1);
        shared_ptr<Expr> right { assign() };

        auto ident { dynamic_pointer_cast<Ident>(expr) };

        if (!ident)
            throw runtime_error("ERROR: Expected an lvalue here.!");

        expr = make_shared<Assign>(ident, right);
    }
    return expr;
}

shared_ptr<Expr> Parser::equality()
{
    shared_ptr<Expr> expr = comparison();

    while (tokens.match(TokenType::DEQ) || tokens.match(TokenType::NEQ)) {
        Token op { tokens.cur() };
        tokens.advance(1);
        shared_ptr<Expr> right { comparison() };
        expr = make_shared<Binary>(expr, op, right);
    }

    return expr;
}

shared_ptr<Expr> Parser::comparison()
{
    shared_ptr<Expr> expr = term();

    while (tokens.match(TokenType::GTE) || tokens.match(TokenType::GT)
        || tokens.match(TokenType::LTE) || tokens.match(TokenType::LT)) {
        Token op { tokens.cur() };
        tokens.advance(1);
        shared_ptr<Expr> right { term() };
        expr = make_shared<Binary>(expr, op, right);
    }
    return expr;
}

shared_ptr<Expr> Parser::term()
{
    shared_ptr<Expr> expr = factor();

    while (tokens.match(TokenType::ADD) || tokens.match(TokenType::SUB)) {
        Token op { tokens.cur() };
        tokens.advance(1);
        shared_ptr<Expr> right { factor() };
        expr = make_shared<Binary>(expr, op, right);
    }
    return expr;
}

shared_ptr<Expr> Parser::factor()
{
    shared_ptr<Expr> expr = unary();

    while (tokens.match(TokenType::MUL) || tokens.match(TokenType::DIV)) {
        Token op { tokens.cur() };
        tokens.advance(1);
        shared_ptr<Expr> right { unary() };
        expr = make_shared<Binary>(expr, op, right);
    }
    return expr;
}

shared_ptr<Expr> Parser::unary()
{
    if (tokens.match(TokenType::SUB) || tokens.match(TokenType::ADD)
        || tokens.match(TokenType::NOT)) {
        Token op = tokens.cur();
        tokens.advance(1);
        shared_ptr<Expr> expr { unary() };
        return make_shared<Unary>(op, expr);
    }

    return primary();
}

shared_ptr<Expr> Parser::primary()
{
    // std::cout << tokens.cur().token_str << '\n';
    if (tokens.match(TokenType::NUMBER)) {
        Token tok { tokens.cur() };
        tokens.advance(1);
        return make_shared<Number>(tok);
    }

    if (tokens.match(TokenType::STRING)) {
        Token tok { tokens.cur() };
        tokens.advance(1);
        return make_shared<String>(tok);
    }

    if (tokens.match(TokenType::LPAREN)) {
        tokens.advance(1);
        shared_ptr<Expr> expr { expression() };
        tokens.expect(TokenType::RPAREN, "Expected a ')'");
        tokens.advance(1);
        return make_shared<Grouping>(expr);
    }
    if (tokens.match(TokenType::IDENT)) {
        if (tokens.peek(1).token_type == TokenType::LPAREN)
            return funcall();

        Token tok { tokens.cur() };
        tokens.advance(1);
        return make_shared<Ident>(tok.token_str);
    }
    throw runtime_error("ERROR: Expected an expression but got "
        + tokens.cur().token_str + " instead!");
}

// funcall: "(" + *expr + ")"
shared_ptr<FunCall> Parser::funcall()
{
    tokens.expect(TokenType::IDENT,
        "Expected an identifier but got a " + tokens.cur().token_str
            + " instead!");

    string name { tokens.cur().token_str };
    tokens.advance(1);
    // vector<shared_ptr<Expr>> exprs {};

    tokens.expect(TokenType::LPAREN,
        "Expected a '(' but got " + tokens.cur().token_str + " instead!");
    tokens.advance(1);

    vector<shared_ptr<Expr>> exprs {};
    while (!tokens.match(TokenType::RPAREN))
        exprs.push_back(expression());

    tokens.advance(1);

    return make_shared<FunCall>(name, exprs);
}

vector<shared_ptr<Expr>> Parser::arg_list()
{
    tokens.expect(TokenType::LPAREN,
        "Expected a '(' but got a " + tokens.cur().token_str + " instead");
    tokens.advance(1);
    vector<shared_ptr<Expr>> args {};

    while (!tokens.match(TokenType::RPAREN)) {
        if (tokens.match(TokenType::FEOF))
            throw runtime_error("Expected ')' but reached EOF");

        args.push_back(expression());
    }

    tokens.advance(1);
    return args;
}

Parser::Parser(const TokenStream& tokens)
    : tokens { tokens }
{
}

shared_ptr<Program> Parser::parse() { return program(); }
