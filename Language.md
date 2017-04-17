# Cloudberry Language Specification

## Entry point
All programs start at the function with the signature `nummy cloudberry (nummy argc, lissy<strry> argv)

## Types
The following types are available:

 - `nully`: nothing (e.g. no return value)
 - `evvy`: base class for everything
 - `charry`: 8 bit signed interger
 - `nummy`: 64 bit signed integer
 - `fuzzy`: 64 bit floating point
 - `strry`: string
 - `lissy<type>`: list (e.g. `lissy<charry>`)
 - `diccy<type, type>`: dictionary (e.g. `diccy<charry, nummy>`)

## Code Blocks
Code blocks are partitioned using indentation (1 tab per level).

## Brainfuck
The following syntax allows inline brainfuck: `$function_name$return_type:input_type:...$BF_CODE$`
Using `,` the arguments can be read, in little endian and the order in which they appear in the arguments. The return value has to be given using `.` in little endian.

## Identifiers
Identifier names (variables, functions, classes) have to be match the following regex: `[A-Za-z_][A-Za-z1-9_]*`.

## Variables
A variable has to be declared before it can used using the following syntax: `type name`.

## Functions
Functions are defined using the following syntax:

```
return_type function_name (argument_type argument_name, ...):
	CODE_BLOCK
```

A function is called using `function_name(expression, ...)`

## Control Structures
The following control structures are allowed:

```
hmm (expression):
	CODE_BLOCK
```

```
hmm (expression):
	CODE_BLOCK
elsa (expression):
  CODE_BLOCK
watevs:
	CODE_BLOCK
```

```
gogo (type i in list):
	CODE_BLOCK
```

```
go (0 <= i < n):
	CODE_BLOCK
```

```
keepitup (expression):
	CODE_BLOCK
```

```
dokeepitup (expression):
	CODE_BLOCK
```

## Classes
A class can be defined with the following syntax:

```
classy class_name -> base_class:
	CODE_BLOCK
```

Inside a class functions and variables can be defined. To access a class member the syntax is `obj.class_member`.

All classes inherit from `evvy`. All types are classes themselves.

## Operators
The following operators are available and in the given precedence

1. () [] .
2. + (unary) - (unary) ! ~ (type) (casting)
3. * / %
4. + -
5. << >>
6. < <= > >=
7. == !=
8. &
9. ^
10. |
11. &&
12. ||
13. = += -= *= /= %= <<= >>= &= ^= |=

## Keywords
The following words are keywords:

 - All types
 - `if`
 - `for`
 - `in`
 - `while`
 - `dowhile`
 - `classy`

## List access
To access an element in a lissy or diccy `variable[index]` is used.

## import modules
Other modules can be imported using `goto module_name`.

## Comments
Line comments start with `#` or `//`. Block comments are contained within `/*` and `*/`.
