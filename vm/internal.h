#define NODE(T,A)      TrNode_new(compiler->vm, AST_##T, (A), 0, 0)
#define NODE2(T,A,B)   TrNode_new(compiler->vm, AST_##T, (A), (B), 0)
#define NODE3(T,A,B,C) TrNode_new(compiler->vm, AST_##T, (A), (B), (C))
#define NODES(N)       TrArray_new2(compiler->vm, 1, (N))
#define PUSH(A,N)      (({ TR_ARRAY_PUSH((A), (N)); }), A)

typedef enum {
  AST_ROOT,
  AST_BLOCK,
  AST_CONST,
  AST_STRING,
  AST_ASSIGN,
  AST_SEND,
  AST_MSG,
  AST_IF,
  AST_UNLESS,
  AST_WHILE,
  AST_UNTIL,
  AST_BOOL,
  AST_NIL,
  AST_SELF,
  AST_RETURN,
} TrNodeType;

typedef struct {
  TR_OBJECT_HEADER;
  TrNodeType ntype;
  OBJ args[3];
} TrNode;

typedef struct {
  int curline;
  TrVM *vm;
  TrBlock *block;
  size_t reg;
  OBJ node;
} TrCompiler;

/* node */
OBJ TrNode_new(VM, TrNodeType type, OBJ a, OBJ b, OBJ c);

/* compiler */
TrCompiler *TrCompiler_new(VM, const char *fn);
void TrCompiler_compile(TrCompiler *c);
void TrCompiler_destroy(TrCompiler *c);

/* parser */
void *TrParserAlloc(void *(*)(size_t));
void TrParser(void *, int, OBJ, TrCompiler *);
void TrParserFree(void *, void (*)(void*));
void TrParserTrace(FILE *stream, char *zPrefix);
