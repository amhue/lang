#include "semantic.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using std::make_shared;
using std::optional;
using std::pair;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::vector;

// class Semantic : public Parser {
// private:
//     using ScopeTable = unordered_map<string, Symbol>;

//     vector<ScopeTable> symbol_table; // { name: { is_func, type } }

// public:
shared_ptr<Program> Semantic::program()
{
    auto prog { Parser::program() };
    symbol_table.push_back(ScopeTable {});

    for (auto decl : prog->decls) {
        if (shared_ptr<FunDecl> fun = std::dynamic_pointer_cast<FunDecl>(decl);
            fun != nullptr)

            symbol_table[0][fun->name] = Symbol { true, fun->type };

        else if (shared_ptr<VarDecl> var
            = std::dynamic_pointer_cast<VarDecl>(decl);
            var != nullptr)

            symbol_table[0][var->ident] = Symbol { false, var->type };
    }

    return prog;
}

// TODO: Redefine completely compound() and primary() here are leave out a
// virtual ... = 0

// Blocks used fo example:
// fun i32 sum(a, b)
// {
//     auto i32 a;
//     auto i32 b;
//
//     return a + b;
// }
// if (a ~= b) { return a; }
shared_ptr<CompStmt> Semantic::compound()
{
    tokens.expect(TokenType::LBRACE,
        "Expected a '{' but got a " + tokens.cur().token_str + " instead!");
    tokens.advance(1);

    // push a new stack entry for scope
    symbol_table.push_back(ScopeTable {});

    vector<shared_ptr<Decl>> decls {};
    vector<shared_ptr<Stmt>> stmts {};

    while (true) {
        if (tokens.match(TokenType::FEOF))
            throw runtime_error("ERROR: Expected a '}' but got "
                + tokens.peek(-1).token_str + " instead!");

        if (tokens.match(TokenType::RBRACE)) {
            tokens.advance(1);
            symbol_table.pop_back();
            break;
        }

        if (tokens.match(TokenType::EXTERN) || tokens.match(TokenType::AUTO)
            || tokens.match(TokenType::BASE_TYPE)) {

            auto decl { declaration() };

            if (shared_ptr<FunDecl> fun
                = std::dynamic_pointer_cast<FunDecl>(decl);
                fun != nullptr)

                (*symbol_table.rbegin())[fun->name]
                    = Symbol { true, fun->type };

            else if (shared_ptr<VarDecl> var
                = std::dynamic_pointer_cast<VarDecl>(decl);
                var != nullptr)

                (*symbol_table.rbegin())[var->ident]
                    = Symbol { false, var->type };

        } else
            stmts.push_back(statement());
    }

    return make_shared<CompStmt>(decls, stmts);
}

shared_ptr<Expr> Semantic::primary()
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
        optional<pair<string, Symbol>> sym {
            std::nullopt
        }; // check if symbol is found in symbol table

        for (auto it = symbol_table.rbegin(); it != symbol_table.rend(); ++it) {
            if (auto search = it->find(tokens.cur().token_str);
                search != it->end()) {
                sym = { search->first,
                    { search->second.is_fun, search->second.type } };
                break;
            }
        }

        if (!sym.has_value())
            throw runtime_error("ERROR: The indentifier "
                + tokens.cur().token_str + " has not been defined!");

        if (tokens.peek(1).token_type == TokenType::LPAREN) {
            if (!sym.value().second.is_fun)
                throw runtime_error("ERROR: The indentifier "
                    + tokens.cur().token_str + " is not a function!");

            return funcall();
        }

        Token tok { tokens.cur() };
        tokens.advance(1);
        return make_shared<Ident>(tok.token_str);
    }

    throw runtime_error("ERROR: Expected an expression but got "
        + tokens.cur().token_str + " instead!");
}

Semantic::Semantic(const TokenStream& tokens)
    : Parser { tokens }
{
}
