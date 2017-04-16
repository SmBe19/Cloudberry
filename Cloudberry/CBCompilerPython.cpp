#include "CBCompilerPython.h"

namespace cb {
	CompilerPython::CompilerPython() {
		errorpos = nullptr;
		errorstr = "";
		indent = 0;
	}
	CompilerPython::~CompilerPython() {}

	CompilerPythonException CompilerPython::weHaveError(AST &a_ast, std::string a_err) {
		errorpos = &a_ast;
		errorstr = a_err;
		return CompilerPythonException();
	}

	std::string CompilerPython::escape_string(std::string &a_str) {
		std::string ns;
		for (char c : a_str) {
			if (c == '\\') {
				ns += "\\\\";
			} else if (c == '"') {
				ns += "\\\"";
			} else if (c == '\n') {
				ns += "\\n";
			} else if (c == '\r') {
				ns += "\\r";
			} else if (c == '\t') {
				ns += "\\t";
			} else {
				ns += c;
			}
		}
		return ns;
	}

	void CompilerPython::printIndent() {
		for (int i = 0; i < indent; i++) {
			fs << "\t";
		}
	}

	void CompilerPython::emptyOrSequence(AST &a_ast) {
		if (a_ast.children.empty()) {
			printIndent();
			fs << "\tpass";
		} else {
			indent++;
			compileNode(a_ast);
			indent--;
		}
	}

	void CompilerPython::op(AST &a_ast, std::string a_op) {
		fs << "(";
		compileNode(a_ast.children[0]);
		fs << ")";
		fs << a_op;
		fs << "(";
		compileNode(a_ast.children[1]);
		fs << ")";
	}

	void CompilerPython::opassign(AST &a_ast, std::string a_op) {
		compileNode(a_ast.children[0]);
		fs << a_op;
		fs << "(";
		compileNode(a_ast.children[1]);
		fs << ")";
	}

	void CompilerPython::compileNode(AST &a_ast) {
		switch (a_ast.type) {
		case AST::Type::sequence:
			for (AST &ast : a_ast.children) {
				printIndent();
				compileNode(ast);
				fs << "\n";
			}
			break;
		case AST::Type::list:
			throw weHaveError(a_ast, "List: Should not happen");
		case AST::Type::classy:
			fs << "class " << a_ast.value;
			if (!a_ast.children[0].value.empty()) {
				fs << " (" << a_ast.children[0].value << ")";
			}
			fs << ":\n";
			emptyOrSequence(a_ast.children[1]);
			break;
		case AST::Type::function:
			fs << "def " << a_ast.value;
			if (!a_ast.children[1].children.empty()) {
				fs << " (";
				bool first = true;
				for (AST &ast : a_ast.children[1].children) {
					if (ast.type != AST::Type::op_declare) {
						throw weHaveError(a_ast, "Function arguments: Should not happen");
					}
					if (!first) {
						fs << ", ";
					}
					fs << ast.children[1].value;
					first = false;
				}
				fs << ")";
			}
			fs << ":\n";
			emptyOrSequence(a_ast.children[2]);
			break;
		case AST::Type::type:
			throw weHaveError(a_ast, "Type: Should not happen");
		case AST::Type::type_generic:
			throw weHaveError(a_ast, "Type generic: Should not happen");
		case AST::Type::identifier:
			fs << a_ast.value;
			break;
		case AST::Type::brainfuck:
			fs << "brainfuck = \"" << a_ast.value << "\"";
			fs << " # There will be brainfuck";
			break;
		case AST::Type::val_nummy:
			fs << a_ast.value;
			break;
		case AST::Type::val_fuzzy:
			fs << a_ast.value;
			break;
		case AST::Type::val_strry:
			fs << "\"" << escape_string(a_ast.value) << "\"";
			break;
		case AST::Type::function_call:
			compileNode(a_ast.children[0]);
			fs << "(";
			{
				bool first = true;
				for (AST &ast : a_ast.children[1].children) {
					if (!first) {
						fs << ", ";
					}
					compileNode(ast);
					first = false;
				}
			}
			fs << ")";
			break;
		case AST::Type::cs_if:
			fs << "if ";
			compileNode(a_ast.children[0]);
			fs << ":\n";
			emptyOrSequence(a_ast.children[1]);
			printIndent();
			fs << "else:\n";
			emptyOrSequence(a_ast.children[2]);
			break;
		case AST::Type::cs_for:
			fs << "\n";
			compileNode(a_ast.children[0]);
			printIndent();
			fs << "while ";
			compileNode(a_ast.children[1]);
			fs << ":\n";
			indent++;
			compileNode(a_ast.children[3]);
			compileNode(a_ast.children[2]);
			indent--;
			break;
		case AST::Type::cs_forin:
			fs << "for ";
			fs << a_ast.children[0].children[1].value;
			fs << " in ";
			compileNode(a_ast.children[1]);
			fs << ":\n";
			emptyOrSequence(a_ast.children[2]);
			break;
		case AST::Type::cs_while:
			fs << "while ";
			compileNode(a_ast.children[0]);
			fs << ":\n";
			emptyOrSequence(a_ast.children[1]);
			break;
		case AST::Type::cs_dowhile:
			compileNode(a_ast.children[1]);
			fs << "while ";
			compileNode(a_ast.children[0]);
			fs << ":\n";
			emptyOrSequence(a_ast.children[1]);
			break;
		case AST::Type::cs_goto:
			fs << "import " << a_ast.value << "\n";
			break;
		case AST::Type::op_access:
			compileNode(a_ast.children[0]);
			fs << ".";
			compileNode(a_ast.children[1]);
			break;
		case AST::Type::op_listaccess:
			compileNode(a_ast.children[0]);
			fs << "[";
			compileNode(a_ast.children[1]);
			fs << "]";
			break;
		case AST::Type::op_plus:
			op(a_ast, "+");
			break;
		case AST::Type::op_minus:
			op(a_ast, "-");
			break;
		case AST::Type::op_not:
			op(a_ast, " not ");
			break;
		case AST::Type::op_bitnot:
			op(a_ast, "~");
			break;
		case AST::Type::op_times:
			op(a_ast, "*");
			break;
		case AST::Type::op_divide:
			op(a_ast, "//");
			break;
		case AST::Type::op_mod:
			op(a_ast, "%");
			break;
		case AST::Type::op_leftshift:
			op(a_ast, "<<");
			break;
		case AST::Type::op_rightshift:
			op(a_ast, ">>");
			break;
		case AST::Type::op_smaller:
			op(a_ast, "<");
			break;
		case AST::Type::op_smallereq:
			op(a_ast, "<=");
			break;
		case AST::Type::op_greater:
			op(a_ast, ">");
			break;
		case AST::Type::op_greatereq:
			op(a_ast, ">=");
			break;
		case AST::Type::op_equals:
			op(a_ast, "==");
			break;
		case AST::Type::op_notequals:
			op(a_ast, "!=");
			break;
		case AST::Type::op_bitand:
			op(a_ast, "&");
			break;
		case AST::Type::op_bitxor:
			op(a_ast, "^");
			break;
		case AST::Type::op_bitor:
			op(a_ast, "|");
			break;
		case AST::Type::op_and:
			op(a_ast, " and ");
			break;
		case AST::Type::op_or:
			op(a_ast, " or ");
			break;
		case AST::Type::op_assign:
			opassign(a_ast, "=");
			break;
		case AST::Type::op_assignplus:
			opassign(a_ast, "+=");
			break;
		case AST::Type::op_assignminus:
			opassign(a_ast, "-=");
			break;
		case AST::Type::op_assigntimes:
			opassign(a_ast, "*=");
			break;
		case AST::Type::op_assigndivide:
			opassign(a_ast, "//=");
			break;
		case AST::Type::op_assignmod:
			opassign(a_ast, "%=");
			break;
		case AST::Type::op_assignleftshift:
			opassign(a_ast, "<<=");
			break;
		case AST::Type::op_assignrightshift:
			opassign(a_ast, ">>=");
			break;
		case AST::Type::op_assignbitand:
			opassign(a_ast, "&=");
			break;
		case AST::Type::op_assignbitxor:
			opassign(a_ast, "^=");
			break;
		case AST::Type::op_assignbitor:
			opassign(a_ast, "|=");
			break;
		case AST::Type::op_assignand:
			opassign(a_ast, "&&=");
			break;
		case AST::Type::op_assignor:
			opassign(a_ast, "||=");
			break;
		case AST::Type::op_declare:
			fs << a_ast.children[1].value;
			fs << " = ";
			if ("nummy" == a_ast.children[0].value) {
				fs << "0";
			} else if ("fuzzy" == a_ast.children[0].value) {
				fs << "0";
			} else if ("charry" == a_ast.children[0].value) {
				fs << "\"\"";
			} else if ("strry" == a_ast.children[0].value) {
				fs << "\"\"";
			} else if ("lissy" == a_ast.children[0].value) {
				fs << "[]";
			} else if ("diccy" == a_ast.children[0].value) {
				fs << "{}";
			} else {
				fs << a_ast.children[0].value << "()";
			}
			break;
		default:
			throw weHaveError(a_ast, "Unknown AST type");
		}
	}


	int CompilerPython::compile(AST &a_ast, std::string &a_file) {
		if (a_file.size() == 0) {
			a_file = CB_DEFAULT_OUT_PYTHON;
		}
		fs = std::ofstream(a_file);

		fs << "#!/bin/python\n";
		fs << "# -*- coding: utf-8 -*-\n";
		fs << "import sys\n";
		fs << "class evvy:\n\tpass\n";

		try {
			compileNode(a_ast);
		} catch (CompilerPythonException e) {
			return 1;
		}

		fs << "cloudberry(len(sys.argv), sys.argv)\n";

		return 0;
	}
}
