#include <stdlib.h>

#include "frame.h"
#include "error/error.h"

Frame *New_Global_Frame(Bytecode *c, int ngvar) {
    Frame *f = malloc(sizeof(Frame));

    f->prev = NULL;
    f->func_name = "<global>";
    f->code = c ? c->code : NULL;
    f->codesize = c ? c->len : 0;
    f->pc = 0;

    f->gvars = malloc(sizeof(struct MxcObject *) * ngvar);
    for(int i = 0; i < ngvar; ++i) {
        f->gvars[i] = NULL;
    }
    f->lvars = NULL;

    f->stackptr = (MxcObject **)malloc(sizeof(MxcObject *) * 1000);
    f->occurred_rterr.type = RTERR_NONEERR; 

    return f;
}

Frame *New_Frame(userfunction *u, Frame *prev) {
    Frame *f = malloc(sizeof(Frame));

    f->prev = prev;
    f->func_name = u->name;
    f->code = u->code;
    f->codesize = u->codesize;

    f->lvar_info = u->var_info;
    f->lvars = malloc(sizeof(struct MxcObject *) * u->nlvars);
    for(int i = 0; i < u->nlvars; ++i) {
        f->lvars[i] = NULL;
    }

    f->gvars = prev->gvars;
    f->pc = 0;
    f->nlvars = u->nlvars;
    f->stackptr = prev->stackptr;

    return f;
}

void Delete_Frame(Frame *f) {
    free(f->lvars);
    free(f);
}
