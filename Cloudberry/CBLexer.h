#ifndef CBLEXER_H
#define CBLEXER_H

#include <vector>
#include <string>

namespace cb{

	class Token{
	public:
		enum class Type {
			indent,
			keyword,
			identifier,
			strry,
			nummy,
			fuzzy
		};
		Token();
		Token(Type);
		~Token();
		Type type;
	};

	class Lexer {
	public:
		Lexer();
		~Lexer();
		void lex(std::string);
		std::vector<Token> getTokens();
	protected:
		std::vector<Token> tokens;
	};

}

#endif // !CBLEXER_H
