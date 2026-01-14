#include "lexer.hpp"
#include "types.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

using std::cout;
using std::find;
using std::runtime_error;
using std::size_t;
using std::string;

void TokenStream::print() const
{
    for (auto token : tokens) {
        cout << static_cast<int>(token.token_type) << ": " << token.token_str
             << '\n';
    }
}

TokenStream::TokenStream(string buf)
    : tokens {}
    , current { 0 }
{
    size_t i = 0;
    while (i < buf.length()) {
        if (isspace(buf[i])) {
            i++;
            continue;
        }

        else if (isalpha(buf[i])) {
            string tok_str;
            while (i < buf.length() && isalnum(buf[i]))
                tok_str += buf[i++];

            TokenType tok_type;
            if (tok_str == "extern")
                tok_type = TokenType::EXTERN;
            else if (tok_str == "static")
                tok_type = TokenType::STATIC;
            else if (tok_str == "auto")
                tok_type = TokenType::AUTO;
            else if (tok_str == "return")
                tok_type = TokenType::RETURN;
            else if (tok_str == "fun")
                tok_type = TokenType::FUN;
            else if (tok_str == "if")
                tok_type = TokenType::IF;
            else if (tok_str == "else")
                tok_type = TokenType::ELSE;
            else if (tok_str == "loop")
                tok_type = TokenType::LOOP;
            else if (find(base_types.begin(), base_types.end(), tok_str)
                != base_types.end())
                tok_type = TokenType::BASE_TYPE;
            else
                tok_type = TokenType::IDENT;

            tokens.push_back({ tok_type, tok_str });
            continue;
        }

        else if (isdigit(buf[i])) {
            string tok_str;
            while (isdigit(buf[i]))
                tok_str += buf[i++];
            tokens.push_back({ TokenType::NUMBER, tok_str });
        }

        else if (buf[i] == '`') {
            string tok_str;
            tok_str += buf[i++];

            while (i < buf.length() && buf[i] != '`') {
                tok_str += buf[i++];
            }

            if (i < buf.length()) {
                tok_str += buf[i++];
                tokens.push_back({ TokenType::STRING, tok_str });
            } else {
                tokens.push_back({ TokenType::ERR, tok_str });
                break;
            }
        } else
            switch (buf[i]) {
            case '{': {
                tokens.push_back({ TokenType::LBRACE, "{" });
                ++i;
                break;
            }
            case '}': {
                tokens.push_back({ TokenType::RBRACE, "}" });
                ++i;
                break;
            }
            case '(': {
                tokens.push_back({ TokenType::LPAREN, "(" });
                ++i;
                break;
            }
            case ')': {
                tokens.push_back({ TokenType::RPAREN, ")" });
                ++i;
                break;
            }
            case ';': {
                tokens.push_back({ TokenType::SCOLON, ";" });
                ++i;
                break;
            }
            case '=': {
                if (buf[i + 1] == '=') {
                    tokens.push_back({ TokenType::DEQ, "==" });
                    i += 2;
                    break;
                }
                tokens.push_back({ TokenType::EQ, "=" });
                ++i;
                break;
            }
            case '>': {
                tokens.push_back({ TokenType::GT, ">" });
                ++i;
                break;
            }
            case '<': {
                tokens.push_back({ TokenType::LT, "<" });
                ++i;
                break;
            }
            case '+': {
                tokens.push_back({ TokenType::ADD, "+" });
                ++i;
                break;
            }
            case '-': {
                tokens.push_back({ TokenType::SUB, "-" });
                ++i;
                break;
            }
            case '*': {
                tokens.push_back({ TokenType::MUL, "*" });
                ++i;
                break;
            }
            case '/': {
                tokens.push_back({ TokenType::DIV, "/" });
                ++i;
                break;
            }
            case '%': {
                tokens.push_back({ TokenType::MOD, "%" });
                ++i;
                break;
            }
            case '~': {
                if (buf[i + 1] == '=') {
                    tokens.push_back({ TokenType::NEQ, "~=" });
                    i += 2;
                    break;
                }
                tokens.push_back({ TokenType::NOT, "~" });
                ++i;
                break;
            }
            default: {
                tokens.push_back({ TokenType::ERR, string { buf[i] } });
                ++i;
                break;
            }
            }
    }
    tokens.push_back({ TokenType::FEOF, "EOF" });
}

const Token& TokenStream::peek(size_t n) const
{
    return tokens.at(current + n);
}
const Token& TokenStream::advance(size_t n) { return tokens.at(current += n); }

inline const Token& TokenStream::cur() const { return peek(0); }

bool TokenStream::match(TokenType type) const
{
    return tokens.at(current).token_type == type;
}

const Token& TokenStream::expect(TokenType type, string msg)
{
    if (match(type))
        return advance(0);
    throw runtime_error { "ERROR: " + msg + " at " + cur().token_str };
}

bool TokenStream::is_end() const { return cur().token_type == TokenType::FEOF; }
