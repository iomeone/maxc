#ifndef MAXC_SEMA_H
#define MAXC_SEMA_H

#include "ast.h"
#include "maxc.h"
#include "type.h"
#include "method.h"
#include "env.h"

class SemaAnalyzer {
  public:
    Ast_v &run(Ast_v &);
    size_t ngvar;
  private:
    Ast_v ret_ast;

    std::stack<NodeFunction *, std::vector<NodeFunction *>> fn_saver;

    Scope scope;
    FuncEnv fnenv;

    void setup_bltin();

    Ast *visit(Ast *);
    Ast *visit_binary(Ast *);
    Ast *visit_unary(Ast *);
    Ast *visit_assign(Ast *);
    Ast *visit_vardecl(Ast *);
    Ast *visit_if(Ast *);
    Ast *visit_while(Ast *);
    Ast *visit_return(Ast *);
    Ast *visit_member(Ast *);
    Ast *visit_block(Ast *);
    Ast *visit_load(Ast *);
    Ast *visit_fncall(Ast *);
    Ast *visit_funcdef(Ast *);
    Ast *visit_bltinfn_call(NodeFnCall *);

    NodeVariable *do_variable_determining(std::string &);

    Type *checktype(Type *, Type *);
};

#endif
