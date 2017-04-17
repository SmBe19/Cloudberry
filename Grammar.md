# Cloudberry Grammar
```
PROGRAM = { GOTO_STATEMENT | CLASS | FUNCTION | BRAINFUCK};
CHAR = "A" | … | "Z" | "a" | … | "z";
DECIMAL = "0" | … | "9";
NUMBER = ["-"], "1" | … | "9", {DECIMAL};
FLOAT = ["-"], NUMBER, ".", {DECIMAL};
LVALUE = IDENTIFIER | LISTACCESS | CLASSACCESS;
RVALUE = IDENTIFIER | LISTACCESS | CLASSACCESS | NUMBER | FLOAT | STRING | FUNCTION_CALL | EXPRESSION;
IDENTIFIER = CHAR | "_", { CHAR | DECIMAL | "_" };
TYPE = "nully" | "evvy" | "charry" | "nummy" | "fuzzy" | "strry" | LISSY | DICCY | IDENTIFIER;
LISSY = "lissy", "<", TYPE, ">";
DICCY = "diccy", "<", TYPE, ",", TYPE ">";
LISTACCESS = LVALUE, "[", RVALUE, "]";
CLASS = "classy", IDENTIFIER, ["->", IDENTIFIER], ":", SUITE;
CLASSACCESS = LVALUE | CLASSACCESS, ".", IDENTIFIER;
FUNCTION = TYPE, IDENTIFIER, "(", [TYPE, IDENTIFIER, {",", TYPE, IDENTIFIER}], ")", ":", SUITE;
BRAINFUCK = "$", IDENTIFIER, "$", TYPE, {":", TYPE}, "$", BRAINFUCK_CODE, "$";
BRAINFUCK_CODE = {CHAR | DECIMAL | "+", "-", "<", ">", "[", "]", ".", ","};
SUITE = NEWLINE, INDENT, {STATEMENT | CLASS | FUNCTION}, DEDENT;
DECLARATION = TYPE, IDENTIFIER;
IF_STATEMENT = "hmm", "(", EXPRESSION, ")", ":", SUITE, { "elsa", "(",  EXPRESSION, ")", ":", SUITE }, ["watevs", ":", SUITE];
FOR_STATEMENT = FOR_UP_STATEMENT | FOR_DOWN_STATEMENT | FOR_IN_STATEMENT;
FOR_UP_STATEMENT = "go", "(", RVALUE, ")", "<" | "<=", IDENTIFIER, "<" | "<=", RVALUE, ")", ":", SUITE;
FOR_DOWN_STATEMENT = "go", "(", RVALUE, ")", ">" | ">=", IDENTIFIER, ">" | ">=", RVALUE, ")", ":", SUITE;
FOR_IN_STATEMENT = "gogo", "(", TYPE, IDENTIFIER, "in", RVALUE, ")", ":", SUITE;
WHILE_STATEMENT = "keepitup", "(", EXPRESSION, ")", ":", SUITE;
DOWHILE_STATEMENT = "dokeepitup", "(", EXPRESSION, ")", ":", SUITE;
GOTO_STATEMENT = "goto", IDENTIFIER, {",", IDENTIFIER};
STATEMENT = SIMPLE_STATEMENT | COMPOUND_STATEMENT;
SIMPLE_STATEMENT = ASSIGN_STATEMENT | EXPRESSION, NEWLINE;
COMPOUND_STATEMENT = FOR_STATEMENT | IF_STATEMENT | WHILE_STATEMENT | DOWHILE_STATEMENT;
ASSIGN_STATEMENT = DECLARATION | (DECLARATION | LVALUE, ASSIGN_OPERATOR, EXPRESSION);
EXPRESSION = ["("], RVALUE, {OPERATOR, RVALUE}, [")"];
OPERATOR = BINARY_OPERATOR | ASSIGN_OPERATOR;
BINARY_OPERATOR = "+" | "-" | "*" | "/" | "%" | "!" | "~" | "<<" | ">>" | "<" | "<=" | ">" | ">=" | "==" | "!=" | "&" | "^" | "|" | "&&" | "||";
ASSIGN_OPERATOR = "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "<<=" | ">>=" | "&=" | "^=" | "|=" | "&&=" | "||=";
FUNCTION_CALL = IDENTIFIER | CLASSACCESS, "(", [EXPRESSION, {",", EXPRESSION}], ")";
```
