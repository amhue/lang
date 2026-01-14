#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class TokenType {
    IDENT,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    SCOLON,
    COMMA,
    EXTERN,
    AUTO,
    BASE_TYPE, // Defined in types.hpp
    FUN,
    STATIC,
    NUMBER,
    RETURN,
    STRING,
    GT, // >
    LT, // <
    GTE, // >=
    LTE, // <=
    DEQ, // ==
    EQ, // =
    NEQ, // ~=
    NOT, // ~
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    IF,
    ELSE,
    LOOP,
    ERR,
    FEOF,
};

struct Token {
    TokenType token_type;
    std::string token_str;
};

class TokenStream {
private:
    std::vector<Token> tokens;
    std::size_t current;

public:
    void print() const;
    TokenStream(std::string buf);
    const Token& peek(std::size_t n) const;
    const Token& advance(std::size_t n);
    const Token& cur() const;
    bool match(TokenType type) const;
    const Token& expect(TokenType type, std::string msg);
    bool is_end() const;
};
