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
  const       = [A-Z] [a-zA-Z0-9_]*;
  int         = [0-9]+;
  string      = '"' (any - '"')* '"' | "'" (any - "'")* "'";
  symbol      = ':' id;
  comment     = "#"+ (any - newline)* newline;
  dot         = '.';
  assign      = '=' | '+=' | '-=';
  unop        = '!';
  binop       = '==' | '!=' | '||' | '&&' | '|' | '&' | '<' | '<=' | '>' | '>=' | '<<' | '>>' | '**' | '*' | '/' | '%' | '+' | '-' | '@@' | '@' | '..';
  
  main := |*
    whitespace;
    comment;
    
    # keywords
    "if"        => { TOKEN(IF); };
    "unless"    => { TOKEN(UNLESS); };
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
    # HACK any better way to do this?
    ".class"    => { TOKEN(DOT); TOKEN_V(ID, tr_intern("class")); };
    
    # ponctuation
    ","         => { TOKEN(COMMA); };
    "("         => { TOKEN(O_PAR); };
    ")"         => { TOKEN(C_PAR); };
    
    id          => { TOKEN_V(ID, tr_intern(BUFFER(ts, te-ts))); };
    const       => { TOKEN_V(CONST, tr_intern(BUFFER(ts, te-ts))); };
    symbol      => { TOKEN_V(SYMBOL, tr_intern(BUFFER(ts+1, te-ts-1))); };
    binop       => { TOKEN_V(BINOP, tr_intern(BUFFER(ts, te-ts))); };
    assign      => { TOKEN_V(ASSIGN, tr_intern(BUFFER(ts, te-ts))); };
    string      => { TOKEN_V(STRING, TrString_new(vm, BUFFER(ts+1, te-ts-2), te-ts-2)); };
    int         => { TOKEN_V(INT, TrFixnum_new(vm, atoi(BUFFER(ts, te-ts)))); };
    term        => { TOKEN_U(TERM); };
    dot         => { TOKEN(DOT); };
  *|;
  
  write data nofinal;
}%%

inline void tr_free(void *ptr) { return TR_FREE(ptr); }

TrBlock *TrBlock_compile(VM, char *code, char *fn, int trace) {
  int cs, act;
  char *p, *pe, *ts, *te, *eof = 0;
  void *parser = TrParserAlloc(TR_MALLOC);
  int last = 0;
  char *buf = 0;
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
  
  if (trace)
    fclose(tracef);
    
  TrCompiler_compile(compiler);
  TrBlock *b = compiler->block;
  TrCompiler_destroy(compiler);
  
  return b;
}
