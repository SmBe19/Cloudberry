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
		brainfuck_name,
		brainfuck_types,
		brainfuck_code,
		function_args,
		goto_args,
		generic_stuff,
	};

	void addASTElement(AST &a_newast, AST &a_parent, std::stack<AST*> &a_currentASTs) {
		a_parent.children.push_back(a_newast);
		a_currentASTs.push(&(a_parent.children[a_parent.children.size() - 1]));
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

	int Parser::parse(std::vector<Token> &a_tokens) {
		// add buffer at end
		for (int i = 0; i < 7; ++i) {
			a_tokens.push_back(Token(Token::Type::newline, "\\n", -1));
		}

		Token::Type types_brainfuck_name[] = { Token::Type::bf_delimiter };
		Token::Type types_brainfuck_types[] = { Token::Type::bf_delimiter, Token::Type::colon };
		Token::Type types_brainfuck_code[] = { Token::Type::bf_delimiter };
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
				case Token::Type::colon:
					break;
				case Token::Type::comma:
					break;
				case Token::Type::newline:
					currentIndentation = 0;
					if (!unusedIdentifiers.empty() || !unusedOperators.empty() || !unusedTypes.empty() || !unusedExpressions.empty()) {
						errorpos = pos;
						errorstr = "init: unused tokens " + tokenjoin(unusedIdentifiers) + "; " + tokenjoin(unusedOperators) + "; " + tokenjoin(unusedTypes) + "; " + tokenjoin(unusedExpressions);
						return 1;
					}
					break;
				case Token::Type::bf_delimiter:
					state = ParserState::brainfuck_name;
					na = AST(AST::Type::function, "");
					addASTElement(na, ca, currentASTs);
					break;
				case Token::Type::val_strry:
					break;
				case Token::Type::val_nummy:
					break;
				case Token::Type::val_fuzzy:
					break;
				case Token::Type::kw_if:
					break;
				case Token::Type::kw_for:
					break;
				case Token::Type::kw_in:
					break;
				case Token::Type::kw_while:
					break;
				case Token::Type::kw_dowhile:
					break;
				case Token::Type::kw_classy:
				{
					std::string baseType = "evvy";
					if (checkTypes(a_tokens, pos + 1, 4, types_classy)) {
						baseType = a_tokens[pos + 3].value;
					} else if (checkTypes(a_tokens, pos + 1, 2, types_classy + 2)) {
					} else {
						errorpos = pos;
						errorstr = "classy: invalid token " + tokenjoin(a_tokens, pos, 5);
						return 1;
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
				case Token::Type::kw_goto:
					state = ParserState::goto_args;
					break;
				case Token::Type::op_parenthese_open:
					break;
				case Token::Type::op_parenthese_close:
					break;
				case Token::Type::op_bracket_open:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						errorpos = pos;
						errorstr = "init: invalid token " + t.value;
						return 1;
					}
					break;
				case Token::Type::op_bracket_close:
					break;
				case Token::Type::op_plus:
					break;
				case Token::Type::op_minus:
					break;
				case Token::Type::op_not:
					break;
				case Token::Type::op_bitnot:
					break;
				case Token::Type::op_times:
					break;
				case Token::Type::op_divide:
					break;
				case Token::Type::op_mod:
					break;
				case Token::Type::op_leftshift:
					break;
				case Token::Type::op_rightshift:
					break;
				case Token::Type::op_smaller:
					break;
				case Token::Type::op_smallereq:
					break;
				case Token::Type::op_greater:
					break;
				case Token::Type::op_greatereq:
					break;
				case Token::Type::op_equals:
					break;
				case Token::Type::op_notequals:
					break;
				case Token::Type::op_bitand:
					break;
				case Token::Type::op_bitxor:
					break;
				case Token::Type::op_bitor:
					break;
				case Token::Type::op_and:
					break;
				case Token::Type::op_or:
					break;
				case Token::Type::op_assign:
					break;
				case Token::Type::op_assignplus:
					break;
				case Token::Type::op_assignminus:
					break;
				case Token::Type::op_assigntimes:
					break;
				case Token::Type::op_assigndivide:
					break;
				case Token::Type::op_assignmod:
					break;
				case Token::Type::op_assignleftshift:
					break;
				case Token::Type::op_assignrightshift:
					break;
				case Token::Type::op_assignbitand:
					break;
				case Token::Type::op_assignbitxor:
					break;
				case Token::Type::op_assignbitor:
					break;
				case Token::Type::op_access:
					break;
				case Token::Type::op_inherit:
					break;
				default:
					errorpos = pos;
					errorstr = "init: invalid token " + t.value;
					return 1;
				}
				break;
			case ParserState::brainfuck_name:
				switch (t.type) {
				case Token::Type::bf_funcname:
					ca.value = t.value;
					if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_name)) {
						na = AST(AST::Type::list, "");
						addASTElement(na, ca, currentASTs);
						state = ParserState::brainfuck_types;
						pos++;
						break;
					}
					// fall through
				default:
					errorpos = pos;
					errorstr = "brainfuck_name: invalid token " + t.value;
					return 1;
				}
				break;
			case ParserState::brainfuck_types:
				switch (t.type) {
				case Token::Type::bf_type:
					na = AST(AST::Type::type, t.value);
					ca.children.push_back(na);
					if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_types)) {
						currentASTs.pop();
						state = ParserState::brainfuck_code;
						pos++;
						break;
					} else if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_types + 1)) {
						pos++;
						break;
					}
					// fall through
				default:
					errorpos = pos;
					errorstr = "brainfuck_types: invalid token " + t.value;
					return 1;
				}
				break;
			case ParserState::brainfuck_code:
				switch (t.type) {
				case Token::Type::bf_code:
					na = AST(AST::Type::brainfuck, t.value);
					ca.children.push_back(na);
					if (checkTypes(a_tokens, pos + 1, 1, types_brainfuck_code)) {
						currentASTs.pop();
						state = ParserState::init;
						pos++;
						break;
					}
					// fall through
				default:
					errorpos = pos;
					errorstr = "brainfuck_code: invalid token " + t.value;
					return 1;
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
							errorpos = pos;
							errorstr = "function_args: invalid token " + t.value;
							return 1;
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
				case Token::Type::op_bracket_open:
					if (!unusedIdentifiers.empty()) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						errorpos = pos;
						errorstr = "function_args: invalid token " + t.value;
						return 1;
					}
					break;
				case Token::Type::op_parenthese_close:
					if (checkTypes(a_tokens, pos, 2, types_function_args)) {
						currentASTs.pop();
						currentIndentation++;
						state = ParserState::init;
						break;
					}
					// fall through
				default:
					errorpos = pos;
					errorstr = "function_args: invalid token " + t.value;
					return 1;
				}
				break;
			case ParserState::goto_args:
				switch (t.type) {
				case Token::Type::identifier:
					if (a_tokens[pos + 1].type != Token::Type::comma && a_tokens[pos + 1].type != Token::Type::newline) {
						errorpos = pos;
						errorstr = "goto_args: invalid token " + t.value;
						return 1;
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
					errorpos = pos;
					errorstr = "goto_args: invalid token " + t.value;
					return 1;
				}
				break;
			case ParserState::generic_stuff:
				switch (t.type) {
				case Token::Type::identifier:
					if (unusedIdentifiers.empty()) {
						unusedIdentifiers.push_back(t);
					} else {
						errorpos = pos;
						errorstr = "generic_stuff: invalid token " + t.value;
						return 1;
					}
					break;
				case Token::Type::comma:
					if (unusedIdentifiers.size() == 1) {
						unusedTypes.back().children.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
					} else if (unusedTypes.size() > 2) {
						unusedTypes[unusedTypes.size() - 2].children.push_back(unusedTypes.back());
						unusedTypes.pop_back();
					} else {
						errorpos = pos;
						errorstr = "generic_stuff: invalid token " + t.value;
						return 1;
					}
					break;
				case Token::Type::op_bracket_close:
					if (unusedIdentifiers.size() == 1) {
						unusedTypes.back().children.push_back(AST(AST::Type::type, unusedIdentifiers[0].value));
					} else if (unusedTypes.size() > 2) {
						unusedTypes[unusedTypes.size() - 2].children.push_back(unusedTypes.back());
						unusedTypes.pop_back();
					} else {
						errorpos = pos;
						errorstr = "generic_stuff: invalid token " + t.value;
						return 1;
					}
					unusedIdentifiers.clear();
					state = lastStates.top();
					lastStates.pop();
					break;
				case Token::Type::op_bracket_open:
					if (unusedIdentifiers.size() == 1) {
						unusedTypes.push_back(AST(AST::Type::type_generic, unusedIdentifiers[0].value));
						unusedIdentifiers.pop_back();
						lastStates.push(state);
						state = ParserState::generic_stuff;
					} else {
						errorpos = pos;
						errorstr = "generic_stuff: invalid token " + t.value;
						return 1;
					}
					break;
				default:
					errorpos = pos;
					errorstr = "generic_stuff: invalid token " + t.value;
					return 1;
				}
				break;
			default:
				errorpos = pos;
				errorstr = "invalid state - this should not happen";
				return 1;
			}
		}
		return 0;
	}

}