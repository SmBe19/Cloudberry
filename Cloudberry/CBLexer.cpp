#include "CBLexer.h"

namespace cb{
	Token::Token(){}
	Token::Token(Token::Type a_type){
		type = a_type;
	}
	Token::~Token(){}

	Lexer::Lexer(){}
	Lexer::~Lexer(){}
	std::vector<Token> Lexer::getTokens(){
		return tokens;
	}
	void Lexer::lex(std::string code){

	}
}