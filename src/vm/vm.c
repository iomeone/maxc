#include "vm.h"
#include "ast.h"
#include "bytecode.h"
#include "error/error.h"
#include "error/runtime-err.h"
#include "literalpool.h"
#include "maxc.h"
#include "object/object.h"
#include "builtins.h"

#define DPTEST

static int vm_exec(Frame *);

int error_flag = 0;

extern bltinfn_ty bltinfns[];

#ifndef DPTEST
#define Dispatch() goto *codetable[(frame->code[frame->pc])]
#else
#define DISPATCH_CASE(name, smallname)                                         \
    case OP_##name:                                                            \
        goto code_##smallname;

#define Dispatch()                                                             \
    do {                                                                       \
        switch(frame->code[frame->pc]) {                                       \
            DISPATCH_CASE(END, end)                                            \
            DISPATCH_CASE(IPUSH, ipush)                                        \
            DISPATCH_CASE(CPUSH, cpush)                                        \
            DISPATCH_CASE(FPUSH, fpush)                                        \
            DISPATCH_CASE(LPUSH, lpush)                                        \
            DISPATCH_CASE(LOAD_GLOBAL, load_global)                            \
            DISPATCH_CASE(LOAD_LOCAL, load_local)                              \
            DISPATCH_CASE(RET, ret)                                            \
            DISPATCH_CASE(STORE_LOCAL, store_local)                            \
            DISPATCH_CASE(STORE_GLOBAL, store_global)                          \
            DISPATCH_CASE(CALL, call)                                          \
            DISPATCH_CASE(PUSHCONST_0, pushconst_0)                            \
            DISPATCH_CASE(PUSHCONST_1, pushconst_1)                            \
            DISPATCH_CASE(PUSHCONST_2, pushconst_2)                            \
            DISPATCH_CASE(PUSHCONST_3, pushconst_3)                            \
            DISPATCH_CASE(PUSHTRUE, pushtrue)                                  \
            DISPATCH_CASE(PUSHFALSE, pushfalse)                                \
            DISPATCH_CASE(PUSHNULL, pushnull)                                  \
            DISPATCH_CASE(LTE, lte)                                            \
            DISPATCH_CASE(LT, lt)                                              \
            DISPATCH_CASE(GT, gt)                                              \
            DISPATCH_CASE(GTE, gte)                                            \
            DISPATCH_CASE(EQ, eq)                                              \
            DISPATCH_CASE(FEQ, feq)                                            \
            DISPATCH_CASE(NOTEQ, noteq)                                        \
            DISPATCH_CASE(FNOTEQ, fnoteq)                                      \
            DISPATCH_CASE(JMP_NOTEQ, jmp_noteq)                                \
            DISPATCH_CASE(JMP_EQ, jmp_eq)                                \
            DISPATCH_CASE(JMP, jmp)                                            \
            DISPATCH_CASE(JMP_NOTERR, jmp_noterr)                              \
            DISPATCH_CASE(SUB, sub)                                            \
            DISPATCH_CASE(ADD, add)                                            \
            DISPATCH_CASE(MUL, mul)                                            \
            DISPATCH_CASE(DIV, div)                                            \
            DISPATCH_CASE(MOD, mod)                                            \
            DISPATCH_CASE(FADD, fadd)                                          \
            DISPATCH_CASE(FSUB, fsub)                                          \
            DISPATCH_CASE(FMUL, fmul)                                          \
            DISPATCH_CASE(FDIV, fdiv)                                          \
            DISPATCH_CASE(FMOD, fmod)                                          \
            DISPATCH_CASE(FLT, flt)                                            \
            DISPATCH_CASE(FLTE, flte)                                          \
            DISPATCH_CASE(FGT, fgt)                                            \
            DISPATCH_CASE(FGTE, fgte)                                          \
            DISPATCH_CASE(INC, inc)                                            \
            DISPATCH_CASE(DEC, dec)                                            \
            DISPATCH_CASE(NOT, not)                                            \
            DISPATCH_CASE(INEG, ineg)                                          \
            DISPATCH_CASE(FNEG, fneg)                                          \
            DISPATCH_CASE(LOGAND, logand)                                      \
            DISPATCH_CASE(LOGOR, logor)                                        \
            DISPATCH_CASE(FLOGAND, flogand)                                    \
            DISPATCH_CASE(FLOGOR, flogor)                                      \
            DISPATCH_CASE(BLTINFN_SET, bltinfnset)                             \
            DISPATCH_CASE(CALL_BLTIN, call_bltin)                              \
            DISPATCH_CASE(POP, pop)                                            \
            DISPATCH_CASE(STRINGSET, stringset)                                \
            DISPATCH_CASE(SUBSCR, subscr)                                      \
            DISPATCH_CASE(SUBSCR_STORE, subscr_store)                          \
            DISPATCH_CASE(STRUCTSET, structset)                                \
            DISPATCH_CASE(LISTSET, listset)                                    \
            DISPATCH_CASE(LISTSET_SIZE, listset_size)                          \
            DISPATCH_CASE(LISTLENGTH, listlength)                              \
            DISPATCH_CASE(FUNCTIONSET, functionset)                            \
            DISPATCH_CASE(TUPLESET, tupleset)                                  \
            DISPATCH_CASE(MEMBER_LOAD, member_load)                            \
            DISPATCH_CASE(MEMBER_STORE, member_store)                          \
            DISPATCH_CASE(ITER_NEXT, iter_next)                                \
            DISPATCH_CASE(STRCAT, strcat)                                      \
        default:                                                               \
            printf("err:%d\n", frame->code[frame->pc]);                        \
            mxc_raise_err(frame, RTERR_UNIMPLEMENTED);                         \
        }                                                                      \
    } while(0)
#endif

#define List_Setitem(ob, index, item) (ob->elem[index] = (item))
#define List_Getitem(ob, index) (ob->elem[index])

#define Exit_Frame()    \
    do {    \
        ;   \
    } while(0)

#define Member_Getitem(ob, offset) (ob->field[offset])
#define Member_Setitem(ob, offset, item) (ob->field[offset] = (item))

#define READ_i32(code, pc)                                                     \
    ((int64_t)(((uint8_t)code[(pc) + 3] << 24) + ((uint8_t)code[(pc) + 2] << 16) + \
               ((uint8_t)code[(pc) + 1] << 8) + ((uint8_t)code[(pc) + 0])))

#define READ_i8(code, pc) ((int64_t)(code[(pc)]))

#define CASE(op) op:

int VM_run(Frame *frame) {
#ifdef MXC_DEBUG
    printf(MUTED("ptr: %p")"\n", frame->stackptr);
#endif

    int ret = vm_exec(frame);

#ifdef MXC_DEBUG
    printf(MUTED("ptr: %p")"\n", frame->stackptr);
#endif

    return ret;
}

static int vm_exec(Frame *frame) {

#define Push(ob) (*frame->stackptr++ = (MxcObject *)(ob))
#define Pop() (*--frame->stackptr)
#define Top() (frame->stackptr[-1])
#define SetTop(ob) (frame->stackptr[-1] = ((MxcObject *)(ob)))

#ifndef DPTEST
    static const void *codetable[] = {
        &&code_end,          &&code_push,         &&code_ipush,
        &&code_pushconst_0,  &&code_pushconst_1,  &&code_pushconst_2,
        &&code_pushconst_3,  &&code_pushtrue,     &&code_pushfalse,
        &&code_fpush,        &&code_pop,          &&code_add,
        &&code_sub,          &&code_mul,          &&code_div,
        &&code_mod,          &&code_logor,        &&code_logand,
        &&code_eq,           &&code_noteq,        &&code_lt,
        &&code_lte,          &&code_gt,           &&code_gte,
        &&code_fadd,         &&code_fsub,         &&code_fmul,
        &&code_fdiv,         &&code_fmod,         &&code_flogor,
        &&code_flogand,      &&code_feq,          &&code_fnoteq,
        &&code_flt,          &&code_flte,         &&code_fgt,
        &&code_fgte,         &&code_jmp,          &&code_jmp_eq,
        &&code_jmp_noteq,    &&code_inc,          &&code_dec,
        &&code_load_global,  &&code_load_local,   &&code_store_global,
        &&code_store_local,  &&code_listset,      &&code_subscr,
        &&code_subscr_store, &&code_stringset,    &&code_tupleset,
        &&code_functionset,  &&code_bltinfnset,   &&code_structset,
        &&code_ret,          &&code_call,         &&code_call_bltin,
        &&code_member_load,  &&code_member_store,
    };
#endif

    MxcObject **gvmap = frame->gvars;

    Frame *new_frame;
    int key;

    Dispatch();

    CASE(code_ipush) {
        Push(new_intobject(READ_i32(frame->code, frame->pc + 1)));
        frame->pc += 5;

        Dispatch();
    }
    CASE(code_cpush) {
        ++frame->pc;
        Push(new_charobject(READ_i8(frame->code, frame->pc)));
        ++frame->pc;

        Dispatch();
    }
    CASE(code_lpush) {
        ++frame->pc;
        key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        Push(new_intobject(((Literal *)ltable->data[key])->lnum));

        Dispatch();
    }
    CASE(code_pushconst_0) {
        ++frame->pc;
        Push(new_intobject(0));

        Dispatch();
    }
    CASE(code_pushconst_1) {
        ++frame->pc;
        Push(new_intobject(1));

        Dispatch();
    }
    CASE(code_pushconst_2) {
        ++frame->pc;
        Push(new_intobject(2));

        Dispatch();
    }
    CASE(code_pushconst_3) {
        ++frame->pc;
        Push(new_intobject(3));

        Dispatch();
    }
    CASE(code_pushtrue) {
        ++frame->pc;
        Push(&MxcTrue);
        INCREF(&MxcTrue);

        Dispatch();
    }
    CASE(code_pushfalse) {
        ++frame->pc;
        Push(&MxcFalse);
        INCREF(&MxcFalse);

        Dispatch();
    }
    CASE(code_pushnull) {
        ++frame->pc;
        Push(&MxcNull);
        INCREF(&MxcNull);

        Dispatch();
    }
    CASE(code_fpush){
        ++frame->pc;

        key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        Push(new_floatobject(((Literal *)ltable->data[key])->fnumber));

        Dispatch();
    }
    CASE(code_pop) {
        ++frame->pc;
        (void)Pop();

        Dispatch();
    }
    CASE(code_add) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(IntAdd(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_fadd) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(FloatAdd(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_strcat) {
        ++frame->pc;

        StringObject *r = (StringObject *)Pop();
        StringObject *l = (StringObject *)Top();

        SetTop(str_concat(l, r));

        Dispatch();
    }
    CASE(code_sub) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(IntSub(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_fsub) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(FloatSub(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_mul) {
        ++frame->pc; // mul

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(IntMul(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_fmul) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(FloatMul(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_div) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        MxcObject *res = (MxcObject *)int_div(l, r);
        if(!res) {
            mxc_raise_err(frame, RTERR_ZERO_DIVISION);
            goto exit_failure;
        }

        SetTop(res);
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_fdiv) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        MxcObject *res = (MxcObject *)float_div(l, r);
        if(!res) {
            mxc_raise_err(frame, RTERR_ZERO_DIVISION);
            goto exit_failure;
        }

        SetTop(res);
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_mod) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_mod(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_logor) {
        ++frame->pc;

        BoolObject *r = (BoolObject *)Pop();
        BoolObject *l = (BoolObject *)Top();

        SetTop(bool_logor(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_logand) {
        ++frame->pc;

        BoolObject *r = (BoolObject *)Pop();
        BoolObject *l = (BoolObject *)Top();

        SetTop(bool_logand(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_eq) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_eq(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_feq) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_eq(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_noteq) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_noteq(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_fnoteq) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_neq(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_lt) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_lt(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_flt) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_lt(l, r));

        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_lte) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_lte(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_gt) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_gt(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_fgt) {
        ++frame->pc;

        FloatObject *r = (FloatObject *)Pop();
        FloatObject *l = (FloatObject *)Top();

        SetTop(float_gt(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_gte) {
        ++frame->pc;

        IntObject *r = (IntObject *)Pop();
        IntObject *l = (IntObject *)Top();

        SetTop(int_gte(l, r));
        DECREF((MxcObject *)r);
        DECREF((MxcObject *)l);

        Dispatch();
    }
    CASE(code_inc) {
        ++frame->pc;

        IntObject *u = (IntObject *)Top();
        ++u->inum;

        Dispatch();
    }
    CASE(code_dec) {
        ++frame->pc;

        IntObject *u = (IntObject *)Top();
        --u->inum;

        Dispatch();
    }
    CASE(code_ineg) {
        ++frame->pc;

        IntObject *u = (IntObject *)Top();
        SetTop(new_intobject(-(u->inum)));
        DECREF((MxcObject *)u);

        Dispatch();
    }
    CASE(code_fneg) {
        ++frame->pc;

        FloatObject *u = (FloatObject *)Top();
        SetTop(new_floatobject(-(u->fnum)));
        DECREF((MxcObject *)u);

        Dispatch();
    }
    CASE(code_not) {
        ++frame->pc;

        BoolObject *b = (BoolObject *)Top();
        SetTop(bool_not(b));
        DECREF((MxcObject *)b);

        Dispatch();
    }
    CASE(code_store_global) {
        key = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        MxcObject *old = gvmap[key];
        if(old) {
            DECREF(old);
        }

        gvmap[key] = Top();

        Dispatch();
    }
    CASE(code_store_local) {
        key = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        MxcObject *old = (MxcObject *)frame->lvars[key];
        if(old) {
            DECREF(old);
        }

        frame->lvars[key] = Top();

        Dispatch();
    }
    CASE(code_load_global) {
        key = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        MxcObject *ob = gvmap[key];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(code_load_local) {
        key = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        MxcObject *ob = (MxcObject *)frame->lvars[key];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(code_jmp) {
        ++frame->pc;

        frame->pc = READ_i32(frame->code, frame->pc);

        Dispatch();
    }
    CASE(code_jmp_eq) {
        ++frame->pc;

        IntObject *a = (IntObject *)Pop();
        if(a->inum == 1)
            frame->pc = READ_i32(frame->code, frame->pc);
        else
            frame->pc += 4;

        DECREF((MxcObject *)a);

        Dispatch();
    }
    CASE(code_jmp_noteq) {
        ++frame->pc;

        IntObject *a = (IntObject *)Pop();
        if(a->inum == 0)
            frame->pc = READ_i32(frame->code, frame->pc);
        else
            frame->pc += 4; // skip arg

        DECREF((MxcObject *)a);

        Dispatch();
    }
    CASE(code_jmp_noterr) {
        ++frame->pc;

        if(!error_flag) {
            frame->pc = READ_i32(frame->code, frame->pc);
        }
        else {
            frame->pc += 4;
        }

        error_flag--;

        Dispatch();
    }
    CASE(code_listset) {
        ++frame->pc;

        int n = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        ListObject *ob = new_listobject(n);

        ((MxcIterable *)ob)->next = Top();

        for(int i = 0; i < n; ++i) {
            List_Setitem(ob, i, Pop());
        }
        Push(ob);

        Dispatch();
    }
    CASE(code_listset_size) {
        ++frame->pc;

        IntObject *n = (IntObject *)Pop();
        MxcObject *init = Pop();

        ListObject *ob = new_listobject_size(n, init);

        ((MxcIterable *)ob)->next = init;

        Push(ob);

        Dispatch();
    }
    CASE(code_listlength) {
        ++frame->pc;

        ListObject *ls = (ListObject *)Pop();

        Push(new_intobject(ls->size));

        Dispatch();
    }
    CASE(code_subscr) {
        ++frame->pc;

        MxcIterable *ls = (MxcIterable *)Pop();
        IntObject *idx = (IntObject *)Pop();
        MxcObject *ob = ls->get(ls, idx->inum);
        if(!ob) {
            raise_outofrange(frame,
                             (MxcObject *)idx,
                             (MxcObject *)new_intobject(ls->length));
            goto exit_failure;
        }
        DECREF((MxcObject *)ls);
        DECREF((MxcObject *)idx);
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
    CASE(code_subscr_store) {
        ++frame->pc;
        MxcIterable *ob = (MxcIterable *)Pop();
        IntObject *idx = (IntObject *)Pop();
        MxcObject *top = Top();
        if(!ob->set(ob, idx->inum, top)) {
            raise_outofrange(frame,
                             (MxcObject *)idx,
                             (MxcObject *)new_intobject(ob->length));
            goto exit_failure;
        }

        DECREF((MxcObject *)ob);
        DECREF((MxcObject *)idx);
        INCREF(top);

        Dispatch();
    }
    CASE(code_stringset) {
        ++frame->pc;
        key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        Push(new_stringobject(((Literal *)ltable->data[key])->str));

        Dispatch();
    }
    CASE(code_tupleset) {
        ++frame->pc;

        Dispatch();
    }
    CASE(code_functionset) {
        key = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        Push(new_functionobject(((Literal *)ltable->data[key])->func));

        Dispatch();
    }
    CASE(code_bltinfnset) {
        key = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        Push(new_bltinfnobject(bltinfns[key]));

        Dispatch();
    }
    CASE(code_structset) {
        ++frame->pc;

        int nfield = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        Push(new_structobject(nfield));

        Dispatch();
    }
    CASE(code_call) {
        ++frame->pc;

        FunctionObject *callee = (FunctionObject *)Pop();

        new_frame = New_Frame(callee->func, frame);

        int res = vm_exec(new_frame);

        frame = new_frame->prev;
        frame->stackptr = new_frame->stackptr;
        Delete_Frame(new_frame);

        if(res) {
            return 1;
        }

        Dispatch();
    }
    CASE(code_call_bltin) {
        int nargs = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        BltinFuncObject *callee = (BltinFuncObject *)Pop();

        frame->stackptr -= nargs;
        MxcObject *ret = callee->func(frame->stackptr, nargs);
        DECREF((MxcObject *)callee);

        Push(ret);

        Dispatch();
    }
    CASE(code_member_load) {
        int offset = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        StructObject *ob = (StructObject *)Pop();
        MxcObject *data = Member_Getitem(ob, offset);
        INCREF(data);

        Push(data);

        Dispatch();
    }
    CASE(code_member_store) {
        int offset = READ_i32(frame->code, frame->pc + 1);
        frame->pc += 5;

        StructObject *ob = (StructObject *)Pop();
        MxcObject *data = Top();

        Member_Setitem(ob, offset, data);

        Dispatch();
    }
    CASE(code_iter_next) {
        ++frame->pc;

        MxcIterable *iter = (MxcIterable *)Top();
        MxcObject *res = iterable_next(iter); 
        if(!res) {
            frame->pc = READ_i32(frame->code, frame->pc); 
        }
        else {
            frame->pc += 4;
        }
        Push(res);

        Dispatch();
    }
    CASE(code_ret) {
        ++frame->pc;

        for(size_t i = 0; i < frame->nlvars; ++i) {
            if(frame->lvars[i])
                DECREF(frame->lvars[i]);
        }

        return 0;
    }
    CASE(code_end) {
        /* exit_success */
        return 0;
    }
    // TODO
    CASE(code_flogor)
    CASE(code_flogand)
    CASE(code_fmod)
    CASE(code_flte)
    CASE(code_fgte) {
        mxc_raise_err(frame, RTERR_UNIMPLEMENTED);
        goto exit_failure;
    }

exit_failure:
    runtime_error(frame->occurred_rterr);

    return 1;
}
