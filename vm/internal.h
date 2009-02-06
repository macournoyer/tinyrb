#define NODE(T,A)      TrNode_new(compiler->vm, AST_##T, (A), 0, 0)
#define NODE2(T,A,B)   TrNode_new(compiler->vm, AST_##T, (A), (B), 0)
#define NODE3(T,A,B,C) TrNode_new(compiler->vm, AST_##T, (A), (B), (C))
#define NODES(N)       TrArray_new2(compiler->vm, 1, (N))
#define PUSH(A,N)      (({ TR_ARRAY_PUSH((A), (N)); }), A)

typedef enum {
  AST_ROOT,
  AST_CONST,
  AST_STRING,
  AST_ASSIGN,
  AST_SEND,
  AST_MSG,
} TrNodeType;

typedef struct {
  TR_OBJECT_HEADER;
  TrNodeType ntype;
  OBJ args[3];
} TrNode;

OBJ TrNode_new(VM, TrNodeType type, OBJ a, OBJ b, OBJ c);
