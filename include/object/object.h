#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include "function.h"
#include "object/objimpl.h"
#include "builtins.h"

#define OBJECT_HEAD MxcObject base
#define ITERABLE_OBJECT_HEAD MxcIterable base
#define ITERABLE(ob) ((MxcIterable *)(ob))

typedef struct StringObject StringObject;
typedef struct MxcObject MxcObject;
typedef struct MxcIterable MxcIterable;

typedef StringObject *(*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);

#define OBJIMPL(ob) (((MxcObject *)ob)->impl)

struct MxcObject {
    MxcObjImpl *impl;
    int refcount;
    /* gc: TODO */
    unsigned int marked: 1;
};

struct MxcIterable {
    OBJECT_HEAD;
    MxcObject *next;
    size_t length;
    int index;
};

typedef struct IntObject {
    OBJECT_HEAD;
    int64_t inum;
} IntObject;

typedef struct FloatObject {
    OBJECT_HEAD;
    double fnum;
} FloatObject;

typedef struct BoolObject {
    OBJECT_HEAD;
    int64_t boolean;
} BoolObject;

typedef struct CharObject {
    OBJECT_HEAD;
    char ch;
} CharObject;

typedef struct ListObject {
    ITERABLE_OBJECT_HEAD;
    MxcObject **elem;
} ListObject;

struct StringObject {
    ITERABLE_OBJECT_HEAD;
    char *str;
    bool isdyn;
};

typedef struct ErrorObject {
    OBJECT_HEAD;
    const char *errmsg;
} ErrorObject;

typedef struct TupleObject {
    OBJECT_HEAD;
} TupleObject; // TODO

typedef struct FunctionObject {
    OBJECT_HEAD;
    userfunction *func;
} FunctionObject;

typedef struct BltinFuncObject {
    OBJECT_HEAD;
    bltinfn_ty func;
} BltinFuncObject;

typedef struct StructObject {
    OBJECT_HEAD;
    MxcObject **field;
} StructObject;

typedef struct NullObject {
    OBJECT_HEAD;
} NullObject;

IntObject *new_intobject(int64_t);
IntObject *int_add(IntObject *, IntObject *);
IntObject *int_sub(IntObject *, IntObject *);
IntObject *int_mul(IntObject *, IntObject *);
IntObject *int_div(IntObject *, IntObject *);
IntObject *int_mod(IntObject *, IntObject *);
BoolObject *int_eq(IntObject *, IntObject *);
BoolObject *int_noteq(IntObject *, IntObject *);
BoolObject *int_lt(IntObject *, IntObject *);
BoolObject *int_lte(IntObject *, IntObject *);
BoolObject *int_gt(IntObject *, IntObject *);
BoolObject *int_gte(IntObject *, IntObject *);
BoolObject *float_eq(FloatObject *, FloatObject *);
BoolObject *float_neq(FloatObject *, FloatObject *);
BoolObject *float_lt(FloatObject *, FloatObject *);
BoolObject *float_gt(FloatObject *, FloatObject *);
IntObject *int_inc(IntObject *);
IntObject *int_dec(IntObject *);

BoolObject *bool_logor(BoolObject *, BoolObject *);
BoolObject *bool_logand(BoolObject *, BoolObject *);
BoolObject *bool_not(BoolObject *);

FloatObject *new_floatobject(double);
FloatObject *float_div(FloatObject *, FloatObject *);

MxcObject *list_get(MxcIterable *, size_t);
MxcObject *list_set(MxcIterable *, size_t, MxcObject *);
MxcObject *str_index(MxcIterable *, size_t);
MxcObject *str_index_set(MxcIterable *, size_t, MxcObject *);

CharObject *new_charobject(char);
CharObject *new_charobject_ref(char *c);
StringObject *new_stringobject(char *, bool);
StringObject *str_concat(StringObject *, StringObject *);
FunctionObject *new_functionobject(userfunction *);
BltinFuncObject *new_bltinfnobject(bltinfn_ty);
MxcObject *iterable_next(MxcIterable *);
ListObject *new_listobject(size_t);
ListObject *new_listobject_size(IntObject *, MxcObject *);
StructObject *new_structobject(int);
ErrorObject *new_errorobject(const char *);

extern NullObject MxcNull;
extern BoolObject MxcTrue;
extern BoolObject MxcFalse;

#define Mxc_NULL  ((MxcObject *)&MxcNull)
#define Mxc_TRUE  ((MxcObject *)&MxcTrue)
#define Mxc_FALSE ((MxcObject *)&MxcFalse)

#define Mxc_RetNull() return INCREF(&MxcNull), Mxc_NULL
#define Mxc_RetTrue() return INCREF(&MxcTrue), Mxc_TRUE
#define Mxc_RetFalse() return INCREF(&MxcFalse), Mxc_FALSE

#define MxcBool_RetTrue() return INCREF(&MxcTrue), &MxcTrue
#define MxcBool_RetFalse() return INCREF(&MxcFalse), &MxcFalse

#define IntAdd(l, r) (new_intobject(l->inum + r->inum))
#define IntSub(l, r) (new_intobject(l->inum - r->inum))
#define IntMul(l, r) (new_intobject(l->inum * r->inum))
#define IntDiv(l, r) (new_intobject(l->inum / r->inum))
#define IntXor(l, r) (new_intobject(l->inum ^ r->inum))
#define FloatAdd(l, r) (new_floatobject(l->fnum + r->fnum))
#define FloatSub(l, r) (new_floatobject(l->fnum - r->fnum))
#define FloatMul(l, r) (new_floatobject(l->fnum * r->fnum))
#define FloatDiv(l, r) (new_floatobject(l->fnum / r->fnum))

/* reference counter */
#define INCREF(ob) (++((MxcObject *)(ob))->refcount)

#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--((MxcObject *)(ob))->refcount == 0) {                             \
            OBJIMPL((MxcObject *)ob)->dealloc((MxcObject *)ob);                \
        }                                                                      \
    } while(0)

#endif
