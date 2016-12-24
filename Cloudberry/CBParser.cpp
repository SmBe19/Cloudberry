#include "CBParser.h"

namespace cb {
	AST::AST() {}

	AST::~AST() {}

	std::vector<AST> AST::getChildren() {
		return children;
	}

	Parser::Parser() {}

	Parser::~Parser() {}

	AST Parser::getRootAST() {
		return root;
	}

	int Parser::parse(std::vector<Token> a_tokens) {

	}

}