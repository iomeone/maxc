#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#include <iostream>
#include <utility>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <map>
#include <regex>

/*
 *  main
 */

class Maxc {
    public:
        int run(std::string src);

        void show_usage();
        std::string version = "0.0.1";
    private:
};

/*
 *  token
 */

enum class TOKEN_TYPE {
    END,
    NUM,
    SYMBOL,
    STRING,
    CHAR,
    IDENTIFER
};

struct location_t {
    int line;
    int col;
};

typedef struct {
    TOKEN_TYPE type;
    std::string value;
    int line, col;
} token_t;

class Token {
    public:
        std::vector<token_t> token_v;

        void push_num(std::string value, int line, int col);
        void push_symbol(std::string value, int line, int col);
        void push_ident(std::string value, int line, int col);
        void push_string(std::string value, int line, int col);
        void push_char(std::string value, int line, int col);
        void push_end(int, int);

        void show();

        token_t get();
        token_t get_step();
        token_t see(int p);
        bool is_value(std::string tk);
        bool is_type(TOKEN_TYPE ty);
        bool isctype();
        bool is_stmt();
        bool skip(std::string val);
        bool expect(std::string val);
        void step();
        bool step_to(std::string val);

        void save();
        void rewind();

    private:
        int save_point;
        bool issaved;
        int pos = 0;
};

/*
 *  lexer
 */

class Lexer {
    public:
        Token run(std::string src);
};

/*
 *  ctype
 */

enum class CTYPE {
    NONE,
    INT,
    UINT,
    INT64,
    UINT64,
    CHAR,
    STRING,
    LIST,
    PTR,
    DYNAMIC,
};

struct type_t {
    CTYPE type;
    int size;   //array size

    type_t() {}
    type_t(CTYPE ty): type(ty){}
    type_t(CTYPE ty, int size): type(ty), size(size) {}
};

class Type {
    private:
        type_t type;
    public:
        Type *ptr;

        Type() {}
        Type(CTYPE ty): type(ty) {}
        Type(Type *t): type(CTYPE::PTR), ptr(t) {}    //ptr
        Type(CTYPE ty, int size, Type *t): type(ty, size), ptr(t) {}

        std::string show();
        int get_size();
        type_t get();
};

typedef std::vector<Type *> Type_v;


/*
 *  AST, parser
 */

enum class NDTYPE {
    NUM = 100,
    CHAR,
    LIST,
    SYMBOL,
    IDENT,
    RETURN,
    FUNCDEF,
    FUNCCALL,
    FUNCPROTO,
    VARDECL,
    ASSIGNMENT,
    VARIABLE,
    BLOCK,
    STRING,
    BINARY,
    DOT,
    UNARY,
    IF,
    EXPRIF,
    FOR,
    WHILE,
    PRINT,
    PRINTLN,
    FORMAT,
    TYPEOF,
};

class Ast {
    public:
        Type *ctype;
        virtual NDTYPE get_nd_type() = 0;
};

typedef std::vector<Ast *> Ast_v;

//Variable, arguments

struct var_t {
    Type *type;
    std::string name;
};

struct arg_t {
    Type *type;
    std::string name;
};


/*
struct func_t {
    Type *ret_type;
    std::string name;
    std::vector<arg_t> args;
    Ast_v block;
    Varlist localvars;
    Env env;
};


class Funclist {
    public:
        std::vector<func_t> funcs;
        void push(func_t f);
        func_t *find(std::string);
};
*/

//AST
//nodes

class Node_number: public Ast {
    public:
        int number;
        virtual NDTYPE get_nd_type() { return NDTYPE::NUM; }

        Node_number(int _n): number(_n) {
            ctype = new Type(CTYPE::INT);
        }
};

class Node_char: public Ast {
    public:
        char ch;
        virtual NDTYPE get_nd_type() { return NDTYPE::CHAR; }

        Node_char(char _c): ch(_c) {
            ctype = new Type(CTYPE::CHAR);
        }
};

class Node_string: public Ast {
    public:
        std::string string;
        virtual NDTYPE get_nd_type() { return NDTYPE::STRING; }

        Node_string(std::string _s): string(_s){
            ctype = new Type(CTYPE::STRING);
        }
};

class Node_list: public Ast {
    public:
        Ast_v elem;
        size_t nsize;
        virtual NDTYPE get_nd_type() { return NDTYPE::LIST; }

        Node_list(Ast_v e, size_t s): elem(e), nsize(s) {
            ctype = new Type(CTYPE::LIST);
        }
};

class Node_binop: public Ast {
    public:
        std::string symbol;
        Ast *left;
        Ast *right;
        virtual NDTYPE get_nd_type() { return NDTYPE::BINARY; }

        Node_binop(std::string _s, Ast *_l, Ast *_r, Type *_t):
            symbol(_s), left(_l), right(_r) { ctype = _t; }
};

class Node_dotop: public Ast {
    public:
        Ast *left;
        Ast *right;
        virtual NDTYPE get_nd_type() { return NDTYPE::DOT; }

        Node_dotop(Ast *l, Ast *r): left(l), right(r) {}
};

class Node_unaop: public Ast {
    public:
        std::string op;
        Ast *expr;
        virtual NDTYPE get_nd_type() { return NDTYPE::UNARY; }

        Node_unaop(std::string _o, Ast *_e, Type *_t):
            op(_o), expr(_e){ ctype = _t; }
};

class Node_assignment: public Ast {
    public:
        Ast *dst;
        Ast *src;
        virtual NDTYPE get_nd_type() { return NDTYPE::ASSIGNMENT; }

        Node_assignment(Ast *_d, Ast *_s): dst(_d), src(_s) {}
};


class Node_variable: public Ast {
    public:
        var_t vinfo;
        bool isglobal = false;
        int id;
        Node_variable *vid;
        virtual NDTYPE get_nd_type() { return NDTYPE::VARIABLE; }

        Node_variable(var_t _v, bool _b): vinfo(_v), isglobal(_b){
            ctype = _v.type; vid = this;
        }
};

//Variable list

class Varlist {
    public:
        void push(Node_variable *v);
        std::vector<Node_variable *> var_v;
        std::vector<Node_variable *> get();
        Node_variable *find(std::string n);
        void show();
        void reset();
    private:
};

//Env

struct env_t {
    Varlist vars;
    env_t *parent;
    bool isglb;

    env_t(){}
    env_t(bool i): isglb(i){}
};

class Env {
    public:
        env_t *current = nullptr;
        env_t *make();
        env_t *escape();
        env_t *get();
        bool isglobal();
};

class Node_vardecl: public Ast {
    public:
        Varlist var;
        Ast_v init;
        virtual NDTYPE get_nd_type() { return NDTYPE::VARDECL; }

        Node_vardecl(Varlist _v, Ast_v _i): var(_v), init(_i){}
};

//Node func

class Node_func_def: public Ast {
    public:
        std::string name;
        Varlist args;
        Ast_v block;
        Varlist lvars;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCDEF; }

        Node_func_def(Type *_r, std::string _n, Varlist _a, Ast_v _b, Varlist _l):
            name(_n), args(_a), block(_b), lvars(_l) { ctype = _r; }
};

class Node_func_call: public Ast {
    public:
        std::string name;
        Ast_v arg_v;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCCALL; }

        Node_func_call(std::string _n, Ast_v _a):
            name(_n), arg_v(_a){ ctype = new Type(CTYPE::INT); }    //FIXME
};

class Node_func_proto: public Ast {
    public:
        Type *ret_type;
        std::string name;
        Type_v types;
        virtual NDTYPE get_nd_type() { return NDTYPE::FUNCPROTO; }

        Node_func_proto(Type *_r, std::string _n, Type_v _t):
            ret_type(_r), name(_n), types(_t){}
};


class Node_return: public Ast {
    public:
        Ast *cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::RETURN; }

        Node_return(Ast *_a): cont(_a){}
};

class Node_if: public Ast {
    public:
        Ast *cond;
        Ast *then_s, *else_s;
        virtual NDTYPE get_nd_type() { return NDTYPE::IF; }

        Node_if(Ast *_c, Ast *_t, Ast *_e):
            cond(_c), then_s(_t), else_s(_e){}
};

class Node_exprif: public Ast {
    public:
        Ast *cond;
        Ast *then_s, *else_s;
        virtual NDTYPE get_nd_type() { return NDTYPE::EXPRIF; }

        Node_exprif(Ast *_c, Ast *_t, Ast *_e):
            cond(_c), then_s(_t), else_s(_e){}
};

class Node_for: public Ast {
    public:
        Ast *init, *cond, *reinit;
        Ast *body;
        virtual NDTYPE get_nd_type() { return NDTYPE::FOR; }

        Node_for(Ast *_i, Ast *_c, Ast *_r, Ast *_b):
            init(_i), cond(_c), reinit(_r), body(_b){}
};

class Node_while: public Ast {
    public:
        Ast *cond;
        Ast *body;
        virtual NDTYPE get_nd_type() { return NDTYPE::WHILE; }

        Node_while(Ast *_c, Ast *_b): cond(_c), body(_b){}
};

class Node_block: public Ast {
    public:
        Ast_v cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::BLOCK; }

        Node_block(Ast_v _c): cont(_c){}
};

class Node_print: public Ast {
    public:
        Ast *cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::PRINT; }

        Node_print(Ast *c): cont(c) {
            ctype = new Type(CTYPE::NONE);
        }
};

class Node_println: public Ast {
    public:
        Ast *cont;
        virtual NDTYPE get_nd_type() { return NDTYPE::PRINTLN; }

        Node_println(Ast *c): cont(c) {
            ctype = new Type(CTYPE::NONE);
        }
};

class Node_format: public Ast {
    public:
        std::string cont;
        int narg;
        Ast_v args;
        virtual NDTYPE get_nd_type() { return NDTYPE::FORMAT; }

        Node_format(std::string c, int n, Ast_v a):
            cont(c), narg(n), args(a) {
                ctype = new Type(CTYPE::STRING);
        }
};

class Node_typeof: public Ast {
    public:
        Node_variable *var;
        virtual NDTYPE get_nd_type() { return NDTYPE::TYPEOF; }

        Node_typeof(Node_variable *v): var(v) {}
};

/*
 * Object
 */
struct value_t;

class Object {
    public:
};

class ListObject : public Object {
    public:
        std::vector<value_t> lselem;
        size_t get_size();
        value_t get_item(size_t);
        value_t set_item(std::vector<value_t> items);
};

class Parser {
    public:
        Ast_v run(Token);
        void show(Ast *ast);
        Env env;

    private:
        Token token;
        bool is_func_call();
        int skip_ptr();
        Type *checktype(Type *, Type *);

        Ast *var_decl();
        Type *eval_type();
        Ast *assignment();
        Ast *make_assign(Ast *dst, Ast *src);
        Ast *make_assigneq(std::string, Ast *, Ast *);
        Ast *make_return();
        Ast *make_if();
        Ast *make_for();
        Ast *make_while();
        Ast *make_block();
        Ast *make_print();
        Ast *make_println();
        Ast *make_format();
        Ast *make_typeof();
        Ast *func_def();
        Ast *func_call();
        Ast *expr();
        Ast *expr_first();
        Ast *expr_assign();
        Ast *expr_logic_or();
        Ast *expr_logic_and();
        Ast *expr_equality();
        Ast *expr_comp();
        Ast *expr_add();
        Ast *expr_mul();
        Ast *expr_unary();
        Ast *expr_unary_postfix();
        Ast *expr_primary();
        Ast *expr_if();
        Ast *expr_num(token_t token);
        Ast *expr_char(token_t token);
        Ast *expr_string(token_t token);
        Ast *expr_var(token_t token);
        Ast_v eval();
        Ast *statement();

        Varlist vls;
};


/*
 *  codegen
 */

enum class OPCODE {
    PUSH,
    POP,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LOGOR,
    LOGAND,
    EQ,
    NOTEQ,
    LT,
    LTE,
    GT,
    GTE,
    LABEL,
    JMP,
    JMP_EQ,
    JMP_NOTEQ,
    INC,
    DEC,
    PRINT,
    PRINTLN,
    FORMAT,
    TYPEOF,
    LOAD,
    STORE,
    RET,
    CALL,
    FNBEGIN,
    FNEND,
    END,
};

enum class VALUE {
    Number,
    Char,
    String,
    Array,
    Bool,
    Null,
    Object,
    NONE,   //defalut
};

struct value_t {
    VALUE type;
    CTYPE ctype;
    union {
        int num;
        uint32_t unum;
        int64_t inum64;
        uint64_t unum64;
        char ch;
    };
    std::string str;

    value_t() {}
    value_t(int n): type(VALUE::Number), ctype(CTYPE::INT), num(n) {}
    value_t(char c): type(VALUE::Char), ctype(CTYPE::CHAR), ch(c) {}
    value_t(std::string s): type(VALUE::String), ctype(CTYPE::STRING), str(s) {}
};

struct variable_t {
    Node_variable *var;
    value_t val;

    variable_t(Node_variable *_var): var(_var) {}
    variable_t(Node_variable *_vr, value_t _val): var(_vr), val(_val) {}
};

struct vmcode_t {
    OPCODE type;
    VALUE vtype = VALUE::NONE;
    union {
        int num = 0;
        char ch;
    };
    std::string str;
    variable_t *var = nullptr;
    unsigned int nfarg;

    int nline;

    vmcode_t() {}
    vmcode_t(OPCODE t, int l): type(t), nline(l) {}
    vmcode_t(OPCODE t, int v, int l): type(t), vtype(VALUE::Number), num(v), nline(l) {}
    vmcode_t(OPCODE t, char c, int l): type(t), vtype(VALUE::Char), ch(c), nline(l) {}
    vmcode_t(OPCODE t, std::string s, int l): type(t), vtype(VALUE::String), str(s), nline(l) {}
    vmcode_t(OPCODE t, Node_variable *vr, int l):
        type(t), var(new variable_t(vr)), nline(l) {}
    vmcode_t(OPCODE t, std::string s, unsigned int n, int l):
        type(t), str(s), nfarg(n), nline(l) {}  //format
};

class Program {
    public:
        void compile(Ast_v asts, Env e);
        void gen(Ast *ast);
        void show();
        std::vector<vmcode_t> vmcodes;
        std::map<std::string, int> lmap;
    private:
        void emit_head();
        void emit_num(Ast *ast);
        void emit_char(Ast *ast);
        void emit_string(Ast *ast);
        void emit_binop(Ast *ast);
        bool emit_log_andor(Node_binop *b);
        void emit_pointer(Node_binop *b);
        void emit_addr(Ast *ast);
        void emit_unaop(Ast *ast);
        void emit_if(Ast *ast);
        void emit_exprif(Ast *ast);
        void emit_for(Ast *ast);
        void emit_while(Ast *ast);
        void emit_return(Ast *ast);
        void emit_block(Ast *ast);
        void emit_print(Ast *ast);
        void emit_println(Ast *ast);
        void emit_format(Ast *ast);
        void emit_typeof(Ast *ast);
        void emit_assign(Ast *ast);
        void emit_store(Ast *ast);
        void emit_func_def(Ast *ast);
        void emit_func_call(Ast *ast);
        void emit_func_head(Node_func_def*);
        void emit_func_end();
        void emit_vardecl(Ast *ast);
        void emit_variable(Ast *ast);
        void emit_cmp(std::string ord, Node_binop *a);

        //VMcode push
        void vcpush(OPCODE t);
        void vcpush(OPCODE t, int v);
        void vcpush(OPCODE t, char c);
        void vcpush(OPCODE t, std::string s);
        void vcpush(OPCODE t, Node_variable *vr);
        void vcpush(OPCODE t, std::string s, unsigned int n);

        void opcode2str(OPCODE);
        std::string get_label();
        int get_lvar_size();
        int size;
        int get_var_pos(std::string name);
        int align(int n, int base);
        int nline = 0;
        std::string src;
        std::string x86_ord;
        std::string endlabel;
        bool isused_var = false;
        bool isexpr = false;

        int labelnum = 1;

        Env env;
};

/*
 *  VM
 */

struct vmenv_t {
    std::map<Node_variable *, value_t> vmap;
    vmenv_t *parent;

    vmenv_t() {}
};

class VMEnv {
    public:
        vmenv_t *cur;
        vmenv_t *make();
        vmenv_t *escape();
        std::map<Node_variable *, value_t> getvmap();
    private:
};

class VM {
    public:
        int run(std::vector<vmcode_t> &code, std::map<std::string, int> &lmap);
        void exec(std::vector<vmcode_t>);
    private:
        std::stack<value_t> s;
        std::stack<unsigned int> locs;
        std::map<Node_variable *, value_t> gvmap;
        std::map<std::string, int> labelmap;
        unsigned int pc;
        VMEnv env;
};

/*
 *  error
 */

void error(const char *msg, ...);
void error(int line, int col, const char *msg, ...);
void warning(int line, int col, const char *msg, ...);
void runtime_err(const char *msg, ...);
void debug(const char *msg, ...);
std::string skipln(int n);
