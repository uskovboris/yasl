#include "list_methods.h"

int list___get(VM *vm) {
    List_t *ls = (List_t *)POP(vm).value;
    Constant index = POP(vm);
    if (index.type != INT64) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else if (index.value < 0 || index.value >= ls->size) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else {
        PUSH(vm, ls->items[index.value]);
    }
    return 0;
}

int list___set(VM* vm) {
    List_t *ls = (List_t*)POP(vm).value;
    Constant value = POP(vm);
    Constant index = POP(vm);
    if (index.type != INT64) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else if (index.value < 0 || index.value >= ls->size) {
        return -1;
        PUSH(vm, UNDEF_C);
    } else {
        ls->items[index.value] = value;
        PUSH(vm, value);
    }
    return 0;
}


int list_append(VM* vm) {
    Constant ls  = POP(vm);
    Constant val = POP(vm);
    ls_append((List_t*)ls.value, val);
    PUSH(vm, UNDEF_C);
    return 0;
}