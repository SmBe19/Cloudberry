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

	enum class ParserState {
		init,
		function_args,
		goto_args,
		generic_stuff,
		expression,
		for_range,
		forin_type,
		forin_iterable,
		brainfuck_name,
		brainfuck_types,
		brainfuck_code,
	};

	void addASTElement(AST &a_newast, AST &a_parent, std::stack<AST*> &a_currentASTs) {
		a_parent.children.push_back(a_newast);
		a_currentASTs.push(&(a_parent.children.back()));
	}

	bool checkTypes(std::vector<Token> &a_tokens, size_t a_pos, size_t a_cnt, Token::Type a_types[]) {
		for (size_t i = 0; i < a_cnt; i++) {
			if (a_tokens[a_pos + i].type != a_types[i]) {
				return false;
			}
		}
		return true;
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

	int Parser::weHaveError(int a_pos, std::string a_err) {
		errorpos = a_pos;
		errorstr = a_err;
		return 1;
	}

	int Parser::parse(std::vector<Token> &a_tokens) {
		// add buffer at end
		for (int i = 0; i < 7; ++i) {
			a_tokens.push_back(Token(Token::Type::newline, "\\n", -1));
		}

		Token::Type types_brainfuck_stuff[] = { Token::Type::bf_delimiter, Token::Type::colon };
		Token::Type types_classy[] = { Token::Type::identifier, Token::Type::op_inherit, Token::Type::identifier, Token::Type::colon };
		Token::Type types_function_args[] = { Token::Type::op_parenthese_close, Token::Type::colon };

		size_t tokensize = a_tokens.size();
		ParserState state = ParserState::init;
		std::stack<AST*> currentASTs;
		std::stack<ParserState> lastStates;
		std::vector<Token> unusedIdentifiers, unusedOperators;
		std::vector<AST> unusedTypes, unusedExpressions;
		currentASTs.push(&root);
		int currentIndentation = 0, oldIndentation = 0;
		for (size_t pos = 0; pos < tokensize; pos++) {
			Token t = a_tokens[pos];
			AST &ca = *currentASTs.top();
			AST na, naa, naaa, naaaa, naaaaa; // batman

			if (t.type != Token::Type::indent && t.type != Token::Type::newline) {
				while (currentIndentation < oldIndentation) {
					currentASTs.pop();
					oldIndentation--;
				}
				oldIndentation = currentIndentation;
			}

			switch (state) {
			case ParserState::init:
				switch (t.type) {
				case Token::Type::indent:
					currentIndentation++;
					break;
				case Token::Type::newline:
					currentIndentation = 0;
					if (!unusedIdentifiers.empty() || !unusedOperators.empty() || !unusedTypes.empty() || !unusedExpressions.empty()) {
						return weHaveError(pos, "init: unused tokens " + tokenjoin(unusedIdentifiers) + "; " + tokenjoin(unusedOperators) + "; " + tokenjoin(unusedTypes) + "; " + tokenjoin(unusedExpressions));
					}
					break;
				case Token::Type::kw_goto:
					state = ParserState::goto_args;
					break;
				case Token::Type::identifier:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
					}
					unusedIdentifiers.push_back(t);
					if (unusedTypes.size() == 1 && unusedIdentifiers.size() == 1) {
						if (a_tokens[pos + 1].type == Token::Type::op_parenthese_open) {
							// function
							na = AST(AST::Type::function, unusedIdentifiers[0].value);
							ca.children.push_back(na);
							AST &caa = ca.children.back();
							caa.children.push_back(unusedTypes[0]);
							naa = AST(AST::Type::list, "");
							caa.children.push_back(naa);
							na = AST(AST::Type::sequence, "");
							addASTElement(na, caa, currentASTs);
							currentASTs.push(&(caa.children[caa.children.size() - 2]));
							state = ParserState::function_args;
							pos++;
						} else {
							// variable declaration
							na = AST(AST::Type::op_declare, "");
							na.children.push_back(unusedTypes[0]);
							naa = AST(AST::Type::identifier, unusedIdentifiers[0].value);
							na.children.push_back(naa);
							ca.children.push_back(na);
							if (a_tokens[pos + 1].type != Token::Type::newline) {
								unusedExpressions.push_back(naa);
							}
						}
						unusedTypes.clear();
						unusedIdentifiers.clear();
					}
					break;
				case Token::Type::kw_classy:
				{
					std::string baseType = "evvy";
					if (checkTypes(a_tokens, pos + 1, 4, types_classy)) {
						baseType = a_tokens[pos + 3].value;
					} else if (checkTypes(a_tokens, pos + 1, 2, types_classy + 2)) {
					} else {
						return weHaveError(pos, "classy: invalid token " + tokenjoin(a_tokens, pos, 5));
					}
					na = AST(AST::Type::classy, a_tokens[pos + 1].value);
					ca.children.push_back(na);
					AST &caa = ca.children.back();
					naa = AST(AST::Type::type, baseType);
					caa.children.push_back(naa);
					naa = AST(AST::Type::sequence, "");
					addASTElement(naa, caa, currentASTs);
					pos += 4;
					currentIndentation++;
					break;
				}
				case Token::Type::kw_for:
				{
					if (a_tokens[pos + 1].type != Token::Type::op_parenthese_open) {
						return weHaveError(pos, "init: invalid token " + t.value);
					}
					pos += 1;
					bool isForIn = false;
					for (int i = pos;; i++) {
						if (a_tokens[i].type == Token::Type::kw_in) {
							isForIn = true;
							break;
						}
						if (a_tokens[i].type == Token::Type::newline || a_tokens[i].type == Token::Type::op_parenthese_close) {
							break;
						}
					}

					if (isForIn) {
						na = AST(AST::Type::cs_forin, "for");
						addASTElement(na, ca, currentASTs);
						state = ParserState::forin_type;
					} else {
						na = AST(AST::Type::cs_for, "for");
						addASTElement(na, ca, currentASTs);
						lastStates.push(ParserState::for_range);
						state = ParserState::expression;
					}
					break;
				}
				case Token::Type::op_smaller:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						return weHaveError(pos, "init: invalid token " + t.value);
					}
					break;
				case Token::Type::bf_delimiter:
					state = ParserState::brainfuck_name;
					na = AST(AST::Type::function, "");
					addASTElement(na, ca, currentASTs);
					break;
				default:
					return weHaveError(pos, "init: invalid token " + t.value);
				}
				break;
			case ParserState::function_args:
				switch (t.type) {
				case Token::Type::identifier:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
					}
					unusedIdentifiers.push_back(t);
					if (unusedTypes.size() == 1 && unusedIdentifiers.size() == 1) {
						if (a_tokens[pos + 1].type != Token::Type::comma && a_tokens[pos + 1].type != Token::Type::op_parenthese_close) {
							return weHaveError(pos, "function_args: invalid token " + t.value);
						}
						na = AST(AST::Type::op_declare, "");
						na.children.push_back(unusedTypes[0]);
						naa = AST(AST::Type::identifier, unusedIdentifiers[0].value);
						na.children.push_back(naa);
						ca.children.push_back(na);

						unusedTypes.clear();
						unusedIdentifiers.clear();
					}
					break;
				case Token::Type::comma:
					break;
				case Token::Type::op_smaller:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						return weHaveError(pos, "function_args: invalid token " + t.value);
					}
					break;
				case Token::Type::op_parenthese_close:
					if (checkTypes(a_tokens, pos, 2, types_function_args)) {
						currentASTs.pop();
						currentIndentation++;
						state = ParserState::init;
						pos += 1;
					} else {
						return weHaveError(pos, "function_args: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "function_args: invalid token " + t.value);
				}
				break;
			case ParserState::goto_args:
				switch (t.type) {
				case Token::Type::identifier:
					if (a_tokens[pos + 1].type != Token::Type::comma && a_tokens[pos + 1].type != Token::Type::newline) {
						return weHaveError(pos, "goto_args: invalid token " + t.value);
					}
					na = AST(AST::Type::cs_goto, t.value);
					ca.children.push_back(na);
					break;
				case Token::Type::comma:
					break;
				case Token::Type::newline:
					state = ParserState::init;
					break;
				default:
					return weHaveError(pos, "goto_args: invalid token " + t.value);
				}
				break;
			case ParserState::generic_stuff:
				switch (t.type) {
				case Token::Type::identifier:
					if (unusedIdentifiers.empty()) {
						unusedIdentifiers.push_back(t);
					} else {
						return weHaveError(pos, "generic_stuff: invalid token " + t.value);
					}
					break;
				case Token::Type::comma:
					if (unusedIdentifiers.size() == 1) {
						unusedTypes.back().children.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
					} else if (unusedTypes.size() > 2) {
						unusedTypes[unusedTypes.size() - 2].children.push_back(unusedTypes.back());
						unusedTypes.pop_back();
					} else {
						return weHaveError(pos, "generic_stuff: invalid token " + t.value);
					}
					break;
				case Token::Type::op_greater:
					if (unusedIdentifiers.size() == 1) {
						unusedTypes.back().children.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
					} else if (unusedTypes.size() > 2) {
						unusedTypes[unusedTypes.size() - 2].children.push_back(unusedTypes.back());
						unusedTypes.pop_back();
					} else {
						return weHaveError(pos, "generic_stuff: invalid token " + t.value);
					}
					unusedIdentifiers.clear();
					state = lastStates.top();
					lastStates.pop();
					break;
				case Token::Type::op_smaller:
					if (unusedIdentifiers.size() == 1) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						return weHaveError(pos, "generic_stuff: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "generic_stuff: invalid token " + t.value);
				}
				break;
			case ParserState::expression:
				switch (t.type) {
				case Token::Type::identifier:
					break;
				default:
					return weHaveError(pos, "expression: invalid token " + t.value);
				}
				break;
			case ParserState::forin_type:
				switch (t.type) {
				case Token::Type::identifier:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
					}
					unusedIdentifiers.push_back(t);
					if (unusedTypes.size() == 1 && unusedIdentifiers.size() == 1) {
						na = AST(AST::Type::op_declare, "");
						na.children.push_back(unusedTypes[0]);
						naa = AST(AST::Type::identifier, unusedIdentifiers[0].value);
						na.children.push_back(naa);
						ca.children.push_back(na);

						lastStates.push(ParserState::forin_iterable);
						state = ParserState::expression;

						unusedTypes.clear();
						unusedIdentifiers.clear();
					}
					break;
				case Token::Type::op_smaller:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						return weHaveError(pos, "forin_type: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "forin_type: invalid token " + t.value);
				}
				break;
			case ParserState::forin_iterable:
				switch (t.type) {
				case Token::Type::op_parenthese_close:
					if (unusedExpressions.size() == 1 && a_tokens[pos + 1].type == Token::Type::colon) {
						ca.children.push_back(unusedExpressions.back());
						unusedExpressions.clear();
						currentASTs.pop();
						na = AST(AST::Type::sequence, "");
						addASTElement(na, ca, currentASTs);
						pos += 1;
					} else {
						return weHaveError(pos, "forin_iterable: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "forin_iterable: invalid token " + t.value);
				}
				break;
			case ParserState::for_range:
				switch (t.type) {
				case Token::Type::identifier:
					if (unusedIdentifiers.empty()) {
						unusedIdentifiers.push_back(t);
					} else {
						return weHaveError(pos, "for_range: invalid token " + t.value);
					}
					break;
				case Token::Type::op_smaller:
				case Token::Type::op_smallereq:
				case Token::Type::op_greater:
				case Token::Type::op_greatereq:
					unusedOperators.push_back(t);
					if (unusedOperators.size() == 1 && unusedIdentifiers.empty()) {
					} else if (unusedOperators.size() == 2) {
						lastStates.push(state);
						state = ParserState::expression;
					} else {
						return weHaveError(pos, "for_range: invalid token " + t.value);
					}
					break;
				case Token::Type::op_parenthese_close:
					if (unusedExpressions.size() == 2 && unusedOperators.size() == 2 && unusedIdentifiers.size() == 1 && a_tokens[pos + 1].type == Token::Type::colon) {
						bool goUp = (unusedOperators[0].type == Token::Type::op_smaller || unusedOperators[0].type == Token::Type::op_smallereq);
						if (goUp != (unusedOperators[1].type == Token::Type::op_smaller || unusedOperators[1].type == Token::Type::op_smallereq)) {
							return weHaveError(pos, "for_range: operator mismatch");
						}
						naaaaa = AST(AST::Type::sequence, "");
						naaa = AST(AST::Type::val_nummy, "1");
						na = AST(AST::Type::op_declare, "");
						naa = AST(AST::Type::type, "nummy");
						na.children.push_back(naa);
						naa = AST(AST::Type::identifier, unusedIdentifiers[0].value);
						na.children.push_back(naa);
						naaaaa.children.push_back(na);
						na = AST(AST::Type::op_assign, "=");
						na.children.push_back(naa);
						na.children.push_back(unusedExpressions[0]);
						naaaaa.children.push_back(na);
						if (unusedOperators[0].type == Token::Type::op_smaller) {
							na = AST(AST::Type::op_assignplus, "+=");
							na.children.push_back(naa);
							na.children.push_back(naaa);
							naaaaa.children.push_back(na);
						} else if (unusedOperators[0].type == Token::Type::op_greater) {
							na = AST(AST::Type::op_assignminus, "+=");
							na.children.push_back(naa);
							na.children.push_back(naaa);
							naaaaa.children.push_back(na);
						}
						ca.children.push_back(naaaaa);
						switch (unusedOperators[1].type) {
						case Token::Type::op_smaller:
							na = AST(AST::Type::op_smaller, "<");
							break;
						case Token::Type::op_smallereq:
							na = AST(AST::Type::op_smallereq, "<=");
							break;
						case Token::Type::op_greater:
							na = AST(AST::Type::op_greater, ">");
							break;
						case Token::Type::op_greatereq:
							na = AST(AST::Type::op_greatereq, ">=");
							break;
						default:
							return weHaveError(pos, "for_range: operator mismatch");
						}
						na.children.push_back(naa);
						na.children.push_back(unusedExpressions[1]);
						ca.children.push_back(na);

						if (goUp) {
							na = AST(AST::Type::op_assignplus, "+=");
						} else {
							na = AST(AST::Type::op_assignminus, "-=");
						}
						na.children.push_back(naa);
						na.children.push_back(naaa);
						ca.children.push_back(na);

						pos += 1;
					} else {
						return weHaveError(pos, "for_range: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "for_range: invalid token " + t.value);
				}
				break;
			case ParserState::brainfuck_name:
				switch (t.type) {
				case Token::Type::bf_funcname:
					ca.value = t.value;
					if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_stuff)) {
						na = AST(AST::Type::list, "");
						addASTElement(na, ca, currentASTs);
						state = ParserState::brainfuck_types;
						pos++;
					} else {
						return weHaveError(pos, "brainfuck_name: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "brainfuck_name: invalid token " + t.value);
				}
				break;
			case ParserState::brainfuck_types:
				switch (t.type) {
				case Token::Type::bf_type:
					na = AST(AST::Type::type, t.value);
					ca.children.push_back(na);
					if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_stuff)) {
						currentASTs.pop();
						state = ParserState::brainfuck_code;
						pos++;
					} else if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_stuff + 1)) {
						pos++;
					} else {
						return weHaveError(pos, "brainfuck_types: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "brainfuck_types: invalid token " + t.value);
				}
				break;
			case ParserState::brainfuck_code:
				switch (t.type) {
				case Token::Type::bf_code:
					na = AST(AST::Type::brainfuck, t.value);
					ca.children.push_back(na);
					if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_stuff)) {
						currentASTs.pop();
						state = ParserState::init;
						pos++;
					} else {
						return weHaveError(pos, "brainfuck_code: invalid token " + t.value);
					}
					break;
				default:
					return weHaveError(pos, "brainfuck_code: invalid token " + t.value);
				}
				break;
			default:
				return weHaveError(pos, "invalid state - this should not happen");
			}
		}
		return 0;
	}

}