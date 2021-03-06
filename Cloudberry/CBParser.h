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
			sequence, // list of commands [-, children:any, ...]
			list, // list of AST elements [-, children:any, ...]
			classy, // classy [name, baseType:identifier, body:sequence]
			function, // function [name, returnType:type, arguments:list<op_declare>
			type, // type [name]
			type_generic, // type [name, type:type, ...]
			identifier, // identifier [name]
			brainfuck, // brainfuck code [code]
			val_nummy, // nummy val [val]
			val_fuzzy, // fuzzy val [val]
			val_strry, // strry val [val]
			function_call, // func() [-, func:lvalue, arguments:list<rvalue>]
			cs_if, // if [-, expression:expression, true:sequence, false:sequence]
			cs_for, // for [-, init:sequence, check:expression, update:sequence, body:sequence]
			cs_forin, // forin [-, var:declare, iterable:expression, body:sequence]
			cs_while, // while [-, expression:expression, body:sequence]
			cs_dowhile, // dowhile [-, expression:expression, body:sequence]
			cs_goto, // goto [name]
			op_access, // a.b [-, a:lvalue, b:identifier]
			op_listaccess, // a[b] [-, a:lvalue, b:list<rvalue>]
			op_plus, // a + b [-, a:expression, b:expression]
			op_minus, // a - b [-, a:expression, b:expression]
			op_not, // !a [-, a:expression]
			op_bitnot, // ~a [-, a:expression]
			op_times, // a * b [-, a:expression, b:expression]
			op_divide, // a / b [-, a:expression, b:expression]
			op_mod, // a % b [-, a:expression, b:expression]
			op_leftshift, // a << b [-, a:expression, b:expression]
			op_rightshift, // a >> b [-, a:expression, b:expression]
			op_smaller, // a < b [-, a:expression, b:expression]
			op_smallereq, // a <= b [-, a:expression, b:expression]
			op_greater, // a > b [-, a:expression, b:expression]
			op_greatereq, // a >= b [-, a:expression, b:expression]
			op_equals, // a == b [-, a:expression, b:expression]
			op_notequals, // a != b [-, a:expression, b:expression]
			op_bitand, // a & b [-, a:expression, b:expression]
			op_bitxor, // a ^ b [-, a:expression, b:expression]
			op_bitor, // a | b [-, a:expression, b:expression]
			op_and, // a && b [-, a:expression, b:expression]
			op_or, // a || b [-, a:expression, b:expression]
			op_assign, // a = b [-, a:lvalue, b:expression]
			op_assignplus, // a += b [-, a:lvalue, b:expression]
			op_assignminus, // a -= b [-, a:lvalue, b:expression]
			op_assigntimes, // a *= b [-, a:lvalue, b:expression]
			op_assigndivide, // a /= b [-, a:lvalue, b:expression]
			op_assignmod, // a %= b [-, a:lvalue, b:expression]
			op_assignleftshift, // a <<= b [-, a:lvalue, b:expression]
			op_assignrightshift, // a >>= b [-, a:lvalue, b:expression]
			op_assignbitand, // a &= b [-, a:lvalue, b:expression]
			op_assignbitxor, // a ^= b [-, a:lvalue, b:expression]
			op_assignbitor, // a |= b [-, a:lvalue, b:expression]
			op_assignand, // a &&= b [-, a:lvalue, b:expression]
			op_assignor, // a ||= b [-, a:lvalue, b:expression]
			op_declare, // declare variable: a b [-, a:type, b:identifier]
		};
		AST();
		AST(Type, std::string, int);
		~AST();
		std::vector<AST> children;
		Type type;
		std::string value;
		int line;
	};

	class ParserException {};

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
		std::map<Token::Type, AST::Type> tokenToAstType;
		ParserException weHaveError(std::string);
		int currentPosition;
		std::vector<Token> *currentTokens;
		std::stack<AST*> currentASTs;
		std::vector<Token::Type> currentIndentation;
		Token getTokenAt(int) const;
		AST createAST(AST::Type);
		AST createAST(AST::Type, std::string);
		AST createAST(AST::Type, std::string, int);
		bool checkTypes(std::vector<Token::Type>& a_types, int a_offset) const;
		bool checkTypes(std::vector<Token::Type>&) const;
		bool readNewline();
		bool parse_empty_line();
		bool parse_indentation();
		bool parse_goto();
		bool parse_brainfuck();
		bool parse_class();
		bool parse_suite(AST*, bool);
		bool parse_function();
		bool parse_type(AST*);
		bool parse_statement();
		bool parse_assign_statement();
		bool parse_function_call(AST*);
		bool parse_list_or_function(AST*, Token::Type, Token::Type);
		bool parse_expression(AST*);
		bool parse_expression_statement();
		bool parse_for_expression(AST*);
		bool parse_forin_expression(AST*);
		bool parse_for_statement();
		bool parse_forin_statement();
		bool parse_if_statement();
		bool parse_while_statement();
		bool parse_dowhile_statement();
		bool parse_iflike_statement(Token::Type, AST::Type, bool, bool, bool (Parser::*)(AST*));
		bool parse_declaration();
		bool parse_identifier(AST*);
		bool parse_lvalue(AST*);
		bool parse_rvalue(AST*, bool);
	};
}

#endif // !CBPARSER_H
