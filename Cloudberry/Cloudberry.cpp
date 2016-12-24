#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "CBLexer.h"

enum class errortype {
	none,
	lexer,
	parser,
	compiler,
};

errortype error = errortype::none;

std::vector<cb::Token> lexFile(std::string a_file) {
	cb::Lexer lexer;
	std::ifstream fs(a_file);
	std::string line;
	int linenum = 1;
	while (std::getline(fs, line)) {
		if (lexer.lex(line)) {
			std::cerr << "Lexer error " << linenum << ":" << lexer.errorpos << ": " << lexer.errorstr << std::endl;
			error = errortype::lexer;
			break;
		}
		linenum++;
	}
	return lexer.getTokens();
}

int main(int argc, char* argv[]) {

	std::string inputfile = "";
	for (int i = 0; i < argc; i++) {
		inputfile = std::string(argv[i]);
	}

	if (inputfile == "") {
		std::cerr << "No input file given!" << std::endl;
		return -1;
	}

	std::vector<cb::Token> tokens = lexFile(inputfile);

	if (error != errortype::none) {
		return -1;
	}

	std::cerr << "Done lexing file. Found " << tokens.size() << " tokens." << std::endl;

	return 0;
}

