%include {
#include <assert.h>
#include <string.h>
#include "tr.h"
}

%name           TrParser
%token_type     { OBJ }
%token_prefix   TR_TOK_
%extra_argument { TrCompiler *compiler }

%token_destructor {
  (void) compiler;
}

%syntax_error {
  printf("Syntax error:\n");
  printf("  %s unexpected at line %d\n", yyTokenName[yymajor], compiler->curline);
}

/*%right ASSIGN.
%left OP.*/

/* rules */
root ::= statements.
root ::= statements TERM.

statements ::= statements TERM statement.
statements ::= statement.

statement ::= literal.
statement ::= statement DOT call.

statement ::= ID(A). { TrCompiler_getlocal(compiler, A); }
statement ::= ID(A) ASSIGN literal(B). { TrCompiler_setlocal(compiler, A, B); }

literal(A) ::= SYMBOL(B). { A = TrCompiler_pushk(compiler, B); }

call(A) ::= ID(B). { A = TrCompiler_call(compiler, B); }
call(A) ::= ID(B) O_PAR C_PAR. { A = TrCompiler_call(compiler, B); }
