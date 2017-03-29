#include "CBParser.h"

namespace cb {
	AST::AST() {}

	AST::AST(AST::Type a_type, std::string a_value) {
		type = a_type;
		value = a_value;
	}

	AST::~AST() {}

	Parser::Parser() {
		operatorPrecedence[Token::Type::op_parenthese_open] = 1;
		operatorPrecedence[Token::Type::op_parenthese_close] = 1;
		operatorPrecedence[Token::Type::op_bracket_open] = 1;
		operatorPrecedence[Token::Type::op_bracket_close] = 1;
		operatorPrecedence[Token::Type::op_access] = 1;
		operatorPrecedence[Token::Type::op_not] = 2;
		operatorPrecedence[Token::Type::op_bitnot] = 2;
		operatorPrecedence[Token::Type::op_times] = 3;
		operatorPrecedence[Token::Type::op_divide] = 3;
		operatorPrecedence[Token::Type::op_mod] = 3;
		operatorPrecedence[Token::Type::op_plus] = 4;
		operatorPrecedence[Token::Type::op_minus] = 4;
		operatorPrecedence[Token::Type::op_leftshift] = 5;
		operatorPrecedence[Token::Type::op_rightshift] = 5;
		operatorPrecedence[Token::Type::op_smaller] = 6;
		operatorPrecedence[Token::Type::op_smallereq] = 6;
		operatorPrecedence[Token::Type::op_greater] = 6;
		operatorPrecedence[Token::Type::op_greatereq] = 6;
		operatorPrecedence[Token::Type::op_equals] = 7;
		operatorPrecedence[Token::Type::op_notequals] = 7;
		operatorPrecedence[Token::Type::op_bitand] = 8;
		operatorPrecedence[Token::Type::op_bitxor] = 9;
		operatorPrecedence[Token::Type::op_bitor] = 10;
		operatorPrecedence[Token::Type::op_and] = 11;
		operatorPrecedence[Token::Type::op_or] = 12;
		operatorPrecedence[Token::Type::op_assign] = 13;
		operatorPrecedence[Token::Type::op_assignplus] = 13;
		operatorPrecedence[Token::Type::op_assignminus] = 13;
		operatorPrecedence[Token::Type::op_assigntimes] = 13;
		operatorPrecedence[Token::Type::op_assigndivide] = 13;
		operatorPrecedence[Token::Type::op_assignmod] = 13;
		operatorPrecedence[Token::Type::op_assignleftshift] = 13;
		operatorPrecedence[Token::Type::op_assignrightshift] = 13;
		operatorPrecedence[Token::Type::op_assignbitand] = 13;
		operatorPrecedence[Token::Type::op_assignbitxor] = 13;
		operatorPrecedence[Token::Type::op_assignbitor] = 13;

		errorpos = 0;
		errorstr = "";
	}

	Parser::~Parser() {}

	AST &Parser::getRootAST() {
		return root;
	}

	ParserException Parser::weHaveError(std::string a_err) {
		errorpos = currentPosition;
		errorstr = a_err;
		return ParserException();
	}

	Token Parser::getTokenAt(int a_offset) const {
		return currentTokens->at(currentPosition + a_offset);
	}


	template<typename T>
	std::string tokenjoin(std::vector<T> &a_tokens, size_t a_pos, size_t a_cnt) {
		std::string res = "";
		for (size_t i = a_pos; i < a_pos + a_cnt; i++) {
			res += a_tokens[i].value + " ";
		}
		return res;
	}

	template<typename T>
	std::string tokenjoin(std::vector<T> &a_tokens) {
		return tokenjoin(a_tokens, 0, a_tokens.size());
	}

	bool Parser::checkTypes(std::vector<Token::Type> &a_types, int a_offset) const {
		for (int i = 0; i < a_types.size(); i++) {
			if (getTokenAt(i + a_offset).type != a_types[i]) {
				return false;
			}
		}
		return true;
	}

	bool Parser::checkTypes(std::vector<Token::Type> &a_types) const {
		return checkTypes(a_types, 0);
	}

	bool Parser::readNewline() {
		if (!(getTokenAt(0).type == Token::Type::newline)) {
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expcted \n");
		}
		currentPosition += 1;
		return true;
	}

	bool Parser::parse_empty_line() {
		int i;
		for (i = 0; getTokenAt(i).type == Token::Type::indent; i++);
		if (getTokenAt(i).type == Token::Type::newline) {
			currentPosition += i + 1;
			return true;
		}
		return false;
	}

	bool Parser::parse_indentation() {
		if (checkTypes(currentIndentation)) {
			currentPosition += currentIndentation.size();
			return true;
		}
		return false;
	}

	bool Parser::parse_goto() {
		std::vector<Token::Type> reqStart = { Token::Type::kw_goto, Token::Type::identifier };
		std::vector<Token::Type> reqMore = { Token::Type::comma, Token::Type::identifier };

		if (!checkTypes(reqStart)) {
			return false;
		}
		do {
			AST na = AST(AST::Type::cs_goto, getTokenAt(1).value);
			currentASTs.top()->children.push_back(na);
			currentPosition += 2;
		} while (checkTypes(reqMore));
		return readNewline();
	}

	bool Parser::parse_brainfuck() {
		std::vector<Token::Type> reqStart = { Token::Type::bf_delimiter, Token::Type::bf_funcname, Token::Type::bf_delimiter, Token::Type::bf_type };
		std::vector<Token::Type> reqTypes = { Token::Type::colon, Token::Type::bf_type };
		std::vector<Token::Type> reqCode = { Token::Type::bf_delimiter, Token::Type::bf_code, Token::Type::bf_delimiter };
		if (getTokenAt(0).type != Token::Type::bf_delimiter) {
			return false;
		}
		if (!checkTypes(reqStart)) {
			throw weHaveError("Invalid brainfuck signature");
		}
		std::string name = getTokenAt(1).value;
		std::string returnType = getTokenAt(3).value;
		currentPosition += 4;
		std::vector<std::string> arguments;
		while (checkTypes(reqTypes)) {
			arguments.push_back(getTokenAt(1).value);
			currentPosition += 2;
		}
		if (!checkTypes(reqCode)) {
			throw weHaveError("Missing brainfuck code");
		}
		std::string code = getTokenAt(1).value;
		currentPosition += 3;

		// build brainfuck AST
		AST ast_func = AST(AST::Type::function, name);
		AST ast_ret = AST(AST::Type::type, returnType);
		AST ast_arglist = AST(AST::Type::list, "");
		for (std::string argument : arguments) {
			AST ast_arg = AST(AST::Type::op_declare, "");
			AST ast_argtype = AST(AST::Type::type, argument);
			AST ast_argname = AST(AST::Type::identifier, "");
			ast_arg.children.push_back(ast_argtype);
			ast_arg.children.push_back(ast_argname);
			ast_arglist.children.push_back(ast_arg);
		}
		AST ast_funcbody = AST(AST::Type::sequence, "");
		AST ast_bf = AST(AST::Type::brainfuck, code);
		ast_funcbody.children.push_back(ast_bf);
		ast_func.children.push_back(ast_ret);
		ast_func.children.push_back(ast_arglist);
		ast_func.children.push_back(ast_funcbody);
		currentASTs.top()->children.push_back(ast_func);
		return readNewline();
	}


	bool Parser::parse_class() {
		std::vector<Token::Type> reqStart = { Token::Type::kw_classy, Token::Type::identifier };
		std::vector<Token::Type> reqInheritance = { Token::Type::op_inherit, Token::Type::identifier };
		std::vector<Token::Type> reqEnd = { Token::Type::colon, Token::Type::newline };

		if (!checkTypes(reqStart)) {
			return false;
		}

		std::string name = getTokenAt(1).value;
		currentPosition += 2;
		std::string inheritance = "";
		if (checkTypes(reqInheritance)) {
			inheritance = getTokenAt(1).value;
			currentPosition += 2;
		}

		if (!checkTypes(reqEnd)) {
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected :\n");
		}
		currentPosition += 2;

		// TODO: add AST

		return parse_suite();
	}

	bool Parser::parse_suite() {
		currentIndentation.push_back(Token::Type::indent);
		bool wasEmpty = false;
		while ((wasEmpty = parse_empty_line() || parse_indentation())) {
			if (wasEmpty) {
				continue;
			}
			if (parse_function()) {
				continue;
			}
			if (parse_class()) {
				continue;
			}
			if (parse_statement()) {
				continue;
			}
			throw weHaveError("Unexpected token " + getTokenAt(0).value);
		}
		currentIndentation.pop_back();
		return true;
	}

	bool Parser::parse_function() {
		std::vector<Token::Type> reqStart = { Token::Type::identifier, Token::Type::identifier, Token::Type::op_parenthese_open };
		std::vector<Token::Type> reqArg = { Token::Type::identifier, Token::Type::identifier };
		std::vector<Token::Type> reqEnd = { Token::Type::op_parenthese_close, Token::Type::colon, Token::Type::newline };

		if (!checkTypes(reqStart)) {
			return false;
		}

		std::string returnType = getTokenAt(0).value;
		std::string name = getTokenAt(1).value;
		std::vector<std::pair<std::string, std::string>> arguments;
		currentPosition += 3;

		while (checkTypes(reqArg)) {
			std::string argtype = getTokenAt(0).value;
			std::string argname = getTokenAt(1).value;
			arguments.push_back({ argtype, argname });
			currentPosition += 2;

			if (getTokenAt(0).type != Token::Type::comma) {
				break;
			}
			currentPosition += 1;
		}

		if (!checkTypes(reqEnd)) {
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected ):\n");
		}
		currentPosition += 3;

		// TODO: add AST

		return parse_suite();
	}

	bool Parser::parse_statement() {
		return false;
	}

	bool Parser::parse_assign_statement() {
		return false;
	}

	bool Parser::parse_expression() {
		return false;
	}

	bool Parser::parse_for_statement() {
		return false;
	}

	bool Parser::parse_if_statement() {
		return false;
	}

	bool Parser::parse_while_statement() {
		return false;
	}

	bool Parser::parse_dowhile_statement() {
		return false;
	}

	bool Parser::parse_declaration() {
		return false;
	}

	bool Parser::parse_lvalue() {
		return false;
	}

	bool Parser::parse_rvalue() {
		return false;
	}

	int Parser::parse(std::vector<Token> &a_tokens) {
		// add buffer at end
		for (int i = 0; i < 7; ++i) {
			a_tokens.push_back(Token(Token::Type::newline, "\\n", -1));
		}

		currentPosition = 0;
		currentTokens = &a_tokens;
		currentASTs.push(&root);

		try {
			while (currentPosition < currentTokens->size()) {
				if (parse_empty_line()) {
					continue;
				}
				if (parse_goto()) {
					continue;
				}
				if (parse_brainfuck()) {
					continue;
				}
				if (parse_class()) {
					continue;
				}
				throw weHaveError("Unexpected token " + getTokenAt(0).value);
			}
		} catch (ParserException e) {
			return 1;
		}

		return 0;
	}

}