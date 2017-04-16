#ifndef CBCOMPILERPYTHON_H
#define CBCOMPILERPYTHON_H

#include <fstream>

#include "CBParser.h"
#include "CloudberryCompileOptions.h"

namespace cb {

	class CompilerPythonException {};

	class CompilerPython {
	public:
		CompilerPython();
		~CompilerPython();
		int compile(AST&, std::string&);
		std::string errorstr;
		AST *errorpos;
	protected:
		std::ofstream fs;
		int indent;
		CompilerPythonException weHaveError(AST&, std::string);
		void printIndent();
		void emptyOrSequence(AST&);
		void op(AST&, std::string);
		void opassign(AST&, std::string);
		static std::string escape_string(std::string&);
		void compileNode(AST&);
	};
}

#endif // !CBCOMPILERPYTHON_H