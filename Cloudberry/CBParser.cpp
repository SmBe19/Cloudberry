#include "CBParser.h"

namespace cb {
	AST::AST() {}

	AST::AST(AST::Type a_type, std::string a_value, int a_line) {
		type = a_type;
		value = a_value;
		line = a_line;
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

		tokenToAstType[Token::Type::op_plus] = AST::Type::op_plus;
		tokenToAstType[Token::Type::op_minus] = AST::Type::op_minus;
		tokenToAstType[Token::Type::op_not] = AST::Type::op_not;
		tokenToAstType[Token::Type::op_bitnot] = AST::Type::op_bitnot;
		tokenToAstType[Token::Type::op_times] = AST::Type::op_times;
		tokenToAstType[Token::Type::op_divide] = AST::Type::op_divide;
		tokenToAstType[Token::Type::op_mod] = AST::Type::op_mod;
		tokenToAstType[Token::Type::op_leftshift] = AST::Type::op_leftshift;
		tokenToAstType[Token::Type::op_rightshift] = AST::Type::op_rightshift;
		tokenToAstType[Token::Type::op_smaller] = AST::Type::op_smaller;
		tokenToAstType[Token::Type::op_smallereq] = AST::Type::op_smallereq;
		tokenToAstType[Token::Type::op_greater] = AST::Type::op_greater;
		tokenToAstType[Token::Type::op_greatereq] = AST::Type::op_greatereq;
		tokenToAstType[Token::Type::op_equals] = AST::Type::op_equals;
		tokenToAstType[Token::Type::op_notequals] = AST::Type::op_notequals;
		tokenToAstType[Token::Type::op_bitand] = AST::Type::op_bitand;
		tokenToAstType[Token::Type::op_bitxor] = AST::Type::op_bitxor;
		tokenToAstType[Token::Type::op_bitor] = AST::Type::op_bitor;
		tokenToAstType[Token::Type::op_and] = AST::Type::op_and;
		tokenToAstType[Token::Type::op_or] = AST::Type::op_or;
		tokenToAstType[Token::Type::op_assign] = AST::Type::op_assign;
		tokenToAstType[Token::Type::op_assignplus] = AST::Type::op_assignplus;
		tokenToAstType[Token::Type::op_assignminus] = AST::Type::op_assignminus;
		tokenToAstType[Token::Type::op_assigntimes] = AST::Type::op_assigntimes;
		tokenToAstType[Token::Type::op_assigndivide] = AST::Type::op_assigndivide;
		tokenToAstType[Token::Type::op_assignmod] = AST::Type::op_assignmod;
		tokenToAstType[Token::Type::op_assignleftshift] = AST::Type::op_assignleftshift;
		tokenToAstType[Token::Type::op_assignrightshift] = AST::Type::op_assignrightshift;
		tokenToAstType[Token::Type::op_assignbitand] = AST::Type::op_assignbitand;
		tokenToAstType[Token::Type::op_assignbitxor] = AST::Type::op_assignbitxor;
		tokenToAstType[Token::Type::op_assignbitor] = AST::Type::op_assignbitor;
		tokenToAstType[Token::Type::op_assignand] = AST::Type::op_assignand;
		tokenToAstType[Token::Type::op_assignor] = AST::Type::op_assignor;

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

	AST Parser::createAST(AST::Type a_type) {
		return createAST(a_type, "");
	}

	AST Parser::createAST(AST::Type a_type, std::string a_value) {
		return createAST(a_type, a_value, currentTokens->at(currentPosition).line);
	}

	AST Parser::createAST(AST::Type a_type, std::string a_value, int a_line) {
		return AST(a_type, a_value, a_line);
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
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected \\n");
		}
		currentPosition += 1;
		return true;
	}

	bool Parser::parse_empty_line() {
		int i;
		// ReSharper disable once CppPossiblyErroneousEmptyStatements
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
			AST na = createAST(AST::Type::cs_goto, getTokenAt(1).value);
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
		AST ast_func = createAST(AST::Type::function, name);
		AST ast_ret = createAST(AST::Type::type, returnType);
		AST ast_arglist = createAST(AST::Type::list);
		int argnum = 0;
		for (std::string argument : arguments) {
			AST ast_arg = createAST(AST::Type::op_declare);
			AST ast_argtype = createAST(AST::Type::type, argument);
			AST ast_argname = createAST(AST::Type::identifier, "bf_" + std::to_string(argnum++));
			ast_arg.children.push_back(ast_argtype);
			ast_arg.children.push_back(ast_argname);
			ast_arglist.children.push_back(ast_arg);
		}
		AST ast_funcbody = createAST(AST::Type::sequence);
		AST ast_bf = createAST(AST::Type::brainfuck, code);
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
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected :\\n");
		}
		currentPosition += 2;

		AST ast_classy = createAST(AST::Type::classy, name);
		AST ast_inheritance = createAST(AST::Type::identifier, inheritance);
		AST ast_body = createAST(AST::Type::sequence);
		ast_classy.children.push_back(ast_inheritance);

		if (!parse_suite(&ast_body, true)) {
			throw weHaveError("Invalid class body");
		}
		ast_classy.children.push_back(ast_body);
		currentASTs.top()->children.push_back(ast_classy);
		return true;
	}

	bool Parser::parse_suite(AST *a_ast, bool a_only_declaration) {
		currentIndentation.push_back(Token::Type::indent);
		currentASTs.push(a_ast);
		bool wasEmpty = false;
		while (((wasEmpty = parse_empty_line())) || parse_indentation()) {
			if (wasEmpty || parse_function() || parse_class()) {
				continue;
			}
			if (a_only_declaration) {
				if (parse_declaration() || parse_assign_statement()) {
					continue;
				}
			} else {
				if (parse_statement()) {
					continue;
				}
			}
			throw weHaveError("Suite: Unexpected token " + getTokenAt(0).value);
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

		std::string name = getTokenAt(0).value;
		AST ast_func = createAST(AST::Type::function, name);
		AST ast_arguments = createAST(AST::Type::list);
		AST ast_body = createAST(AST::Type::sequence);
		currentPosition += 2;

		AST ast_argument_type;
		while (parse_type(&ast_argument_type) && getTokenAt(0).type == Token::Type::identifier) {
			std::string argname = getTokenAt(0).value;
			AST ast_argument = createAST(AST::Type::op_declare);
			AST ast_argument_name = createAST(AST::Type::identifier, getTokenAt(0).value);
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
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected ):\\n");
		}
		currentPosition += 3;
		ast_func.children.push_back(ast_return_type);
		ast_func.children.push_back(ast_arguments);
		ast_func.children.push_back(ast_body);

		currentASTs.top()->children.push_back(ast_func);
		return parse_suite(&currentASTs.top()->children.back().children.back(), false);
	}

	bool Parser::parse_type(AST *a_ast) {
		if (getTokenAt(0).type != Token::Type::identifier) {
			return false;
		}
		if (getTokenAt(1).type == Token::Type::op_smaller) {
			*a_ast = createAST(AST::Type::type_generic, getTokenAt(0).value);
			currentPosition += 2;
			AST ast_child;
			while (parse_type(&ast_child)) {
				a_ast->children.push_back(ast_child);
				currentPosition += 1;
				if (getTokenAt(-1).type == Token::Type::comma) {
					continue;
				}
				if (getTokenAt(-1).type != Token::Type::op_greater) {
					throw weHaveError("Unexpected token " + getTokenAt(1).value + "; Expected >");
				}
				break;
			}
		} else {
			*a_ast = createAST(AST::Type::type, getTokenAt(0).value);
			currentPosition += 1;
		}
		return true;
	}

	bool Parser::parse_statement() {
		return parse_for_statement() || parse_if_statement() || parse_while_statement()
			|| parse_dowhile_statement() || parse_declaration() || parse_assign_statement()
			|| parse_expression_statement();
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
		case Token::Type::op_assignplus:
		case Token::Type::op_assignminus:
		case Token::Type::op_assigntimes:
		case Token::Type::op_assigndivide:
		case Token::Type::op_assignmod:
		case Token::Type::op_assignleftshift:
		case Token::Type::op_assignrightshift:
		case Token::Type::op_assignbitand:
		case Token::Type::op_assignbitxor:
		case Token::Type::op_assignbitor:
			ast_op = createAST(tokenToAstType[getTokenAt(0).type]);
			break;
		default:
			currentPosition = oldPosition;
			return false;
		}
		currentPosition += 1;

		AST ast_rval;
		if (!parse_expression(&ast_rval)) {
			throw weHaveError("Assign statement: Expected expression");
		}

		ast_op.children.push_back(ast_lval);
		ast_op.children.push_back(ast_rval);

		currentASTs.top()->children.push_back(ast_op);

		return readNewline();
	}

	bool Parser::parse_function_call(AST *a_ast) {
		return parse_list_or_function(a_ast, Token::Type::op_parenthese_open, Token::Type::op_parenthese_close);
	}


	bool Parser::parse_list_or_function(AST *a_ast, Token::Type a_open, Token::Type a_close) {
		int oldPosition = currentPosition;
		AST ast_lvalue;
		if (!parse_lvalue(&ast_lvalue) || getTokenAt(0).type != a_open) {
			currentPosition = oldPosition;
			return false;
		}

		*a_ast = createAST(AST::Type::function_call);
		a_ast->children.push_back(ast_lvalue);
		AST ast_arguments = createAST(AST::Type::list);
		AST ast_argument;
		while (getTokenAt(0).type != a_close) {
			currentPosition += 1;
			if (!parse_rvalue(&ast_argument, true)) {
				throw weHaveError("Expected argument");
			}
			ast_arguments.children.push_back(ast_argument);
			if (getTokenAt(0).type != Token::Type::comma
				&& getTokenAt(0).type != a_close) {
				throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected , or )");
			}
		}
		a_ast->children.push_back(ast_arguments);

		currentPosition += 1;
		return true;
	}


	bool Parser::parse_expression_statement() {
		AST ast_expr;
		if (parse_expression(&ast_expr)) {
			currentASTs.top()->children.push_back(ast_expr);
			return readNewline();
		}
		return false;
	}


	bool Parser::parse_expression(AST *a_ast) {
		std::stack<Token> ops;
		std::stack<AST> values;

		int oldPosition = currentPosition;

		bool stillInExpression = true;
		while (stillInExpression) {
			AST ast_rvalue;
			if (parse_rvalue(&ast_rvalue, false)) {
				values.push(ast_rvalue);
				continue;
			}
			Token tok_now;
			switch (getTokenAt(0).type) {
			case Token::Type::op_parenthese_open:
				currentPosition += 1;
				if (parse_expression(&ast_rvalue) && getTokenAt(0).type == Token::Type::op_parenthese_close) {
					values.push(ast_rvalue);
					currentPosition += 1;
				} else {
					throw weHaveError("Malformed expression");
				}
				break;
			case Token::Type::op_parenthese_close:
			case Token::Type::op_bracket_close:
			case Token::Type::newline:
				while (!ops.empty()) {
					if (values.size() < 2) {
						throw weHaveError("Too few values in expression");
					}
					ast_rvalue = createAST(tokenToAstType[ops.top().type]);
					ops.pop();
					AST ast2 = values.top();
					values.pop();
					AST ast1 = values.top();
					values.pop();
					ast_rvalue.children.push_back(ast1);
					ast_rvalue.children.push_back(ast2);
					values.push(ast_rvalue);
				}
				if (values.size() > 1) {
					throw weHaveError("Too many values in expression");
				}
				if (values.empty()) {
					throw weHaveError("Not enough values in expression");
				}
				*a_ast = values.top();
				stillInExpression = false;
				break;
			case Token::Type::op_plus:
			case Token::Type::op_minus:
			case Token::Type::op_not:
			case Token::Type::op_bitnot:
			case Token::Type::op_times:
			case Token::Type::op_divide:
			case Token::Type::op_mod:
			case Token::Type::op_leftshift:
			case Token::Type::op_rightshift:
			case Token::Type::op_smaller:
			case Token::Type::op_smallereq:
			case Token::Type::op_greater:
			case Token::Type::op_greatereq:
			case Token::Type::op_equals:
			case Token::Type::op_notequals:
			case Token::Type::op_bitand:
			case Token::Type::op_bitxor:
			case Token::Type::op_bitor:
			case Token::Type::op_and:
			case Token::Type::op_or:
				tok_now = getTokenAt(0);
				while (!ops.empty() && operatorPrecedence[ops.top().type] <= operatorPrecedence[tok_now.type]) {
					if (values.size() < 2) {
						throw weHaveError("Too few values in expression");
					}
					ast_rvalue = createAST(tokenToAstType[ops.top().type]);
					ops.pop();
					AST ast2 = values.top();
					values.pop();
					AST ast1 = values.top();
					values.pop();
					ast_rvalue.children.push_back(ast1);
					ast_rvalue.children.push_back(ast2);
					values.push(ast_rvalue);
				}
				ops.push(tok_now);
				currentPosition += 1;
				break;
			default:
				currentPosition = oldPosition;
				return false;
			}
		}

		return true;
	}

	bool Parser::parse_for_expression(AST *a_ast) {
		*a_ast = createAST(AST::Type::sequence);
		int oldPosition = currentPosition;
		AST ast_rvalue1;
		if (!parse_rvalue(&ast_rvalue1, true)
			|| getTokenAt(0).type != Token::Type::op_parenthese_close
			|| getTokenAt(2).type != Token::Type::identifier) {
			currentPosition = oldPosition;
			return false;
		}

		Token tok_now = getTokenAt(1);
		bool is_up = tok_now.type == Token::Type::op_smaller || tok_now.type == Token::Type::op_smallereq;
		bool is_eq = tok_now.type == Token::Type::op_smallereq || tok_now.type == Token::Type::op_greatereq;
		if (!is_up && tok_now.type != Token::Type::op_greater && tok_now.type != Token::Type::op_greatereq) {
			currentPosition = oldPosition;
			return false;
		}

		std::string name = getTokenAt(2).value;
		Token tok_now2 = getTokenAt(3);

		currentPosition += 4;

		if (is_up && tok_now2.type != Token::Type::op_smaller && tok_now2.type != Token::Type::op_smallereq) {
			throw weHaveError("For: wrong operator");
		}
		if (!is_up && tok_now2.type != Token::Type::op_greater && tok_now2.type != Token::Type::op_greatereq) {
			throw weHaveError("For: wrong operator");
		}

		AST ast_rvalue2;
		if (!parse_rvalue(&ast_rvalue2, true)) {
			throw weHaveError("Expected Expression");
		}

		AST ast_init = createAST(AST::Type::sequence);
		AST ast_init_declare = createAST(AST::Type::op_declare);
		AST ast_init_declare_type = createAST(AST::Type::type, "nummy");
		AST ast_init_declare_name = createAST(AST::Type::identifier, name);
		ast_init_declare.children.push_back(ast_init_declare_type);
		ast_init_declare.children.push_back(ast_init_declare_name);
		ast_init.children.push_back(ast_init_declare);
		AST ast_one = createAST(AST::Type::val_nummy, "1");
		if (!is_eq) {
			AST ast_nrvalue1 = createAST(is_up ? AST::Type::op_plus : AST::Type::op_minus);
			ast_nrvalue1.children.push_back(ast_rvalue1);
			ast_nrvalue1.children.push_back(ast_one);
			ast_rvalue1 = ast_nrvalue1;
		}
		AST ast_init_assign = createAST(AST::Type::op_assign);
		ast_init_assign.children.push_back(ast_init_declare_name);
		ast_init_assign.children.push_back(ast_rvalue1);
		ast_init.children.push_back(ast_init_assign);

		AST ast_check = createAST(tokenToAstType[tok_now2.type]);
		ast_check.children.push_back(ast_init_declare_name);
		ast_check.children.push_back(ast_rvalue2);
		AST ast_update = createAST(AST::Type::sequence);
		AST ast_update_change = createAST(is_up ? AST::Type::op_assignplus : AST::Type::op_assignminus);
		ast_update_change.children.push_back(ast_init_declare_name);
		ast_update_change.children.push_back(ast_one);
		ast_update.children.push_back(ast_update_change);

		a_ast->children.push_back(ast_init);
		a_ast->children.push_back(ast_check);
		a_ast->children.push_back(ast_update);

		return true;
	}

	bool Parser::parse_forin_expression(AST *a_ast) {
		*a_ast = createAST(AST::Type::sequence);
		int oldPosition = currentPosition;
		AST ast_type;
		if (!parse_type(&ast_type) || getTokenAt(0).type != Token::Type::identifier) {
			currentPosition = oldPosition;
			return false;
		}
		std::string name = getTokenAt(0).value;
		AST ast_declare = createAST(AST::Type::op_declare);
		AST ast_name = createAST(AST::Type::identifier, name);
		ast_declare.children.push_back(ast_type);
		ast_declare.children.push_back(ast_name);

		currentPosition += 1;

		if (getTokenAt(0).type != Token::Type::kw_in) {
			currentPosition = oldPosition;
			return false;
		}
		currentPosition += 1;

		AST ast_rvalue;
		if (!parse_rvalue(&ast_rvalue, true)) {
			currentPosition = oldPosition;
			return false;
		}

		a_ast->children.push_back(ast_declare);
		a_ast->children.push_back(ast_rvalue);

		return true;
	}

	bool Parser::parse_for_statement() {
		return parse_iflike_statement(Token::Type::kw_for, AST::Type::cs_for, false, true, &Parser::parse_for_expression);
	}

	bool Parser::parse_forin_statement() {
		return parse_iflike_statement(Token::Type::kw_forin, AST::Type::cs_forin, false, true, &Parser::parse_forin_expression);
	}

	bool Parser::parse_if_statement() {
		return parse_iflike_statement(Token::Type::kw_if, AST::Type::cs_if, true, false, &Parser::parse_expression);
	}

	bool Parser::parse_while_statement() {
		return parse_iflike_statement(Token::Type::kw_while, AST::Type::cs_while, false, false, &Parser::parse_expression);
	}

	bool Parser::parse_dowhile_statement() {
		return parse_iflike_statement(Token::Type::kw_dowhile, AST::Type::cs_dowhile, false, false, &Parser::parse_expression);
	}

	bool Parser::parse_iflike_statement(Token::Type a_kwtype, AST::Type a_asttype, bool a_elseblock, bool a_copyexpressionchildren, bool(Parser::*a_parse_expression)(AST*)) {
		std::vector<Token::Type> reqStart = { a_kwtype, Token::Type::op_parenthese_open };
		std::vector<Token::Type> reqEnd = { Token::Type::op_parenthese_close, Token::Type::colon, Token::Type::newline };
		std::vector<Token::Type> reqElse = { Token::Type::kw_else, Token::Type::colon, Token::Type::newline };
		if (!checkTypes(reqStart)) {
			return false;
		}
		currentPosition += 2;

		AST ast_expr;
		if (!(this->*a_parse_expression)(&ast_expr)) {
			throw weHaveError("Iflike statement: Expected expression");
		}
		if (!checkTypes(reqEnd)) {
			throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected ):\\n");
		}
		currentPosition += 3;

		AST ast_true;
		parse_suite(&ast_true, false);
		AST ast_false;
		if (a_elseblock) {
			int oldPosition = currentPosition;
			if (parse_indentation() && checkTypes(reqElse)) {
				currentPosition += 3;
				if (!parse_suite(&ast_false, false)) {
					throw weHaveError("Expected else block");
				}
			} else {
				currentPosition = oldPosition;
				ast_false = createAST(AST::Type::sequence);
			}
		}

		AST ast_if = createAST(a_asttype);
		if (a_copyexpressionchildren) {
			for (AST &c : ast_expr.children) {
				ast_if.children.push_back(c);
			}
		} else {
			ast_if.children.push_back(ast_expr);
		}
		ast_if.children.push_back(ast_true);
		if (a_elseblock) {
			ast_if.children.push_back(ast_false);
		}

		currentASTs.top()->children.push_back(ast_if);

		return true;
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
		AST ast_declare = createAST(AST::Type::op_declare);
		AST ast_name = createAST(AST::Type::identifier, name);
		ast_declare.children.push_back(ast_type);
		ast_declare.children.push_back(ast_name);

		currentASTs.top()->children.push_back(ast_declare);

		if (getTokenAt(1).type == Token::Type::op_equals) {
			if (!parse_assign_statement()) {
				throw weHaveError("Invalid assign statement in declaration");
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
		*a_ast = createAST(AST::Type::identifier, getTokenAt(0).value);
		currentPosition += 1;
		return true;
	}

	bool Parser::parse_lvalue(AST *a_ast) {
		int oldPosition = currentPosition;
		std::stack<AST> values;

		bool stillInLvalue = true;
		while (stillInLvalue) {
			AST ast, ast_list, ast_ident;
			switch (getTokenAt(0).type) {
			case Token::Type::op_access:
				currentPosition += 1;
				if (!parse_identifier(&ast_ident)) {
					throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected identifier");
				}
				ast = createAST(AST::Type::op_access);
				ast.children.push_back(values.top());
				values.pop();
				ast.children.push_back(ast_ident);
				values.push(ast);
				break;
			case Token::Type::op_bracket_open:
				currentPosition += 1;
				ast_list = createAST(AST::Type::list);
				do {
					if (!parse_rvalue(&ast, true)) {
						throw weHaveError("Invalid expression in listaccess");
					}
					ast_list.children.push_back(ast);
					currentPosition += 1;
				} while (getTokenAt(-1).type == Token::Type::comma);
				if (getTokenAt(-1).type != Token::Type::op_bracket_close) {
					throw weHaveError("Unexpected token " + getTokenAt(0).value + "; Expected ]");
				}
				ast = createAST(AST::Type::op_listaccess);
				if (values.empty()) {
					throw weHaveError("Missing lvalue for listaccess");
				}
				ast.children.push_back(values.top());
				values.pop();
				ast.children.push_back(ast_list);
				values.push(ast);
				break;
			case Token::Type::identifier:
				if (!values.empty()) {
					stillInLvalue = false;
					break;
				}
				if (!parse_identifier(&ast)) {
					throw weHaveError("lvalue: Invalid identifier");
				}
				values.push(ast);
				break;
			default:
				stillInLvalue = false;
				break;
			}
		}
		if (values.size() > 1) {
			throw weHaveError("Too many values in lvalue");
		}
		if (values.empty()) {
			return false;
		}
		*a_ast = values.top();
		return true;
	}

	bool Parser::parse_rvalue(AST *a_ast, bool a_allow_expression) {
		if (a_allow_expression && parse_expression(a_ast)) {
			return true;
		}
		switch (getTokenAt(0).type) {
		case Token::Type::val_strry:
			*a_ast = createAST(AST::Type::val_strry, getTokenAt(0).value);
			currentPosition += 1;
			return true;
		case Token::Type::val_nummy:
			*a_ast = createAST(AST::Type::val_nummy, getTokenAt(0).value);
			currentPosition += 1;
			return true;
		case Token::Type::val_fuzzy:
			*a_ast = createAST(AST::Type::val_fuzzy, getTokenAt(0).value);
			currentPosition += 1;
			return true;
		default:;
		}
		return parse_function_call(a_ast) || parse_lvalue(a_ast);
	}

	int Parser::parse(std::vector<Token> &a_tokens) {
		// add buffer at end
		for (int i = 0; i < 7; ++i) {
			a_tokens.push_back(Token(Token::Type::eof, "EOF", -1));
		}

		currentPosition = 0;
		currentTokens = &a_tokens;
		currentASTs.push(&root);

		try {
			while (currentPosition < currentTokens->size()) {
				if (parse_empty_line() || parse_goto() || parse_brainfuck() || parse_class() || parse_function()) {
					continue;
				}
				if (getTokenAt(0).type == Token::Type::eof) {
					break;
				}
				throw weHaveError("Unexpected token " + getTokenAt(0).value);
			}
		} catch (ParserException e) {
			return 1;
		}

		return 0;
	}

}