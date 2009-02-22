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

#define BUFFER(str,l)  ({ \
  if (buf) TR_FREE(buf); \
  buf = TR_ALLOC_N(char, (l)); \
  TR_MEMCPY_N(buf, (str), char, (l)); \
  buf[(l)] = '\0'; \
  buf; \
})

%%{
  machine tr;
  
  newline     = "\r"? "\n" %{ compiler->curline++; };
  whitespace  = " " | "\f" | "\t" | "\v";

  term        = (newline | ";");
  id          = [a-z_] [a-zA-Z0-9_]*;
  self        = "self";
  method      = id | self;
  const       = [A-Z] [a-zA-Z0-9_]*;
  int         = [0-9]+;
  string      = '"' (any - '"')* '"' | "'" (any - "'")* "'";
  symbol      = ':' id;
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
    ".class"    => { TOKEN(DOT); TOKEN_V(ID, tr_intern("class")); };
    
    # ponctuation
    "=>"        => { TOKEN(HASHI); };
    ","         => { TOKEN(COMMA); };
    "("         => { TOKEN(O_PAR); };
    ")"         => { TOKEN(C_PAR); };
    "self["     => { TOKEN(SELF); TOKEN(O_SBRA_ID); };
    id "["      => { TOKEN_V(ID, tr_intern(BUFFER(ts, te-ts-1))); TOKEN(O_SBRA_ID); };
    "["         => { TOKEN(O_SBRA); };
    "]"         => { TOKEN(C_SBRA); };
    "{"         => { TOKEN(O_CBRA); };
    "}"         => { TOKEN(C_CBRA); };
    "."         => { TOKEN(DOT); };
    term        => { TOKEN_U(TERM); };
    
    # assign operators
    "="         => { TOKEN_V(ASSIGN, tr_intern(BUFFER(ts, te-ts))); };

    # binary operators
    '=='        => { TOKEN_V(EQ, tr_intern(BUFFER(ts, te-ts))); };
    '!='        => { TOKEN_V(NEQ, tr_intern(BUFFER(ts, te-ts))); };
    '|'         => { TOKEN_V(PIPE, tr_intern(BUFFER(ts, te-ts))); };
    '||'        => { TOKEN_V(OR, tr_intern(BUFFER(ts, te-ts))); };
    '&'         => { TOKEN_V(AMP, tr_intern(BUFFER(ts, te-ts))); };
    '&&'        => { TOKEN_V(AND, tr_intern(BUFFER(ts, te-ts))); };
    '<'         => { TOKEN_V(LT, tr_intern(BUFFER(ts, te-ts))); };
    '<='        => { TOKEN_V(LE, tr_intern(BUFFER(ts, te-ts))); };
    '>'         => { TOKEN_V(GT, tr_intern(BUFFER(ts, te-ts))); };
    '>='        => { TOKEN_V(GE, tr_intern(BUFFER(ts, te-ts))); };
    '<<'        => { TOKEN_V(LSHIFT, tr_intern(BUFFER(ts, te-ts))); };
    '>>'        => { TOKEN_V(RSHIFT, tr_intern(BUFFER(ts, te-ts))); };
    '*'         => { TOKEN_V(MUL, tr_intern(BUFFER(ts, te-ts))); };
    '**'        => { TOKEN_V(POW, tr_intern(BUFFER(ts, te-ts))); };
    '/'         => { TOKEN_V(DIV, tr_intern(BUFFER(ts, te-ts))); };
    '%'         => { TOKEN_V(MOD, tr_intern(BUFFER(ts, te-ts))); };
    '+'         => { TOKEN_V(PLUS, tr_intern(BUFFER(ts, te-ts))); };
    '-'         => { TOKEN_V(MINUS, tr_intern(BUFFER(ts, te-ts))); };
    
    # values
    id          => { TOKEN_V(ID, tr_intern(BUFFER(ts, te-ts))); };
    const       => { TOKEN_V(CONST, tr_intern(BUFFER(ts, te-ts))); };
    symbol      => { TOKEN_V(SYMBOL, tr_intern(BUFFER(ts+1, te-ts-1))); };
    ivar        => { TOKEN_V(IVAR, tr_intern(BUFFER(ts, te-ts))); };
    cvar        => { TOKEN_V(CVAR, tr_intern(BUFFER(ts, te-ts))); };
    global      => { TOKEN_V(GLOBAL, tr_intern(BUFFER(ts, te-ts))); };
    string      => { TOKEN_V(STRING, TrString_new(vm, BUFFER(ts+1, te-ts-2), te-ts-2)); };
    int         => { TOKEN_V(INT, TrFixnum_new(vm, atoi(BUFFER(ts, te-ts)))); };
  *|;
  
  write data nofinal;
}%%

void tr_free(void *ptr) { return TR_FREE(ptr); }

TrBlock *TrBlock_compile(VM, char *code, char *fn, int trace) {
  int cs, act;
  char *p, *pe, *ts, *te, *eof = 0, *buf = 0;
  void *parser = TrParserAlloc(TR_MALLOC);
  int last = 0;
  FILE *tracef = 0;
  TrCompiler *compiler = TrCompiler_new(vm, fn);
  
  p = code;
  pe = p + strlen(code) + 1;
  
  if (trace) {
    tracef = fdopen(2, "w+");
    TrParserTrace(tracef, "[parse] ");
  }
  
  %% write init;
  %% write exec;
  
  TrParser(parser, 0, 0, compiler);
  TrParserFree(parser, tr_free);
  
  if (buf) TR_FREE(buf);
  if (trace) fclose(tracef);
    
  TrCompiler_compile(compiler);
  return compiler->block;
}
