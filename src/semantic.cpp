#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

struct Symbol {
    // std::string& name;
    bool is_fun;
    string type;
};

class Semantic : public Parser {
private:
    using ScopeTable = unordered_map<string, Symbol>;

    vector<ScopeTable> symbol_table; // { name: { is_func, type } }

public:
    shared_ptr<Program> program() override
    {
        auto prog { Parser::program() };
        symbol_table.push_back(ScopeTable {});

        for (auto decl : prog->decls) {
            if (shared_ptr<FunDecl> fun
                = std::dynamic_pointer_cast<FunDecl>(decl);
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
    shared_ptr<CompStmt> compound() override
    {
        auto comp { Parser::compound() };
        comp->decls;
    }
};
