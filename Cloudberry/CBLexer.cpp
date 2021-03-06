#include "CBLexer.h"

namespace cb {
	Token::Token() {}

	Token::Token(Token::Type a_type, std::string a_value, int a_line) {
		type = a_type;
		value = a_value;
		line = a_line;
	}

	Token::~Token() {}

	Lexer::Lexer() {
		identifierToToken["hmm"] = Token::Type::kw_if;
		identifierToToken["elsa"] = Token::Type::kw_elif;
		identifierToToken["watevs"] = Token::Type::kw_else;
		identifierToToken["go"] = Token::Type::kw_for;
		identifierToToken["gogo"] = Token::Type::kw_forin;
		identifierToToken["in"] = Token::Type::kw_in;
		identifierToToken["keepitup"] = Token::Type::kw_while;
		identifierToToken["dokeepitup"] = Token::Type::kw_dowhile;
		identifierToToken["classy"] = Token::Type::kw_classy;
		identifierToToken["goto"] = Token::Type::kw_goto;
		/*
		identifierToToken["nully"] = Token::Type::type_nully;
		identifierToToken["evvy"] = Token::Type::type_evvy;
		identifierToToken["charry"] = Token::Type::type_charry;
		identifierToToken["nummy"] = Token::Type::type_nummy;
		identifierToToken["fuzzy"] = Token::Type::type_fuzzy;
		identifierToToken["strry"] = Token::Type::type_strry;
		identifierToToken["lissy"] = Token::Type::type_lissy;
		identifierToToken["diccy"] = Token::Type::type_diccy;
		*/

		operatorToToken["+"] = Token::Type::op_plus;
		operatorToToken["-"] = Token::Type::op_minus;
		operatorToToken["!"] = Token::Type::op_not;
		operatorToToken["~"] = Token::Type::op_bitnot;
		operatorToToken["*"] = Token::Type::op_times;
		operatorToToken["/"] = Token::Type::op_divide;
		operatorToToken["%"] = Token::Type::op_mod;
		operatorToToken["<<"] = Token::Type::op_leftshift;
		operatorToToken[">>"] = Token::Type::op_rightshift;
		operatorToToken["<"] = Token::Type::op_smaller;
		operatorToToken["<="] = Token::Type::op_smallereq;
		operatorToToken[">"] = Token::Type::op_greater;
		operatorToToken[">="] = Token::Type::op_greatereq;
		operatorToToken["=="] = Token::Type::op_equals;
		operatorToToken["!="] = Token::Type::op_notequals;
		operatorToToken["&"] = Token::Type::op_bitand;
		operatorToToken["^"] = Token::Type::op_bitxor;
		operatorToToken["|"] = Token::Type::op_bitor;
		operatorToToken["&&"] = Token::Type::op_and;
		operatorToToken["||"] = Token::Type::op_or;
		operatorToToken["="] = Token::Type::op_assign;
		operatorToToken["+="] = Token::Type::op_assignplus;
		operatorToToken["-="] = Token::Type::op_assignminus;
		operatorToToken["*="] = Token::Type::op_assigntimes;
		operatorToToken["/="] = Token::Type::op_assigndivide;
		operatorToToken["%="] = Token::Type::op_assignmod;
		operatorToToken["<<="] = Token::Type::op_assignleftshift;
		operatorToToken[">>="] = Token::Type::op_assignrightshift;
		operatorToToken["&="] = Token::Type::op_assignbitand;
		operatorToToken["^="] = Token::Type::op_assignbitor;
		operatorToToken["|="] = Token::Type::op_assignbitor;
		operatorToToken["&&="] = Token::Type::op_assignand;
		operatorToToken["||="] = Token::Type::op_assignor;
		operatorToToken["."] = Token::Type::op_access;
		operatorToToken["->"] = Token::Type::op_inherit;

		indent = "";
		foundIndent = false;
	}

	Lexer::~Lexer() {}

	std::vector<Token> &Lexer::getTokens() {
		return tokens;
	}

	enum class LexerState {
		init,
		maybecomment,
		linecomment,
		blockcomment,
		blockcommentmaybeend,
		op,
		identifier,
		strry,
		strry_escape,
		strry_escape_num,
		nummy,
		fuzzy,
		bf_funcname,
		bf_type,
		bf_code,
	};

	int Lexer::weHaveError(int a_pos, std::string a_errorstr) {
		errorpos = a_pos;
		errorstr = a_errorstr;
		return 1;
	}


	int Lexer::lex(std::string a_code, int line) {
		// add buffer at end
		a_code += "\n";
		size_t codesize = a_code.size();
		LexerState state = LexerState::init;
		std::vector<Token> newTokens;
		std::string value = "", tmp_value = "";
		bool isLineStart = true;
		for (size_t pos = 0; pos < codesize; pos++) {
			char c = a_code[pos];
			if (c == '\r') {
				continue;
			}
			switch (state) {
			case LexerState::init:
				if (isLineStart) {
					if (c == ' ' || c == '\t') {
						value += c;
					}
					if (foundIndent) {
						if (c == ' ' && indent == "\t" || c == '\t' && indent[0] == ' ') {
							return weHaveError(pos, "Mixed tabs and spaces");
						}
						if (value == indent) {
							newTokens.push_back(Token(Token::Type::indent, "\t", line));
							value = "";
						}
					} else {
						if (c != ' ' && c != '\t' && !value.empty()) {
							indent = value;
							value = "";
							foundIndent = true;
							newTokens.push_back(Token(Token::Type::indent, "\t", line));
							for (int i = 0; i < indent.size(); i++) {
								if (indent[0] != indent[i]) {
									return weHaveError(pos, "Mixed tabs and spaces");
								}
							}
						}
					}
					if (c != ' ' && c != '\t') {
						isLineStart = false;
						if (!value.empty()) {
							return weHaveError(pos, "Unused alignment");
						}
					} else {
						continue;
					}
				}
				if (c == ' ') {
				} else if (c == '\t') {
					if (!isLineStart) {
						return weHaveError(pos, "Tabs not at line start");
					}
					newTokens.push_back(Token(Token::Type::indent, "\t", line));
				} else if (c == '\n') {
					isLineStart = true;
					value = "";
				} else if (c == ':') {
					newTokens.push_back(Token(Token::Type::colon, ":", line));
				} else if (c == ',') {
					newTokens.push_back(Token(Token::Type::comma, ",", line));
				} else if (c == '(') {
					newTokens.push_back(Token(Token::Type::op_parenthese_open, "(", line));
				} else if (c == ')') {
					newTokens.push_back(Token(Token::Type::op_parenthese_close, ")", line));
				} else if (c == '[') {
					newTokens.push_back(Token(Token::Type::op_bracket_open, "[", line));
				} else if (c == ']') {
					newTokens.push_back(Token(Token::Type::op_bracket_close, "]", line));
				} else if (c == '"') {
					value = "";
					state = LexerState::strry;
				} else if (c == '#') {
					state = LexerState::linecomment;
				} else if (c == '/') {
					state = LexerState::maybecomment;
				} else if (c == '$') {
					value = "";
					newTokens.push_back(Token(Token::Type::bf_delimiter, "$", line));
					state = LexerState::bf_funcname;
				} else if (c == '_' || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
					value = std::string({ c });
					state = LexerState::identifier;
				} else if ('0' <= c && c <= '9') {
					value = std::string({ c });
					state = LexerState::nummy;
				} else if (operatorChars.find(c) != std::string::npos) {
					value = std::string({ c });
					state = LexerState::op;
				} else {
					return weHaveError(pos, "invalid character");
				}
				break;
			case LexerState::maybecomment:
				if (c == '*') {
					state = LexerState::blockcomment;
				} else if (c == '/') {
					state = LexerState::linecomment;
				} else {
					value = "/";
					state = LexerState::op;
					pos--;
				}
				break;
			case LexerState::linecomment:
				if (c == '\n') {
					state = LexerState::init;
				}
				break;
			case LexerState::blockcomment:
				if (c == '*') {
					state = LexerState::blockcommentmaybeend;
				}
				break;
			case LexerState::blockcommentmaybeend:
				if (c == '/') {
					state = LexerState::init;
				} else {
					state = LexerState::blockcomment;
				}
				break;
			case LexerState::op:
				if (operatorChars.find(c) != std::string::npos) {
					value += c;
				} else {
					Token::Type tokentype = Token::Type::identifier;
					if (operatorToToken.find(value) != operatorToToken.end()) {
						tokentype = operatorToToken[value];
					}
					if (tokentype == Token::Type::identifier) {
						return weHaveError(pos, "unknown operator " + value);
					}
					newTokens.push_back(Token(tokentype, value, line));
					state = LexerState::init;
					pos--;
				}
				break;
			case LexerState::identifier:
				if (c == '_' || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) {
					value += c;
				} else {
					Token::Type tokentype = Token::Type::identifier;
					if (identifierToToken.find(value) != identifierToToken.end()) {
						tokentype = identifierToToken[value];
					}
					newTokens.push_back(Token(tokentype, value, line));
					state = LexerState::init;
					pos--;
				}
				break;
			case LexerState::strry:
				if (c == '\\') {
					state = LexerState::strry_escape;
				} else if (c == '"') {
					newTokens.push_back(Token(Token::Type::val_strry, value, line));
					state = LexerState::init;
				} else if (c == '\n') {
					return weHaveError(pos, "strry value not terminated");
				} else {
					value += c;
				}
				break;
			case LexerState::strry_escape:
				state = LexerState::strry;
				if (c == '\\') {
					value += "\\";
				} else if (c == '"') {
					value += "\"";
				} else if (c == 'n') {
					value += "\n";
				} else if (c == 'r') {
					value += "\r";
				} else if (c == 't') {
					value += "\t";
				} else if (c == 'x') {
					state = LexerState::strry_escape_num;
					tmp_value = "";
				}
				break;
			case LexerState::strry_escape_num:
				if ('A' <= c && c <= 'F') {
					c -= 'A' - 'a';
				}
				if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f')) {
					tmp_value += c;
				} else {
					return weHaveError(pos, "invalid character in \\xhh escape sequence");
				}
				if (tmp_value.size() == 2) {
					int cval = 0;
					for (size_t i = 0; i < tmp_value.size(); i++) {
						cval <<= 4;
						char atmp = tmp_value[i];
						if ('0' <= atmp && atmp <= '9') {
							cval += atmp - '0';
						} else if ('a' <= atmp && atmp <= 'f') {
							cval += atmp - 'a' + 10;
						} else {
							return weHaveError(pos, "invalid character in \\xhh escape sequence");
						}
					}
					value += (char)cval;
					state = LexerState::strry;
				}
				break;
			case LexerState::nummy:
				if ('0' <= c && c <= '9') {
					value += c;
				} else if (c == '.') {
					value += c;
					state = LexerState::fuzzy;
				} else if (c == 'x' && !value.compare("0")) {
					value += c;
				} else {
					newTokens.push_back(Token(Token::Type::val_nummy, value, line));
					state = LexerState::init;
					pos--;
				}
				break;
			case LexerState::fuzzy:
				if ('0' <= c && c <= '9') {
					value += c;
				} else {
					newTokens.push_back(Token(Token::Type::val_fuzzy, value, line));
					state = LexerState::init;
					pos--;
				}
				break;
			case LexerState::bf_funcname:
				if (c == '_' || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9' && value.size() == 0)) {
					value += c;
				} else if (c == '$') {
					newTokens.push_back(Token(Token::Type::bf_funcname, value, line));
					value = "";
					newTokens.push_back(Token(Token::Type::bf_delimiter, "$", line));
					state = LexerState::bf_type;
				} else {
					return weHaveError(pos, "invalid character in brainfuck function name");
				}
				break;
			case LexerState::bf_type:
				if (c == '_' || ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9' && value.size() == 0)) {
					value += c;
				} else if (c == '$') {
					newTokens.push_back(Token(Token::Type::bf_type, value, line));
					value = "";
					newTokens.push_back(Token(Token::Type::bf_delimiter, "$", line));
					state = LexerState::bf_code;
				} else if (c == ':') {
					newTokens.push_back(Token(Token::Type::bf_type, value, line));
					value = "";
					newTokens.push_back(Token(Token::Type::colon, ":", line));
				} else {
					return weHaveError(pos, "invalid character in brainfuck function name");
				}
				break;
				break;
			case LexerState::bf_code:
				if (c == '$') {
					newTokens.push_back(Token(Token::Type::bf_code, value, line));
					value = "";
					newTokens.push_back(Token(Token::Type::bf_delimiter, "$", line));
					state = LexerState::init;
				} else {
					value += c;
				}
				break;
			default:
				return weHaveError(pos, "invalid state - this should not happen");
			}
			if (c == '\n' || pos == codesize - 1) {
				if (state != LexerState::init) {
					return weHaveError(pos, "invalid end state");
				}
				bool anythingelse = false;
				for (Token token : newTokens) {
					if (token.type != Token::Type::indent) {
						anythingelse = true;
						break;
					}
				}
				newTokens.push_back(Token(Token::Type::newline, "\\n", line));
				line++;
				if (anythingelse) {
					tokens.insert(tokens.end(), newTokens.begin(), newTokens.end());
					newTokens.clear();
				}
			}
		}
		return 0;
	}
}