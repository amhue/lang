#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lexer.hpp"

struct Expr {
    virtual ~Expr() { }
};

struct Ident : Expr {
    std::string name;
    Ident(std::string& name)
        : name { name }
    {
    }
};

struct Literal : Expr { };

struct Number : Literal {
    int64_t number;
    Number(Token token)
        : number { std::stoi(token.token_str) }
    {
    }
};

struct String : Literal {
    std::string str;
    String(Token& token)
        : str { token.token_str }
    {
    }
};

struct Assign : Expr {
    std::shared_ptr<Ident> ident;
    std::shared_ptr<Expr> expr;

    Assign(std::shared_ptr<Ident> ident, std::shared_ptr<Expr> expr)
        : ident { std::move(ident) }
        , expr { std::move(expr) }
    {
    }
};

struct Unary : Expr {
    Token op;
    std::shared_ptr<Expr> expr;

    Unary(Token& op, std::shared_ptr<Expr> expr)
        : op { op }
        , expr { std::move(expr) }
    {
    }
};

struct Binary : Expr {
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;

    Binary(std::shared_ptr<Expr> left, Token& op, std::shared_ptr<Expr> right)
        : left { std::move(left) }
        , op { op }
        , right { std::move(right) }
    {
    }
};

struct Grouping : Expr {
    std::shared_ptr<Expr> expr;
    Grouping(std::shared_ptr<Expr> expr)
        : expr { std::move(expr) }
    {
    }
};

struct Decl {
    virtual ~Decl() { }
};

struct Stmt {
    virtual ~Stmt() { }
};

struct ExprStmt : Stmt {
    std::shared_ptr<Expr> expr;
    ExprStmt(std::shared_ptr<Expr> expr)
        : expr { std::move(expr) }
    {
    }
};

struct RetStmt : Stmt {
    std::shared_ptr<Expr> expr;
    RetStmt(std::shared_ptr<Expr> expr)
        : expr { std::move(expr) }
    {
    }
};

struct CompStmt : Stmt {
    std::vector<std::shared_ptr<Decl>> decls;
    std::vector<std::shared_ptr<Stmt>> stmts;

    CompStmt(std::vector<std::shared_ptr<Decl>> decls,
        std::vector<std::shared_ptr<Stmt>> stmts)
        : decls { std::move(decls) }
        , stmts { std::move(stmts) }
    {
    }
};

struct BreakStmt : Stmt { };

struct ContStmt : Stmt { };

struct EmptyStmt : Stmt { };

struct VarDecl : Decl {
    using VarType = TokenType;
    // std::shared_ptr<Expr> expr;
    std::string ident;
    std::string type;
    VarType var_type; // auto / extern

    VarDecl( // std::shared_ptr<Expr> expr,
        std::string& ident, std::string& type, VarType var_type)
        : // expr { std::move(expr) }
        // ,
        ident { std::move(ident) }
        , type { std::move(type) }
        , var_type { var_type }
    {
    }
};

struct FunDecl : Decl {
    std::string name;
    std::string type;
    std::vector<std::string> param_list;
    std::shared_ptr<CompStmt> comp_stmt;

    FunDecl(std::string& name, std::string type,
        std::vector<std::string> param_list,
        std::shared_ptr<CompStmt> comp_stmt)
        : name { std::move(name) }
        , type { std::move(type) }
        , param_list { std::move(param_list) }
        , comp_stmt { std::move(comp_stmt) }
    {
    }
};

struct Program {
    std::vector<std::shared_ptr<Decl>> decls;

    Program(std::vector<std::shared_ptr<Decl>> decls)
        : decls { decls }
    {
    }
    virtual ~Program() { }
};

struct IfStmt : Stmt {
    std::shared_ptr<Expr> cond;
    std::shared_ptr<Stmt> if_branch;
    std::shared_ptr<Stmt> else_branch;

    IfStmt(std::shared_ptr<Expr> cond, std::shared_ptr<Stmt> if_branch,
        std::shared_ptr<Stmt> else_branch)
        : cond { std::move(cond) }
        , if_branch { std::move(if_branch) }
        , else_branch { std::move(else_branch) }
    {
    }
};

struct LoopStmt : Stmt {
    std::shared_ptr<Expr> cond;
    std::shared_ptr<Stmt> body;

    LoopStmt(std::shared_ptr<Expr> cond, std::shared_ptr<Stmt> body)
        : cond { std::move(cond) }
        , body { std::move(body) }
    {
    }
};

struct FunCall : Expr {
    std::string name;
    std::vector<std::shared_ptr<Expr>> exprs;

    FunCall(std::string& name, std::vector<std::shared_ptr<Expr>> exprs)
        : name { std::move(name) }
        , exprs { std::move(exprs) }
    {
    }
};

class Parser {
protected:
    TokenStream tokens;

public:
    virtual std::shared_ptr<Program> program();
    virtual std::shared_ptr<Decl> declaration();
    virtual std::vector<std::string> param_list();
    virtual std::shared_ptr<CompStmt> compound() = 0;
    virtual std::shared_ptr<Stmt> statement();
    virtual std::shared_ptr<Expr> expression();
    virtual std::shared_ptr<Expr> assign();
    virtual std::shared_ptr<Expr> equality();
    virtual std::shared_ptr<Expr> comparison();
    virtual std::shared_ptr<Expr> term();
    virtual std::shared_ptr<Expr> factor();
    virtual std::shared_ptr<Expr> unary();
    virtual std::shared_ptr<Expr> primary() = 0;
    virtual std::shared_ptr<FunCall> funcall();
    virtual std::vector<std::shared_ptr<Expr>> arg_list();

    Parser(const TokenStream& tokens);
    virtual ~Parser() = default;
    std::shared_ptr<Program> parse();
};
