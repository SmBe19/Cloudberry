#ifndef CBPARSER_H
#define CBPARSER_H

#include <vector>

#include "CBLexer.h"

namespace cb {
	class AST {
	public:
		AST();
		~AST();
		std::vector<AST> getChildren();
	protected:
		std::vector<AST> children;
	};

	class Parser {
	public:
		Parser();
		~Parser();
		int parse(std::vector<Token>);
		AST getRootAST();
		std::string errorstr;
		int errorpos;
	protected:
		AST root;
	};
}

#endif // !CBPARSER_H
