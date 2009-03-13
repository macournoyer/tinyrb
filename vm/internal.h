#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#define TR_ALLOC(T)          (T *)TR_MALLOC(sizeof(T))
#define TR_ALLOC_N(T,N)      (T *)TR_MALLOC(sizeof(T)*(N))

#define TR_MEMZERO(X,T)      memset((X), 0, sizeof(T))
#define TR_MEMZERO_N(X,T,N)  memset((X), 0, sizeof(T)*(N))
#define TR_MEMCPY(X,Y,T)     memcpy((X), (Y), sizeof(T))
#define TR_MEMCPY_N(X,Y,T,N) memcpy((X), (Y), sizeof(T)*(N))

/* ast building macros */
#define NODE(T,A)            TrNode_new(compiler->vm, AST_##T, (A), 0, 0, compiler->line)
#define NODE2(T,A,B)         TrNode_new(compiler->vm, AST_##T, (A), (B), 0, compiler->line)
#define NODE3(T,A,B,C)       TrNode_new(compiler->vm, AST_##T, (A), (B), (C), compiler->line)
#define NODES(I)             TrArray_new2(compiler->vm, 1, (I))
#define NODES_N(N,I...)      TrArray_new2(compiler->vm, (N), ##I)
#define PUSH_NODE(A,N)       TR_ARRAY_PUSH((A),(N))
#define SYMCAT(A,B)          tr_intern(strcat(TR_STR_PTR(A), TR_STR_PTR(B)))

/* This provides the compiler about branch hints, so it
   keeps the normal case fast. Stolen from Rubinius. */
#ifdef __GNUC__
#define likely(x)       __builtin_expect((long int)(x),1)
#define unlikely(x)     __builtin_expect((long int)(x),0)
#else
#define likely(x) x
#define unlikely(x) x
#endif

typedef enum {
  AST_ROOT,
  AST_BLOCK,
  AST_VALUE,
  AST_STRING,
  AST_ASSIGN,
  AST_ARG,
  AST_SEND,
  AST_MSG,
  AST_IF,
  AST_UNLESS,
  AST_AND,
  AST_OR,
  AST_WHILE,
  AST_UNTIL,
  AST_BOOL,
  AST_NIL,
  AST_SELF,
  AST_RETURN,
  AST_YIELD,
  AST_DEF,
  AST_METHOD,
  AST_PARAM,
  AST_CLASS,
  AST_MODULE,
  AST_CONST,
  AST_SETCONST,
  AST_ARRAY,
  AST_HASH,
  AST_RANGE,
  AST_GETIVAR,
  AST_SETIVAR,
  AST_GETCVAR,
  AST_SETCVAR,
  AST_GETGLOBAL,
  AST_SETGLOBAL
} TrNodeType;

typedef struct {
  TR_OBJECT_HEADER;
  TrNodeType ntype;
  OBJ args[3];
  size_t line;
} TrNode;

typedef struct {
  int line;
  OBJ filename;
  TrVM *vm;
  TrBlock *block;
  size_t reg;
  OBJ node;
} TrCompiler;

/* node */
OBJ TrNode_new(VM, TrNodeType type, OBJ a, OBJ b, OBJ c, size_t line);

/* compiler */
TrCompiler *TrCompiler_new(VM, const char *fn);
void TrCompiler_compile(TrCompiler *c);

/* parser */
void *TrParserAlloc(void *(*)(size_t));
void TrParser(void *, int, OBJ, TrCompiler *);
void TrParserFree(void *, void (*)(void*));
void TrParserTrace(FILE *stream, char *zPrefix);

#endif /* _INTERNAL_H_ */
