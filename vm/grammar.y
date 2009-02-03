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

statements ::= statements TERM statement.
statements ::= statement.

statement ::= call.

call ::= ID(A). { TrCompiler_call(compiler, A); }
call ::= ID(A) O_PAR C_PAR. { TrCompiler_call(compiler, A); }
