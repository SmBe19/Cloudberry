#ifndef CBPARSER_H
#define CBPARSER_H

#include <vector>
#include <map>
#include <stack>
#include <iostream>

#include "CBLexer.h"

namespace cb {
	class AST {
	public:
		enum class Type {
			sequence,
			list,
			classy,
			function,
			type,
			identifier,
			brainfuck,
			cs_if,
			cs_for,
			cs_while,
			cs_dowhile,
			cs_goto,
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
			op_declare,
		};
		AST();
		AST(Type, std::string);
		~AST();
		std::vector<AST> children;
		Type type;
		std::string value;
	};

	class Parser {
	public:
		Parser();
		~Parser();
		int parse(std::vector<Token>&);
		AST &getRootAST();
		std::string errorstr;
		int errorpos;
	protected:
		AST root;
		std::map<Token::Type, int> operatorPrecedence;
	};
}

#endif // !CBPARSER_H
