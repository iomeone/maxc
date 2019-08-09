#ifndef MAXC_ENV_H
#define MAXC_ENV_H

#include "maxc.h"
#include "type.h"

class NodeVariable;
enum class BltinFnKind;

enum class VarAttr {
    Const = 0b0001,
    Uninit = 0b0010,
};

struct var_t {
    int vattr;
    Type *type;
};

struct arg_t {
    Type *type;
    std::string name;
};

class Varlist {
  public:
    void push(NodeVariable *v);
    void push(std::vector<NodeVariable *> &);
    std::vector<NodeVariable *> var_v;
    std::vector<NodeVariable *> &get();
    void show();
    void set_number();
    void reset();
};

// Function

struct func_t {
    BltinFnKind fnkind;
    Varlist args;
    Type *ftype;
    bool isbuiltin;

    func_t() {}
    func_t(BltinFnKind k, Type *f) : fnkind(k), ftype(f), isbuiltin(true) {}
    func_t(Type *f) : ftype(f), isbuiltin(false) {}
    func_t(Varlist a, Type *f) : args(a), ftype(f), isbuiltin(false) {}
};

struct env_t {
    Varlist vars;
    env_t *parent;
    bool isglb;

    env_t() {}
    env_t(bool i) : isglb(i) {}
};

class Scope {
  public:
    env_t *current = nullptr;
    env_t *make();
    env_t *escape();
    env_t *get();
    bool isglobal();
};

class FuncEnv {
  public:
    env_t *current = nullptr;
    env_t *make();
    env_t *escape();
    bool isglobal();
};

#endif