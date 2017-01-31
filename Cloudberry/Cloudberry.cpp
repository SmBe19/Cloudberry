#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>

#include "CloudberryCompileOptions.h"
#include "CBLexer.h"
#include "CBParser.h"

enum class errortype {
	none,
	lexer,
	parser,
	compiler,
	interpreter,
};

errortype error = errortype::none;
cb::Lexer lexer;
cb::Parser parser;

std::vector<cb::Token> &lexFile(std::string a_file) {
	std::ifstream fs(a_file);
	std::string line;
	int linenum = 1;
	while (std::getline(fs, line)) {
		if (lexer.lex(line, linenum)) {
			std::cerr << "Lexer error " << linenum << ":" << lexer.errorpos << ": " << lexer.errorstr << std::endl;
			error = errortype::lexer;
			break;
		}
		linenum++;
	}
	return lexer.getTokens();
}

cb::AST &parseTokens(std::vector<cb::Token> &a_tokens) {
	if (parser.parse(a_tokens)) {
		std::cerr << "Parser error " << parser.errorpos << ", line " << a_tokens[parser.errorpos].line << ": " << parser.errorstr << std::endl;
		error = errortype::parser;
	}
	return parser.getRootAST();
}

void compileAST(cb::AST a_ast, std::string &a_file) {
}

void runAST(cb::AST &a_ast) {

}

int main(int argc, char* argv[]) {
	std::string inputfile = "";
	std::string outfile = CB_DEFAULT_OUT;
	bool compileFile = true;
	for (int i = 1; i < argc; i++) {
		if (!strcmp("-o", argv[i])) {
			if (i >= argc - 1) {
				std::cerr << "missing argument after -o" << std::endl;
				return -1;
			}
			outfile = std::string(argv[i+1]);
		} else if (!strcmp("-r", argv[i])) {
			compileFile = false;
		} else {
			inputfile = std::string(argv[i]);
		}
	}

	if (inputfile == "") {
		std::cerr << "No input file given!" << std::endl;
		return -1;
	}

	std::vector<cb::Token> &tokens = lexFile(inputfile);

	if (error != errortype::none) {
		return -1;
	}

	if (compileFile) {
		std::cerr << "Done lexing file. Found " << tokens.size() << " tokens." << std::endl;
	}

	cb::AST &ast = parseTokens(tokens);

	if (error != errortype::none) {
		return -1;
	}

	if (compileFile) {
		std::cerr << "Done parsing file." << std::endl;
	}

	if (compileFile) {
		compileAST(ast, outfile);

		if (error != errortype::none) {
			return -1;
		}

		std::cerr << "Done compiling file." << std::endl;
	} else {
		runAST(ast);

		if (error != errortype::none) {
			return -1;
		}
	}

	return 0;
}

