#include "CBParser.h"

namespace cb {
	AST::AST() {}

	AST::AST(AST::Type a_type, std::string a_value) {
		type = a_type;
		value = a_value;
	}

	AST::~AST() {}

	Parser::Parser() {}

	Parser::~Parser() {}

	AST &Parser::getRootAST() {
		return root;
	}

	enum class ParserState {
		init,
		brainfuck_name,
		brainfuck_types,
		brainfuck_code,
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

	std::string tokenjoin(std::vector<Token> &a_tokens, size_t a_pos, size_t a_cnt) {
		std::string res = "";
		for (size_t i = a_pos; i < a_pos + a_cnt; i++) {
			res += a_tokens[i].value + " ";
		}
		return res;
	}

	int Parser::parse(std::vector<Token> &a_tokens) {
		// add buffer at end
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));
		a_tokens.push_back(Token(Token::Type::newline, "\\n"));

		Token::Type types_brainfuck_name[] = { Token::Type::bf_delimiter };
		Token::Type types_brainfuck_types[] = { Token::Type::bf_delimiter, Token::Type::colon };
		Token::Type types_brainfuck_code[] = { Token::Type::bf_delimiter };
		Token::Type types_classy[] = { Token::Type::identifier, Token::Type::op_inherit, Token::Type::identifier, Token::Type::colon };

		size_t tokensize = a_tokens.size();
		ParserState state = ParserState::init;
		std::stack<AST*> currentASTs;
		std::vector<Token> unusedTokens;
		currentASTs.push(&root);
		int currentIndentation = 0, oldIndentation = 0;
		for (size_t pos = 0; pos < tokensize; pos++) {
			Token t = a_tokens[pos];
			AST &ca = *currentASTs.top();
			AST na;

			if (t.type != Token::Type::indent && t.type != Token::Type::newline) {
				if (currentIndentation < oldIndentation) {
					currentASTs.pop();
					oldIndentation = currentIndentation;
				}
			}

			switch (state) {
			case ParserState::init:
				switch (t.type) {
				case Token::Type::bf_delimiter:
					state = ParserState::brainfuck_name;
					na = AST(AST::Type::function, "");
					addASTElement(na, ca, currentASTs);
					break;
				case Token::Type::kw_classy:
					if (checkTypes(a_tokens, pos + 1, 4, types_classy)) {
						na = AST(AST::Type::classy, a_tokens[pos + 1].value);
						addASTElement(na, ca, currentASTs);
						na = AST(AST::Type::type, a_tokens[pos + 3].value);
						ca.children.push_back(na);
						na = AST(AST::Type::sequence, "");
						addASTElement(na, ca, currentASTs);
						pos += 4;
						oldIndentation = currentIndentation + 1;
						break;
					} else {
						errorpos = pos;
						errorstr = "classy: invalid token " + tokenjoin(a_tokens, pos, 5);
						return 1;
					}
					break;
				case Token::Type::identifier:
					unusedTokens.push_back(t);
					break;
				case Token::Type::indent:
					currentIndentation++;
					break;
				case Token::Type::newline:
					currentIndentation = 0;
					if (!unusedTokens.empty()) {
						errorstr = "init: unused tokens " + tokenjoin(unusedTokens, 0, unusedTokens.size());
					}
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
			default:
				errorpos = pos;
				errorstr = "invalid state - this should not happen";
				return 1;
			}
		}
		return 0;
	}

}