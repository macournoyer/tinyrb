%include {
#include <assert.h>
#include <string.h>
#include "tr.h"
#include "internal.h"
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
root ::= statements(A). { compiler->node = (OBJ) NODE(ROOT, A); }
root ::= statements(A) TERM. { compiler->node = (OBJ) NODE(ROOT, A); }

statements(A) ::= statements(B) TERM statement(C). { A = PUSH(B, C); }
statements(A) ::= statement(B). { A = NODES(B); }

statement(A) ::= expr(B). { A = B; }
statement(A) ::= literal(B). { A = B; }
statement(A) ::= flow(B). { A = B; }
statement(A) ::= ID(B) ASSIGN statement(C). { A = NODE2(ASSIGN, B, C); }

flow(A) ::= IF statement(B) TERM statements(C) TERM END. { A = NODE2(IF, B, C); }
flow(A) ::= UNLESS statement(B) TERM statements(C) TERM END. { A = NODE2(UNLESS, B, C); }

literal(A) ::= SYMBOL(B). { A = NODE(CONST, B); }
literal(A) ::= INT(B). { A = NODE(CONST, B); }
literal(A) ::= STRING(B). { A = NODE(STRING, B); }
literal(A) ::= TRUE. { A = NODE(BOOL, 1); }
literal(A) ::= FALSE. { A = NODE(BOOL, 0); }
literal(A) ::= NIL. { A = NODE(NIL, 0); }

expr(A) ::= expr(B) DOT msg(C). { A = NODE2(SEND, B, C); }
expr(A) ::= literal(B) DOT msg(C). { A = NODE2(SEND, B, C); }
expr(A) ::= msg(B). { A = NODE2(SEND, TR_NIL, B); }

msg(A) ::= ID(B). { A = NODE(MSG, B); }
msg(A) ::= ID(B) O_PAR C_PAR. { A = NODE(MSG, B); }
