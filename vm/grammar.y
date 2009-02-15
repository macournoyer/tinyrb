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
  tr_raise("Syntax error:\n"
           "  %s unexpected at line %d\n", yyTokenName[yymajor], compiler->curline);
}

%right ASSIGN.

/* rules */
root ::= statements(A). { compiler->node = (OBJ) NODE(ROOT, A); }
root ::= statements(A) TERM. { compiler->node = (OBJ) NODE(ROOT, A); }

statements(A) ::= statements(B) TERM statement(C). { A = PUSH(B, C); }
statements(A) ::= statement(B). { A = NODES(B); }
statements(A) ::= . { A = TrArray_new(compiler->vm); }

statement(A) ::= expr_out(B). { A = B; }
statement(A) ::= flow(B). { A = B; }
statement(A) ::= def(B). { A = B; }
statement(A) ::= class(B). { A = B; }
statement(A) ::= assign_out(B). { A = B; }

flow(A) ::= IF statement(B) TERM statements(C) opt_term else(D) END. { A = NODE3(IF, B, C, D); }
flow(A) ::= UNLESS statement(B) TERM statements(C) opt_term else(D) END. { A = NODE3(UNLESS, B, C, D); }
else(A) ::= ELSE TERM statements(B) opt_term. { A = B; }
else(A) ::= . { A = 0; }

flow(A) ::= WHILE statement(B) TERM statements(C) opt_term END. { A = NODE2(WHILE, B, C); }
flow(A) ::= UNTIL statement(B) TERM statements(C) opt_term END. { A = NODE2(UNTIL, B, C); }
/* one-liner conditions */
flow(A) ::= expr_out(C) IF statement(B). { A = NODE2(IF, B, NODES(C)); }
flow(A) ::= expr_out(C) UNLESS statement(B). { A = NODE2(UNLESS, B, NODES(C)); }


literal(A) ::= SYMBOL(B). { A = NODE(VALUE, B); }
literal(A) ::= INT(B). { A = NODE(VALUE, B); }
literal(A) ::= STRING(B). { A = NODE(STRING, B); }
literal(A) ::= TRUE. { A = NODE(BOOL, 1); }
literal(A) ::= FALSE. { A = NODE(BOOL, 0); }
literal(A) ::= NIL. { A = NODE(NIL, 0); }
literal(A) ::= SELF. { A = NODE(SELF, 0); }
literal(A) ::= RETURN. { A = NODE(RETURN, 0); }
literal(A) ::= CONST(B). { A = NODE(CONST, B); }
literal(A) ::= O_SBRA C_SBRA. { A = NODE(ARRAY, 0); }
literal(A) ::= O_SBRA args(B) C_SBRA. { A = NODE(ARRAY, B); }

assign_out(A) ::= CONST(B) ASSIGN statement(C). { A = NODE2(SETCONST, B, C); }
assign_out(A) ::= ID(B) ASSIGN statement(C). { A = NODE2(ASSIGN, B, C); }

assign(A) ::= CONST(B) ASSIGN expr(C). { A = NODE2(SETCONST, B, C); }
assign(A) ::= ID(B) ASSIGN expr(C). { A = NODE2(ASSIGN, B, C); }

expr(A) ::= expr(B) DOT name(C). { A = NODE2(SEND, B, C); }
expr(A) ::= expr(B) BINOP(C) msg(D). { A = NODE2(SEND, B, NODE2(MSG, C, NODES(D))); }
expr(A) ::= msg(B). { A = B; }
expr(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA. { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]"), C)); }
/*expr(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA ASSIGN expr(D). { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]="), PUSH(C, D))); }*/

/* outer expression, can pass args w/out parenthesis, eg.: puts "poop" */
expr_out(A) ::= expr(B) DOT name_out(C). { A = NODE2(SEND, B, C); }
expr_out(A) ::= expr(B) BINOP(C) msg_out(D). { A = NODE2(SEND, B, NODE2(MSG, C, NODES(D))); }
expr_out(A) ::= msg_out(B). { A = B; }
expr_out(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA. { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]"), C)); }
expr_out(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA ASSIGN expr(D). { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]="), PUSH(C, D))); }

name(A) ::= ID(B). { A = NODE(MSG, B); }
name(A) ::= ID(B) O_PAR C_PAR. { A = NODE(MSG, B); }
name(A) ::= ID(B) O_PAR args(C) C_PAR. { A = NODE2(MSG, B, C); }

name_out(A) ::= name(B). { A = B; }
name_out(A) ::= ID(B) args(C). { A = NODE2(MSG, B, C); }

msg_out(A) ::= name_out(B). { A = NODE2(SEND, TR_NIL, B); }
msg_out(A) ::= literal(B). { A = B; }

msg(A) ::= name(B). { A = NODE2(SEND, TR_NIL, B); }
msg(A) ::= literal(B). { A = B; }

args(A) ::= args(B) COMMA arg(C). { A = PUSH(B, C); }
args(A) ::= arg(B). { A = NODES(B); }

arg(A) ::= expr(B). { A = B; }
arg(A) ::= assign(B). { A = B; }

def(A) ::= DEF ID(B) TERM statements(D) opt_term END. { A = NODE3(DEF, B, 0, D); }
def(A) ::= DEF ID(B) O_PAR params(C) C_PAR TERM statements(D) opt_term END. { A = NODE3(DEF, B, C, D); }

params(A) ::= params(B) COMMA ID(C). { A = PUSH(B, NODE(PARAM, C)); }
params(A) ::= ID(B). { A = NODES(NODE(PARAM, B)); }

class(A) ::= CLASS CONST(B) TERM statements(C) opt_term END. { A = NODE2(CLASS, B, C); }

opt_term ::= TERM.
opt_term ::= .
