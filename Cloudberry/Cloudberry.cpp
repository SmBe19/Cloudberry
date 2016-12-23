#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "CBLexer.h"

std::vector<cb::Token> lexFile(std::string file){
	cb::Lexer lexer;
	std::ifstream fs(file);
	std::string line;
	while (std::getline(fs, line)){
		lexer.lex(line);
	}
	return lexer.getTokens();
}

int main(int argc, char* argv[]) {

	std::string inputfile = "";
	for (int i = 0; i < argc; i++){
		inputfile = std::string(argv[i]);
	}

	if (inputfile == ""){
		std::cerr << "No input file given!" << std::endl;
		return -1;
	}

	lexFile(inputfile);

	std::cerr << "Done lexing file." << std::endl;

	return 0;
}

