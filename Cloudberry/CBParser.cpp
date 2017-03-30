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
		currentPosition = 0;
		currentTokens = nullptr;
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

		AST ast_classy = AST(AST::Type::classy, name);
		AST ast_inheritance = AST(AST::Type::identifier, inheritance);
		AST ast_body = AST(AST::Type::sequence, "");
		ast_classy.children.push_back(ast_inheritance);
		ast_classy.children.push_back(ast_body);

		currentASTs.top()->children.push_back(ast_classy);
		return parse_suite(&currentASTs.top()->children.back().children.back(), false);
	}

	bool Parser::parse_suite(AST *a_ast, bool a_allow_statement) {
		currentIndentation.push_back(Token::Type::indent);
		currentASTs.push(a_ast);
		bool wasEmpty = false;
		while ((wasEmpty = parse_empty_line() || parse_indentation())) {
			if (wasEmpty || parse_function() || parse_class() || (a_allow_statement && parse_statement())) {
				continue;
			}
			throw weHaveError("Unexpected token " + getTokenAt(0).value);
		}
		currentASTs.pop();
		currentIndentation.pop_back();
		return true;
	}

	bool Parser::parse_function() {
		std::vector<Token::Type> reqStart = { Token::Type::identifier, Token::Type::op_parenthese_open };
		std::vector<Token::Type> reqEnd = { Token::Type::op_parenthese_close, Token::Type::colon, Token::Type::newline };

		int oldPosition = currentPosition;
		AST ast_return_type;
		if (!parse_type(&ast_return_type) || !checkTypes(reqStart)) {
			currentPosition = oldPosition;
			return false;
		}

		std::string name = getTokenAt(1).value;
		AST ast_func = AST(AST::Type::function, name);
		AST ast_arguments = AST(AST::Type::list, "");
		AST ast_body = AST(AST::Type::sequence, "");
		currentPosition += 2;

		AST ast_argument_type;
		while (parse_type(&ast_argument_type) && getTokenAt(0).type == Token::Type::identifier) {
			std::string argname = getTokenAt(0).value;
			AST ast_argument = AST(AST::Type::op_declare, "");
			AST ast_argument_name = AST(AST::Type::identifier, getTokenAt(0).value);
			ast_argument.children.push_back(ast_argument_type);
			ast_argument.children.push_back(ast_argument_name);
			ast_arguments.children.push_back(ast_argument);
			currentPosition += 1;

			if (getTokenAt(0).type != Token::Type::comma) {
				break;
			}
			currentPosition += 1;
		}

		if (!checkTypes(reqEnd)) {
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected ):\n");
		}
		currentPosition += 3;
		ast_func.children.push_back(ast_return_type);
		ast_func.children.push_back(ast_arguments);
		ast_func.children.push_back(ast_body);

		currentASTs.top()->children.push_back(ast_func);
		return parse_suite(&currentASTs.top()->children.back().children.back(), true);
	}

	bool Parser::parse_type(AST *a_ast) {
		if (getTokenAt(0).type != Token::Type::identifier) {
			return false;
		}
		if (getTokenAt(1).type == Token::Type::op_smaller) {
			currentPosition += 1;
			*a_ast = AST(AST::Type::type_generic, getTokenAt(0).value);
			AST ast_child;
			while (parse_type(&ast_child)) {
				a_ast->children.push_back(ast_child);
				currentPosition += 2;
				if (getTokenAt(-1).type == Token::Type::comma) {
					continue;
				}
				if (getTokenAt(-1).type != Token::Type::op_smaller) {
					throw weHaveError("Unexpected token " + getTokenAt(1).value + "; Expected >");
				}
				break;
			}
		} else {
			*a_ast = AST(AST::Type::type, getTokenAt(0).value);
			currentPosition += 1;
		}
		return true;
	}

	bool Parser::parse_statement() {
		return parse_declaration() || parse_assign_statement() || parse_for_statement()
			|| parse_if_statement() || parse_while_statement() || parse_dowhile_statement();
	}

	bool Parser::parse_assign_statement() {
		int oldPosition = currentPosition;
		AST ast_lval;
		if (!parse_lvalue(&ast_lval)) {
			currentPosition = oldPosition;
			return false;
		}

		AST ast_op;
		switch (getTokenAt(0).type) {
		case Token::Type::op_assign:
			ast_op = AST(AST::Type::op_assign, "");
			break;
		case Token::Type::op_assignplus:
			ast_op = AST(AST::Type::op_assignplus, "");
			break;
		case Token::Type::op_assignminus:
			ast_op = AST(AST::Type::op_assignminus, "");
			break;
		case Token::Type::op_assigntimes:
			ast_op = AST(AST::Type::op_assigntimes, "");
			break;
		case Token::Type::op_assigndivide:
			ast_op = AST(AST::Type::op_assigndivide, "");
			break;
		case Token::Type::op_assignmod:
			ast_op = AST(AST::Type::op_assignmod, "");
			break;
		case Token::Type::op_assignleftshift:
			ast_op = AST(AST::Type::op_assignleftshift, "");
			break;
		case Token::Type::op_assignrightshift:
			ast_op = AST(AST::Type::op_assignrightshift, "");
			break;
		case Token::Type::op_assignbitand:
			ast_op = AST(AST::Type::op_assignbitand, "");
			break;
		case Token::Type::op_assignbitxor:
			ast_op = AST(AST::Type::op_assignbitxor, "");
			break;
		case Token::Type::op_assignbitor:
			ast_op = AST(AST::Type::op_assignbitor, "");
			break;
		default:
			currentPosition = oldPosition;
			return false;
		}
		currentPosition += 1;

		AST ast_rval;
		if (!parse_expression(&ast_rval)) {
			throw weHaveError("Expected expression");
		}

		ast_op.children.push_back(ast_lval);
		ast_op.children.push_back(ast_rval);

		currentASTs.top()->children.push_back(ast_op);

		return readNewline();
	}

	bool Parser::parse_function_call(AST *a_ast) {
		return false;
	}

	bool Parser::parse_expression(AST *a_ast) {
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
		int oldPosition = currentPosition;
		AST ast_type;
		if (!parse_type(&ast_type) || getTokenAt(0).type != Token::Type::identifier) {
			currentPosition = oldPosition;
			return false;
		}

		std::string name = getTokenAt(0).value;
		// we do not advance yet because there might be an assignment
		AST ast_declare = AST(AST::Type::op_declare, "");
		AST ast_name = AST(AST::Type::identifier, name);
		ast_declare.children.push_back(ast_type);
		ast_declare.children.push_back(ast_name);

		currentASTs.top()->children.push_back(ast_declare);

		if (getTokenAt(1).type == Token::Type::op_equals) {
			if (!parse_assign_statement()) {
				throw weHaveError("Invalid assign statement");
			}
		} else {
			currentPosition += 1;
			return readNewline();
		}

		return true;
	}

	bool Parser::parse_identifier(AST *a_ast) {
		if (getTokenAt(0).type != Token::Type::identifier) {
			return false;
		}
		*a_ast = AST(AST::Type::identifier, getTokenAt(0).value);
		currentPosition += 1;
		return true;
	}

	bool Parser::parse_lvalue(AST *a_ast) {
		// TODO only allow class access and list access
		return parse_function_call(a_ast) || parse_expression(a_ast);
	}

	bool Parser::parse_rvalue(AST *a_ast) {
		switch (getTokenAt(0).type) {
		case Token::Type::val_strry:
			*a_ast = AST(AST::Type::val_strry, getTokenAt(0).value);
			currentPosition += 1;
			return true;
		case Token::Type::val_nummy:
			*a_ast = AST(AST::Type::val_nummy, getTokenAt(0).value);
			currentPosition += 1;
			return true;
		case Token::Type::val_fuzzy:
			*a_ast = AST(AST::Type::val_fuzzy, getTokenAt(0).value);
			currentPosition += 1;
			return true;
		default:;
		}
		return parse_function_call(a_ast) || parse_expression(a_ast);
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
				if (parse_empty_line() || parse_goto() || parse_brainfuck() || parse_class()) {
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