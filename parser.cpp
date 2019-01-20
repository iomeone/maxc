#include"maxc.h"

Ast_v Parser::run(Token _token) {
    token = _token;
    //Ast *ast = statement();
    Ast_v program = eval();
    for(Ast *ast: program) {
        show(ast);
        puts("");
    }

    return program;
}

Ast *Parser::statement() {
    if(token.is_value(";")) {
        token.step();
        return statement();
    }
    else if(token.is_type()) {
        if(is_func_def())
            return func_def();
        else
            return var_decl();
    }
    else if(token.is_type(TOKEN_TYPE_IDENTIFER)) {
        if(token.see(1).value == "=")
            return assignment();
        else
            return expr_add();
    }
    else
        return expr_add();
}

Ast_v Parser::eval() {
    Ast_v program;

    while(!token.is_type(TOKEN_TYPE_END) && !token.is_value("}")) {
        program.push_back(statement());

        token.skip(";");
    }
    return program;
}

Ast *Parser::func_def() {
    var_type ty = eval_type();
    std::string name = token.get().value;
    token.step();

    if(token.skip("(")) {
        std::vector<arg_t> args;

        while(!token.skip(")")) {
            var_type arg_ty = eval_type();
            std::string arg_name = token.get().value;
            token.step();
            args.push_back((arg_t){arg_ty, arg_name});
        }
        token.skip("{");
        Ast_v b;
        b = eval();

        return new Node_func_def(ty, name, args, b);
    }
}

Ast *Parser::var_decl() {
    std::vector<var_t> decls;
    var_type ty = eval_type();

    while(!token.skip(";")) {
        std::string name = token.get().value;
        decls.push_back((var_t){ty, name});

        token.step();
        token.skip(",");
        /*
        if(token.is_value("="))
            var_init();
        */
    }

    return new Node_var_decl(decls);
}

var_type Parser::eval_type() {
    if(token.is_value("var")) {
        token.step();
        return TYPE_INT;
    }
    else
        error("eval_type ?????");
}

Ast *Parser::assignment() {
    if(!token.is_type(TOKEN_TYPE_IDENTIFER))
        error("left is not identifer");
    Ast *dst = expr_add();
    Ast *src;
    //token.step();
    if(token.skip("="))
        src = expr_add();
    else
        error("????? in assignment");

    return new Node_assignment(dst, src);
}

Ast *Parser::expr_num(token_t token) {
    if(token.type != TOKEN_TYPE_NUM) {
        error("not a number");
    }
    return new Node_number(token.value);
}

Ast *Parser::expr_var(token_t token) {
    return new Node_variable(token.value);
}

Ast *Parser::expr_add() {
    Ast *left = expr_mul();

    while(1) {
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("+")) {
            token.step();
            left = new Node_binop("+", left, expr_mul());
        }
        else if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("-")) {
            token.step();
            left = new Node_binop("-", left, expr_mul());
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_mul() {
    Ast *left = expr_primary();

    while(1) {
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("*")) {
            token.step();
            left = new Node_binop("*", left, expr_primary());
        }
        else if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("/")) {
            token.step();
            left = new Node_binop("/", left, expr_primary());
        }
        else if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("%")) {
            token.step();
            left = new Node_binop("%", left, expr_primary());
        }
        else {
            return left;
        }
    }
}

Ast *Parser::expr_primary() {
    while(1) {
        if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value("(")) {
            token.step();
            Ast *left = expr_add();

            if(token.is_type(TOKEN_TYPE_SYMBOL) && token.is_value(")")) {
                token.step();
                return left;
            }
            else {
                error(") <- not found");
            }
        }
        if(token.is_type(TOKEN_TYPE_IDENTIFER))
            return expr_var(token.get_step());
        if(token.is_type(TOKEN_TYPE_NUM))
            return expr_num(token.get_step());

        error("in expr_primary: ????");
    }
}

void Parser::show(Ast *ast) {
    if(ast != nullptr) {
        switch(ast->get_nd_type()) {
            case ND_TYPE_NUM: {
                Node_number *n = (Node_number *)ast;
                std::cout << n->number << " ";
                break;
            }
            case ND_TYPE_SYMBOL: {
                Node_binop *b = (Node_binop *)ast;
                printf("(");
                std::cout << b->symbol << " ";
                show(b->left);
                show(b->right);
                printf(")");
                break;
            }
            case ND_TYPE_VARDECL: {
                Node_var_decl *v = (Node_var_decl *)ast;
                printf("var_decl: ");
                for(auto decl: v->decl_v)
                    std::cout << "(" << decl.type << ", " << decl.name << ")";
                break;
            }
            case ND_TYPE_ASSIGNMENT: {
                Node_assignment *a = (Node_assignment *)ast;
                printf("(= (");
                show(a->dst);
                printf(") (");
                show(a->src);
                printf("))");
                break;
            }
            case ND_TYPE_FUNCDEF: {
                Node_func_def *f = (Node_func_def *)ast;
                printf("func-def: (");
                std::cout << f->name << "(";
                for(auto a: f->arg_v)
                    std::cout << "(" << a.type << "," << a.name << ")";
                std::cout << ") -> " << f->ret_type << "(" << std::endl;
                for(Ast *b: f->block) {
                    show(b);
                    puts("");
                }
                printf(")");
                break;
            }
            case ND_TYPE_VARIABLE: {
                Node_variable *v = (Node_variable *)ast;
                printf("(var: ");
                std::cout << v->name << ")";
                break;
            }
            default: {
                error("??????");
            }
        }
    }
}

bool Parser::is_func_def() {
    int save = token.pos;
    token.step();
    token.step();

    if(token.skip("(")) {
        while(!token.is_value(")"))
            token.step();
        token.skip(")");
        if(token.skip("{")) {
            token.pos = save;

            return true;
        }
    }

    token.pos = save;

    return false;
}
