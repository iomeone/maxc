/* implementation of iterator object */

#include "object/iterobject.h"
#include "error/error.h"
#include "mem.h"
#include "vm.h"

MxcObject *iterable_next(MxcIterable *iter) {
    if(!iter->next) {
        return NULL;
    }

    MxcObject *res = OBJIMPL(iter)->get(iter, iter->index);
    iter->index++;

    return res;
}
