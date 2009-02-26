#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tr.h"
#include "internal.h"
#include "grammar.h"

#define TOKEN_V(id,v) TrParser(parser, TR_TOK_##id, v, compiler); last = TR_TOK_##id
#define TOKEN_U(id)   if (last != TR_TOK_##id) { TOKEN(id); }
#define TOKEN(id)     TOKEN_V(id, 0)

#define WITH_STR_TERM(P,L,B)  ({ \
  char *str = (char*)P; \
  char old = str[L]; \
  str[L] = '\0'; \
  OBJ obj = B; \
  str[L] = old; \
  obj; \
})
#define SYMBOL(P,L) WITH_STR_TERM(P,L, tr_intern(P))
#define STRING(P,L) WITH_STR_TERM(P,L, TrString_new(vm, P, L))
#define FIXNUM(P,L) WITH_STR_TERM(P,L, TrFixnum_new(vm, atoi(P)))

%%{
  machine tr;
  
  newline     = "\r"? "\n" %{ compiler->line++; };
  whitespace  = " " | "\f" | "\t" | "\v";
  
  term        = (newline | ";");
  char        = [a-zA-Z0-9_];
  var         = [a-z_] char*;
  id          = [a-zA-Z] char*;
  id_op       = id ("?" | "!" | "=");
  const       = [A-Z] char*;
  int         = "-"? [0-9]+;
  string      = '"' (any - '"')* '"' | "'" (any - "'")* "'";
  symbol      = ':' (id | id_op);
  cvar        = '@@' id;
  ivar        = '@' id;
  global      = '$' id;
  comment     = "#"+ (any - newline)*;
  
  main := |*
    whitespace;
    comment;
    
    # keywords
    "if"        => { TOKEN(IF); };
    "unless"    => { TOKEN(UNLESS); };
    "else"      => { TOKEN(ELSE); };
    "while"     => { TOKEN(WHILE); };
    "until"     => { TOKEN(UNTIL); };
    "end"       => { TOKEN(END); };
    "true"      => { TOKEN(TRUE); };
    "false"     => { TOKEN(FALSE); };
    "nil"       => { TOKEN(NIL); };
    "self"      => { TOKEN(SELF); };
    "return"    => { TOKEN(RETURN); };
    "def"       => { TOKEN(DEF); };
    "class"     => { TOKEN(CLASS); };
    "module"    => { TOKEN(MODULE); };
    "do"        => { TOKEN(DO); };
    "yield"     => { TOKEN(YIELD); };
    # HACK any better way to do this?
    ".class"    => { TOKEN(DOT); TOKEN_V(VAR, tr_intern("class")); };
    
    # ponctuation
    "=>"        => { TOKEN(HASHI); };
    ","         => { TOKEN(COMMA); };
    "("         => { TOKEN(O_PAR); };
    ")"         => { TOKEN(C_PAR); };
    "self["     => { TOKEN(SELF); TOKEN(O_SBRA_ID); };
    id "["      => { TOKEN_V(VAR, SYMBOL(ts, te-ts-1)); TOKEN(O_SBRA_ID); };
    "["         => { TOKEN(O_SBRA); };
    "]"         => { TOKEN(C_SBRA); };
    "{"         => { TOKEN(O_CBRA); };
    "}"         => { TOKEN(C_CBRA); };
    "."         => { TOKEN(DOT); };
    term        => { TOKEN_U(TERM); };
    
    # assign operators
    "="         => { TOKEN_V(ASSIGN, SYMBOL(ts, te-ts)); };

    # binary operators
    '=='        => { TOKEN_V(EQ, SYMBOL(ts, te-ts)); };
    '!='        => { TOKEN_V(NEQ, SYMBOL(ts, te-ts)); };
    '|'         => { TOKEN_V(PIPE, SYMBOL(ts, te-ts)); };
    '||'        => { TOKEN_V(OR, SYMBOL(ts, te-ts)); };
    '&'         => { TOKEN_V(AMP, SYMBOL(ts, te-ts)); };
    '&&'        => { TOKEN_V(AND, SYMBOL(ts, te-ts)); };
    '<'         => { TOKEN_V(LT, SYMBOL(ts, te-ts)); };
    '<='        => { TOKEN_V(LE, SYMBOL(ts, te-ts)); };
    '>'         => { TOKEN_V(GT, SYMBOL(ts, te-ts)); };
    '>='        => { TOKEN_V(GE, SYMBOL(ts, te-ts)); };
    '<<'        => { TOKEN_V(LSHIFT, SYMBOL(ts, te-ts)); };
    '>>'        => { TOKEN_V(RSHIFT, SYMBOL(ts, te-ts)); };
    '*'         => { TOKEN_V(MUL, SYMBOL(ts, te-ts)); };
    '**'        => { TOKEN_V(POW, SYMBOL(ts, te-ts)); };
    '/'         => { TOKEN_V(DIV, SYMBOL(ts, te-ts)); };
    '%'         => { TOKEN_V(MOD, SYMBOL(ts, te-ts)); };
    '+'         => { TOKEN_V(PLUS, SYMBOL(ts, te-ts)); };
    '-'         => { TOKEN_V(MINUS, SYMBOL(ts, te-ts)); };
    
    # values
    var         => { TOKEN_V(VAR, SYMBOL(ts, te-ts)); };
    id_op       => { TOKEN_V(ID_OP, SYMBOL(ts, te-ts)); };
    symbol      => { TOKEN_V(SYMBOL, SYMBOL(ts+1, te-ts-1)); };
    const       => { TOKEN_V(CONST, SYMBOL(ts, te-ts)); };
    ivar        => { TOKEN_V(IVAR, SYMBOL(ts, te-ts)); };
    cvar        => { TOKEN_V(CVAR, SYMBOL(ts, te-ts)); };
    global      => { TOKEN_V(GLOBAL, SYMBOL(ts, te-ts)); };
    string      => { TOKEN_V(STRING, STRING(ts+1, te-ts-2)); };
    int         => { TOKEN_V(INT, FIXNUM(ts, te-ts)); };
  *|;
  
  write data nofinal;
}%%

void tr_free(void *ptr) { return TR_FREE(ptr); }

TrBlock *TrBlock_compile(VM, char *code, char *fn, size_t lineno) {
  int cs, act;
  char *p, *pe, *ts, *te, *eof = 0;
  void *parser = TrParserAlloc(TR_MALLOC);
  int last = 0;
  FILE *tracef = 0;
  TrCompiler *compiler = TrCompiler_new(vm, fn);
  compiler->line += lineno;
  compiler->filename = TrString_new2(vm, fn);
  
  p = code;
  pe = p + strlen(code) + 1;
  
  if (vm->debug > 1) {
    tracef = fdopen(2, "w+");
    TrParserTrace(tracef, "[parse] ");
  }
  
  %% write init;
  %% write exec;
  
  TrParser(parser, 0, 0, compiler);
  TrParserFree(parser, tr_free);
  
  if (vm->debug > 1) fclose(tracef);
    
  TrCompiler_compile(compiler);
  
  return compiler->block;
}
