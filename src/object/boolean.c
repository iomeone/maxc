/* implementation of boolean object */

#include "object/boolobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcObject *bool_copy(MxcObject *b) {
    INCREF(b);
    return b;
}

BoolObject *bool_logor(BoolObject *l, BoolObject *r) {
    if(l->boolean || r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *bool_logand(BoolObject *l, BoolObject *r) {
    if(l->boolean && r->boolean)
        MxcBool_RetTrue();
    else
        MxcBool_RetFalse();
}

BoolObject *bool_not(BoolObject *u) {
    if(u->boolean)
        MxcBool_RetFalse();
    else
        MxcBool_RetTrue();
}

StringObject *true_tostring(MxcObject *ob) {
    (void)ob;
    return new_stringobject("true", false);
}

StringObject *false_tostring(MxcObject *ob) {
    (void)ob;
    return new_stringobject("false", false);
}

MxcObjImpl bool_true_objimpl = {
    "bool",
    true_tostring,
    0,
    bool_copy,
    0,
    0,
    0,
};

MxcObjImpl bool_false_objimpl = {
    "bool",
    false_tostring,
    0,
    bool_copy,
    0,
    0,
    0,
};

BoolObject MxcTrue = {
    {
        &bool_true_objimpl,
        1,  /* refcount */
        0,
    },
    true
};

BoolObject MxcFalse = {
    {
        &bool_false_objimpl,
        1,  /* refcount */
        0,
    },
    false
};

