#ifndef MAXC_BLTINFN_H
#define MAXC_BLTINFN_H

#include "env.h"
#include "maxc.h"
#include "bytecode.h"

class MxcObject;

struct userfunction {
    size_t codelength;
    size_t nlvars;
    uint8_t *code;
};

void new_userfunction(userfunction &, bytecode, Varlist);

typedef MxcObject *(*bltinfn_ty)(size_t);

MxcObject *print(size_t);
MxcObject *print_int(size_t);
MxcObject *print_float(size_t);
MxcObject *print_bool(size_t);
MxcObject *print_char(size_t);
MxcObject *print_string(size_t);
MxcObject *print_list(size_t);
MxcObject *println(size_t);
MxcObject *println_int(size_t);
MxcObject *println_float(size_t);
MxcObject *println_bool(size_t);
MxcObject *println_char(size_t);
MxcObject *println_string(size_t);
MxcObject *println_list(size_t);

enum class BltinFnKind {
    Print,
    PrintInt,
    PrintFloat,
    PrintBool,
    PrintChar,
    PrintString,
    PrintList,
    Println,
    PrintlnInt,
    PrintlnFloat,
    PrintlnBool,
    PrintlnChar,
    PrintlnString,
    PrintlnList,
    StringSize,
};

#endif
