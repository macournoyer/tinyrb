#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tr.h"
#include "grammar.h"

#define TOKEN_V(id,v) TrParser(parser, TR_TOK_##id, v, compiler); last = TR_TOK_##id
#define TOKEN_U(id)   if (last != TR_TOK_##id) { TOKEN(id); }
#define TOKEN(id)     TOKEN_V(id, 0)

#define BUFFER(str,l)  ({ \
  if (buf) free(buf); \
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
  id          = [a-zA-Z_]+;
  int         = [0-9]+;
  string      = '"' (any - '"')* '"' | "'" (any - "'")* "'";
  symbol      = ':' id;
  comment     = "#"+ (any - newline)* newline;
  dot         = '.';
  assign      = '=' | '+=' | '-=';
  unary       = '!';
  binary      = '==' | '!=' | '||' | '&&' | '|' | '&' | '<' | '<=' | '>' | '>=' | '<<' | '>>' | '**' | '*' | '/' | '%' | '+' | '-' | '@@' | '@' | '..';
  
  main := |*
    whitespace;
    comment;
    
    id          => { TOKEN_V(ID, tr_intern(BUFFER(ts, te-ts))); };
    symbol      => { TOKEN_V(SYMBOL, tr_intern(BUFFER(ts+1, te-ts-1))); };
    #binary      => { TOKEN_V(OP, tr_intern(BUFFER(ts, te-ts))); };
    assign      => { TOKEN_V(ASSIGN, tr_intern(BUFFER(ts, te-ts))); };
    string      => { TOKEN_V(STRING, (OBJ)BUFFER(ts+1, te-ts-2)); };
    #int         => { TOKEN_V(INT, MinMessage(lobby, MIN_STR(BUFFER(ts, te-ts)), 0, INT2FIX(atoi(BUFFER(ts, te-ts))))); };
    term        => { TOKEN_U(TERM); };
    dot         => { TOKEN(DOT); };
    
    # ponctuation
    # ","         => { TOKEN(COMMA); };
    "("         => { TOKEN(O_PAR); };
    ")"         => { TOKEN(C_PAR); };
    # "{"         => { TOKEN(O_BRA); };
    # "}"         => { TOKEN(C_BRA); };
    # "["         => { TOKEN(O_SQ_BRA); };
    # "]"         => { TOKEN(C_SQ_BRA); };
  *|;
  
  write data nofinal;
}%%

void tr_compile(VM, TrCompiler *compiler, char *code, int trace) {
  int cs, act;
  char *p, *pe, *ts, *te, *eof = 0;
  void *parser = TrParserAlloc(malloc);
  int last = 0;
  char *buf = 0;
  FILE *tracef = 0;
  
  p = code;
  pe = p + strlen(code) + 1;
  
  if (trace) {
    tracef = fdopen(2, "w+");
    TrParserTrace(tracef, "[parse] ");
  }
  
  %% write init;
  %% write exec;
  
  TrParser(parser, 0, 0, compiler);
  TrParserFree(parser, free);
  if (buf) free(buf);
  
  if (trace)
    fclose(tracef);
    
  TrCompiler_finish(compiler);
}
