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
  VM = compiler->vm;
  printf("Syntax error:\n"
         "  %s unexpected at %s:%d\n", yyTokenName[yymajor], TR_STR_PTR(compiler->filename), compiler->line+1);
}

%left OR AND.
%right ASSIGN.
%left CMP EQ NEQ.
%left GT GTE LT LTE.
%left PIPE AMP.
%left LSHIFT RSHIFT.
%left PLUS MINUS.
%left TIMES DIV MOD.
%right POW.

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
literal(A) ::= CONST(B). { A = NODE(CONST, B); }
literal(A) ::= O_SBRA C_SBRA. { A = NODE(ARRAY, 0); }
literal(A) ::= O_SBRA args(B) C_SBRA. { A = NODE(ARRAY, B); }
literal(A) ::= O_CBRA C_CBRA. { A = NODE(HASH, 0); }
literal(A) ::= O_CBRA hash_items(B) C_CBRA. { A = NODE(HASH, B); }
literal(A) ::= O_PAR arg(B) RANGE arg(C) C_PAR. { A = NODE3(RANGE, B, C, 0); }
literal(A) ::= O_PAR arg(B) RANGE_EX arg(C) C_PAR. { A = NODE3(RANGE, B, C, 1); }

leave(A) ::= RETURN. { A = NODE(RETURN, 0); }
leave(A) ::= YIELD. { A = NODE(YIELD, 0); }
leave(A) ::= YIELD O_PAR args(B) C_PAR. { A = NODE(YIELD, B); }

leave_out(A) ::= leave(B). { A = B; }
leave_out(A) ::= YIELD args(B). { A = NODE(YIELD, B); }

assign_out(A) ::= CONST(B) ASSIGN statement(C). { A = NODE2(SETCONST, B, C); }
assign_out(A) ::= VAR(B) ASSIGN statement(C). { A = NODE2(ASSIGN, B, C); }
assign_out(A) ::= IVAR(B) ASSIGN statement(C). { A = NODE2(SETIVAR, B, C); }
assign_out(A) ::= CVAR(B) ASSIGN statement(C). { A = NODE2(SETCVAR, B, C); }
assign_out(A) ::= GLOBAL(B) ASSIGN statement(C). { A = NODE2(SETGLOBAL, B, C); }

assign(A) ::= CONST(B) ASSIGN expr(C). { A = NODE2(SETCONST, B, C); }
assign(A) ::= VAR(B) ASSIGN expr(C). { A = NODE2(ASSIGN, B, C); }
assign(A) ::= IVAR(B) ASSIGN expr(C). { A = NODE2(SETIVAR, B, C); }
assign(A) ::= CVAR(B) ASSIGN expr(C). { A = NODE2(SETCVAR, B, C); }
assign(A) ::= GLOBAL(B) ASSIGN expr(C). { A = NODE2(SETGLOBAL, B, C); }

expr(A) ::= expr(B) DOT name(C). { A = NODE2(SEND, B, C); }
expr(A) ::= expr(B) bin_op(C) msg(D). { A = NODE2(SEND, B, NODE2(MSG, C, NODES(D))); }
expr(A) ::= msg(B). { A = B; }
expr(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA. { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]"), C)); }

/* outer expression, can pass args w/out parenthesis, eg.: puts "poop" */
expr_out(A) ::= expr(B) DOT name_out(C). { A = NODE2(SEND, B, C); }
expr_out(A) ::= expr(B) DOT name_out(C) block(D). { A = NODE3(SEND, B, C, D); }
expr_out(A) ::= expr(B) DOT VAR(C) ASSIGN(D) arg(E). { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, SYMCAT(C,D), NODES(E))); }
expr_out(A) ::= expr(B) bin_op(C) msg_out(D). { A = NODE2(SEND, B, NODE2(MSG, C, NODES(D))); }
expr_out(A) ::= msg_out(B). { A = B; }
expr_out(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA. { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]"), C)); }
expr_out(A) ::= expr(B) O_SBRA_ID args(C) C_SBRA ASSIGN expr(D). { VM = compiler->vm; A = NODE2(SEND, B, NODE2(MSG, tr_intern("[]="), PUSH(C, D))); }

id(A) ::= VAR(B). { A = B; }
id(A) ::= ID_OP(B). { A = B; }

name(A) ::= id(B). { A = NODE(MSG, B); }
name(A) ::= id(B) O_PAR C_PAR. { A = NODE(MSG, B); }
name(A) ::= id(B) O_PAR args(C) C_PAR. { A = NODE2(MSG, B, C); }

name_out(A) ::= name(B). { A = B; }
name_out(A) ::= id(B) args(C). { A = NODE2(MSG, B, C); }

msg_out(A) ::= name_out(B). { A = NODE2(SEND, TR_NIL, B); }
msg_out(A) ::= name_out(B) block(C). { A = NODE3(SEND, TR_NIL, B, C); }
msg_out(A) ::= literal(B). { A = B; }
msg_out(A) ::= leave_out(B). { A = B; }
msg_out(A) ::= IVAR(B). { A = NODE(GETIVAR, B); }
msg_out(A) ::= CVAR(B). { A = NODE(GETCVAR, B); }
msg_out(A) ::= GLOBAL(B). { A = NODE(GETGLOBAL, B); }

msg(A) ::= name(B). { A = NODE2(SEND, TR_NIL, B); }
msg(A) ::= literal(B). { A = B; }
msg(A) ::= leave(B). { A = B; }
msg(A) ::= IVAR(B). { A = NODE(GETIVAR, B); }
msg(A) ::= CVAR(B). { A = NODE(GETCVAR, B); }
msg(A) ::= GLOBAL(B). { A = NODE(GETGLOBAL, B); }

args(A) ::= args(B) COMMA arg(C). { A = PUSH(B, C); }
args(A) ::= arg(B). { A = NODES(B); }

arg(A) ::= expr(B). { A = B; }
arg(A) ::= assign(B). { A = B; }

block(A) ::= DO TERM statements(B) opt_term END. { A = NODE(BLOCK, B); }
block(A) ::= DO PIPE params(C) PIPE TERM statements(B) opt_term END. { A = NODE2(BLOCK, B, C); }

hash_items(A) ::= hash_items(B) COMMA expr(C) HASHI expr(D). { A = PUSH(B, C); A = PUSH(B, D); }
hash_items(A) ::= expr(B) HASHI expr(C). { A = NODES_N(2, B, C); }

method_name(A) ::= id(B). { A = B; }
method_name(A) ::= bin_op(B). { A = B; }

def(A) ::= DEF method_name(B) TERM statements(D) opt_term END. { A = NODE3(DEF, B, 0, D); }
def(A) ::= DEF method_name(B) O_PAR params(C) C_PAR TERM statements(D) opt_term END. { A = NODE3(DEF, B, C, D); }

params(A) ::= params(B) COMMA id(C). { A = PUSH(B, NODE(PARAM, C)); }
params(A) ::= params(B) COMMA MUL id(C). { A = PUSH(B, NODE2(PARAM, C, 1)); }
params(A) ::= VAR(B). { A = NODES(NODE(PARAM, B)); }
params(A) ::= MUL VAR(B). { A = NODES(NODE2(PARAM, B, 1)); }

class(A) ::= CLASS CONST(B) subclass(C) TERM statements(D) opt_term END. { A = NODE3(CLASS, B, C, D); }
class(A) ::= MODULE CONST(B) TERM statements(C) opt_term END. { A = NODE3(MODULE, B, 0, C); }

subclass(A) ::= LT CONST(B). { A = B; }
subclass(A) ::= . { A = 0; }

opt_term ::= TERM.
opt_term ::= .

bin_op(A) ::= EQ(B). { A = B; }
bin_op(A) ::= NEQ(B). { A = B; }
bin_op(A) ::= CMP(B). { A = B; }
bin_op(A) ::= LT(B). { A = B; }
bin_op(A) ::= LE(B). { A = B; }
bin_op(A) ::= GT(B). { A = B; }
bin_op(A) ::= GE(B). { A = B; }
bin_op(A) ::= OR(B). { A = B; }
bin_op(A) ::= PIPE(B). { A = B; }
bin_op(A) ::= AND(B). { A = B; }
bin_op(A) ::= AMP(B). { A = B; }
bin_op(A) ::= LSHIFT(B). { A = B; }
bin_op(A) ::= RSHIFT(B). { A = B; }
bin_op(A) ::= POW(B). { A = B; }
bin_op(A) ::= MUL(B). { A = B; }
bin_op(A) ::= DIV(B). { A = B; }
bin_op(A) ::= MOD(B). { A = B; }
bin_op(A) ::= PLUS(B). { A = B; }
bin_op(A) ::= MINUS(B). { A = B; }
