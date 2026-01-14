#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

using std::cerr;
using std::cout;
using std::exit;
using std::ifstream;
using std::stringstream;

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        cerr << "ERROR: No file provided!\n"
             << "USAGE: " << argv[0] << " example.a" << '\n';
        exit(EXIT_FAILURE);
    }

    cout << "INFO: File " << argv[1] << '\n';

    ifstream input;
    input.exceptions(ifstream::badbit);

    try {
        input.open(argv[1]);
        if (!input.good())
            throw ifstream::failure("ERROR: No file present!");

        cout << "INFO: Opened " << argv[1] << " successfully!\n";
    } catch (const ifstream::failure& e) {
        cerr << "ERROR: "
             << "Could not open file " << argv[1] << '\n'
             << "ERROR: " << e.what() << '\n';
    }

    stringstream input_buf {};
    input_buf << input.rdbuf();
    input.close();

    TokenStream token_stream { input_buf.str() };
    token_stream.print();

    Parser parser { token_stream };
    auto program = parser.parse();

    return 0;
}
