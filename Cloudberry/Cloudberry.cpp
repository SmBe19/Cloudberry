#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#elif _WIN32
#else
#error Platform not supported
#endif

#include "CloudberryCompileOptions.h"
#include "CBLexer.h"
#include "CBParser.h"
#include "CBCompilerPython.h"

enum class errortype {
	none,
	lexer,
	parser,
	compiler,
	interpreter,
};

enum class compilerbackend {
	python,
	cxx,
	c,
	as
};

errortype error = errortype::none;
cb::Lexer lexer;
cb::Parser parser;
cb::CompilerPython compilerPython;

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
	fs.close();
	return lexer.getTokens();
}

cb::AST &parseTokens(std::vector<cb::Token> &a_tokens) {
	if (parser.parse(a_tokens)) {
		std::cerr << "Parser error " << parser.errorpos << ", line " << a_tokens[parser.errorpos].line << ": " << parser.errorstr << std::endl;
		error = errortype::parser;
	}
	return parser.getRootAST();
}

void compileAST(cb::AST &a_ast, compilerbackend backend, std::string &a_file) {
	switch (backend) {
	case compilerbackend::python:
		if (compilerPython.compile(a_ast, a_file)) {
			std::cerr << "Compiler error line " << compilerPython.errorpos->line << ": " << compilerPython.errorstr << std::endl;
			error = errortype::compiler;
		}
		break;
	case compilerbackend::cxx:
	case compilerbackend::c:
	case compilerbackend::as:
	default:
		std::cerr << "Not yet implemented!" << std::endl;
	}
}

void runAST(cb::AST &a_ast) {
	std::string file = CB_RUN_FILE_NAME;
	compileAST(a_ast, compilerbackend::python, file);
	if (error == errortype::none) {
		#ifdef __linux__
		pid_t pid;
		if(pid = fork()){
			// parent
			int status;
			do{
				waitpid(pid, &status, 0);
			} while(!WIFEXITED(status));
			if(remove(CB_RUN_FILE_NAME)){
				perror("Could not delete run file");
			}
		} else {
			// child
			if(execlp(CB_RUN_PYTHON_NAME, CB_RUN_PYTHON_NAME, CB_RUN_FILE_NAME, (char*) NULL) == -1){
				perror("Could not start python");
			}
		}
		#elif _WIN32
		std::string cmd = CB_RUN_PYTHON_NAME " " CB_RUN_FILE_NAME;
		std::cerr << "Run with cmd " << cmd << std::endl;
		#else
		#error Platform not supported
		#endif
	}
}

int main(int argc, char* argv[]) {
	std::string inputfile = "";
	std::string outfile = "";
	bool compileFile = true;
	compilerbackend backend = compilerbackend::as;
	for (int i = 1; i < argc; i++) {
		if (!strcmp("-o", argv[i])) {
			if (i >= argc - 1) {
				std::cerr << "missing argument after -o" << std::endl;
				return -1;
			}
			outfile = std::string(argv[i + 1]);
			i += 1;
		} else if (!strcmp("-c", argv[i])) {
			if (i >= argc - 1) {
				std::cerr << "missing argument after -c" << std::endl;
				return -1;
			}
			if (!strcmp("python", argv[i + 1])) {
				backend = compilerbackend::python;
			} else if (!strcmp("cxx", argv[i + 1])) {
				backend = compilerbackend::cxx;
			} else if (!strcmp("c", argv[i + 1])) {
				backend = compilerbackend::c;
			} else if (!strcmp("asm", argv[i + 1])) {
				backend = compilerbackend::as;
			}
			i += 1;
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
		compileAST(ast, backend, outfile);

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

