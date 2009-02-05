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

statement(A) ::= expr(B). { A = B; }
statement(A) ::= literal(B). { A = B; }
statement(A) ::= ID(B) ASSIGN statement(C). { A = TrCompiler_setlocal(compiler, B, C); }

literal(A) ::= SYMBOL(B). { A = TrCompiler_pushk(compiler, B); }

expr(A) ::= expr DOT call(B). { A = B; }
expr(A) ::= literal DOT call(B). { A = B; }
expr(A) ::= call(B). { A = B; }

call(A) ::= ID(B). { A = TrCompiler_call(compiler, B); }
call(A) ::= ID(B) O_PAR C_PAR. { A = TrCompiler_call(compiler, B); }
