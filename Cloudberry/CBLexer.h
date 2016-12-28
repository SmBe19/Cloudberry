#ifndef CBLEXER_H
#define CBLEXER_H

#include <vector>
#include <map>
#include <string>
#include <iostream>

namespace cb {

	class Token {
	public:
		enum class Type {
			indent,
			identifier,
			colon,
			comma,
			newline,
			bf_delimiter,
			bf_funcname,
			bf_type,
			bf_typedelimiter,
			bf_code,
			val_strry,
			val_nummy,
			val_fuzzy,
			type_nully,
			type_evvy,
			type_charry,
			type_nummy,
			type_fuzzy,
			type_strry,
			type_lissy,
			type_diccy,
			kw_if,
			kw_for,
			kw_in,
			kw_while,
			kw_dowhile,
			kw_classy,
			kw_goto,
			op_parenthese_open,
			op_parenthese_close,
			op_bracket_open,
			op_bracket_close,
			op_plus,
			op_minus,
			op_not,
			op_bitnot,
			op_times,
			op_divide,
			op_mod,
			op_leftshift,
			op_rightshift,
			op_smaller,
			op_smallereq,
			op_greater,
			op_greatereq,
			op_equals,
			op_notequals,
			op_bitand,
			op_bitxor,
			op_bitor,
			op_and,
			op_or,
			op_assign,
			op_assignplus,
			op_assignminus,
			op_assigntimes,
			op_assigndivide,
			op_assignmod,
			op_assignleftshift,
			op_assignrightshift,
			op_assignbitand,
			op_assignbitxor,
			op_assignbitor,
			op_access,
			op_inherit,
		};
		Token();
		Token(Type, std::string);
		~Token();
		Type type;
		std::string value;
	};

	class Lexer {
	public:
		Lexer();
		~Lexer();
		int lex(std::string);
		std::vector<Token> &getTokens();
		std::string errorstr;
		int errorpos;
	protected:
		std::vector<Token> tokens;
		std::map<std::string, Token::Type> identifierToToken;
		std::map<std::string, Token::Type> operatorToToken;
		std::string operatorChars = "+-!~*/%<>=&|^.";
	};

}

#endif // !CBLEXER_H
