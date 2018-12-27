#include "list_methods.h"

#include <stdio.h>

#include "VM.h"
#include "YASL_Object.h"
#include "list.h"
#include "yasl_state.h"

int list___get(struct YASL_State *S) {
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__get");
    struct List *ls = YASL_GETLIST(PEEK(S->vm));
    if (!YASL_ISINT(index)) {
        S->vm->sp++;
        return -1;
    } else if (YASL_GETINT(index) < -ls->count || YASL_GETINT(index) >= ls->count) {
        printf("IndexError\n");
        return -1;
    } else {
        if (index.value.ival >= 0) {
            vm_pop(S->vm);
            vm_push(S->vm, ls->items[YASL_GETINT(index)]);
        }
        else {
            vm_pop(S->vm);
            vm_push(S->vm, ls->items[YASL_GETINT(index) + ls->count]);
        }
    }
    return 0;
}

int list___set(struct YASL_State *S) {
    struct YASL_Object value = vm_pop(S->vm);
    struct YASL_Object index = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.__set");
    struct List *ls = YASL_GETLIST(vm_pop(S->vm));
    if (!YASL_ISINT(index)) {
        printf("TypeError: cannot index list with non-integer\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else if (YASL_GETINT(index) < -ls->count || YASL_GETINT(index) >= ls->count) {
        printf("%d || %d\n", YASL_GETINT(index) < -ls->count, YASL_GETINT(index) >= ls->count);
        printf("IndexError\n");
        vm_push(S->vm, YASL_UNDEF());
        return -1;
    } else {
        if (YASL_GETINT(index) >= 0) ls->items[YASL_GETINT(index)] = value;
        else ls->items[YASL_GETINT(index) + ls->count] = value;
        vm_push(S->vm, value);
    }
    return 0;
}

int table_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count);

int list_tostr_helper(struct YASL_State *S, void **buffer, size_t buffer_size, size_t buffer_count) {
	size_t string_count = 0;
	size_t string_size = 8;
	char *string = malloc(string_size);

	string[string_count++] = '[';
	struct List *list = vm_peeklist(S->vm, S->vm->sp);
	if (list->count == 0)    {
		vm_pop(S->vm);
		string[string_count++] = ']';

		vm_push(S->vm, YASL_STR(str_new_sized_heap(0, string_count, string)));
		return 0;
	}

	for (int64_t i = 0; i < list->count; i++) {
		vm_push(S->vm, list->items[i]);

		if (YASL_ISLIST(VM_PEEK(S->vm, S->vm->sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist(S->vm, S->vm->sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + 7 >= string_size) {
					string_size *= 2;
					string = realloc(string, string_size);
				}
				string[string_count++] = '[';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = ']';
				string[string_count++] = ',';
				string[string_count++] = ' ';
				continue;
			} else {
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_size] = vm_peeklist(S->vm, S->vm->sp);
				list_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
			}
		} else if (YASL_ISTBL(VM_PEEK(S->vm, S->vm->sp))) {
			int found = 0;
			for (size_t j = 0; j < buffer_count; j++) {
				if (buffer[j] == vm_peeklist(S->vm, S->vm->sp)) {
					found = 1;
					break;
				}
			}
			if (found) {
				if (string_count + 7 >= string_size) {
					string_size *= 2;
					string = realloc(string, string_size);
				}
				string[string_count++] = '{';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = '.';
				string[string_count++] = '}';
				string[string_count++] = ',';
				string[string_count++] = ' ';
				continue;
			} else {
				size_t tmp_buffer_size = buffer_count == buffer_size ? buffer_size * 2 : buffer_size;
				void **tmp_buffer = malloc(tmp_buffer_size * sizeof(void *));
				memcpy(tmp_buffer, buffer, sizeof(void *) * buffer_count);
				tmp_buffer[buffer_count] = vm_peeklist(S->vm, S->vm->sp);
				table_tostr_helper(S, tmp_buffer, tmp_buffer_size, buffer_size + 1);
				free(tmp_buffer);
			}
		} else {
			YASL_Types index = VM_PEEK(S->vm, S->vm->sp).type;
			struct YASL_Object key = YASL_STR(str_new_sized(strlen("tostr"), "tostr"));
			struct YASL_Object *result = table_search(S->vm->builtins_htable[index], key);
			str_del(YASL_GETSTR(key));
			YASL_GETCFN(*result)->value(S);
		}

		String_t *str = vm_popstr(S->vm);
		while (string_count + yasl_string_len(str) >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		memcpy(string + string_count, str->str + str->start, yasl_string_len(str));
		string_count += yasl_string_len(str);
		// str_del(str);

		if (string_count + 2 >= string_size) {
			string_size *= 2;
			string = realloc(string, string_size);
		}

		string[string_count++] = ',';
		string[string_count++] = ' ';
	}
	vm_pop(S->vm);

	string_count -= 2;
	string[string_count++] = ']';

	vm_push(S->vm, YASL_STR(str_new_sized_heap(0, string_count, string)));

	return 0;
}

int list_tostr(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.tostr");

	void **buffer = malloc(8*sizeof(void *));
	buffer[0] = vm_peeklist(S->vm, S->vm->sp);
	list_tostr_helper(S, buffer, 8, 1);
	free(buffer);

	return 0;
}

/*
void ls_print(struct RC_List* ls) {
    ByteBuffer *seen = bb_new(sizeof(int64_t)*2);
    ls_print_h(ls, seen);
}

void ls_print_h(struct RC_List* ls, ByteBuffer *seen) {
    int i = 0;
    if (ls->list->count == 0) {
        printf("[]");
        return;
    }
    printf("[");
    while (i < ls->list->count) {
        if (YASL_ISLIST(ls->list->items[i])) {
            if (isvalueinarray(ls->list->items[i].value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...]");
            } else {
                bb_intbytes8(seen, (int64_t)ls);
                bb_intbytes8(seen, ls->list->items[i].value.ival);
                ls_print_h(ls->list->items[i].value.lval, seen);
            }
        } else if (YASL_ISTBL(ls->list->items[i])) {
            if (isvalueinarray(ls->list->items[i].value.ival, (int64_t*)seen->bytes, seen->count/sizeof(int64_t))) {
                printf("[...->...]");
            } else {
                bb_intbytes8(seen, (int64_t)ls);
                bb_intbytes8(seen, ls->list->items[i].value.ival);
                ht_print_h(ls->list->items[i].value.mval, seen);
            }
        } else {
            print(ls->list->items[i]);
        }
        printf(", ");
        i++;
    }
    printf("\b\b]");
}
*/

int list_push(struct YASL_State *S) {
    struct YASL_Object val = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.push");
    ls_append(YASL_GETLIST(PEEK(S->vm)), val);
    return 0;
}

int list_copy(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.copy");
    struct List *ls = YASL_GETLIST(vm_pop(S->vm));
    struct RC_UserData *new_ls = ls_new_sized(ls->size);
    ((struct List *)new_ls->data)->count = ls->count;
    memcpy(((struct List *)new_ls->data)->items, ls->items, ((struct List *)new_ls->data)->count*sizeof(struct YASL_Object));

    vm_push(S->vm, YASL_LIST(new_ls));
    return 0;
}

int list_extend(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    struct List *extend_ls = YASL_GETLIST(vm_pop(S->vm));
    ASSERT_TYPE(S->vm, Y_LIST, "list.extend");
    struct List *ls  = YASL_GETLIST(vm_pop(S->vm));

    struct List *exls = extend_ls;
    for(int64_t i = 0; i < exls->count; i++) {
        ls_append(ls, exls->items[i]);
    }
    vm_push(S->vm, YASL_UNDEF());
    return 0;
}

int list_pop(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.pop");
    struct List *ls  = YASL_GETLIST(vm_pop(S->vm));
    if (ls->count == 0) {
        puts("cannot pop from empty list.");
        exit(EXIT_FAILURE);
    }
    vm_push(S->vm, ls->items[--ls->count]);
    return 0;
}

int list_search(struct YASL_State *S) {
    struct YASL_Object needle = vm_pop(S->vm);
    ASSERT_TYPE(S->vm, Y_LIST, "list.search");
    struct List *haystack = YASL_GETLIST(vm_pop(S->vm));
    struct YASL_Object index = YASL_UNDEF();
    for (int64_t i = 0; i < haystack->count; i++) {
        if (!isfalsey(isequal(haystack->items[i], needle)))
            index = YASL_INT(i);
    }
    vm_push(S->vm, index);
    return 0;
}

int list_reverse(struct YASL_State *S) {
    ASSERT_TYPE(S->vm, Y_LIST, "list.reverse");
    struct List *ls = YASL_GETLIST(vm_pop(S->vm));
    ls_reverse(ls);
    vm_push(S->vm, YASL_UNDEF());
    return 0;
}

int list_slice(struct YASL_State *S) {
	struct YASL_Object end_index = vm_pop(S->vm);
	struct YASL_Object start_index = vm_pop(S->vm);
	ASSERT_TYPE(S->vm, Y_LIST, "list.slice");
	struct List *list = YASL_GETLIST(vm_pop(S->vm));
	if (!YASL_ISINT(start_index) || !YASL_ISINT(end_index)) {
		return -1;
	} else if (YASL_GETINT(start_index) < -list->count ||
		   YASL_GETINT(start_index) > list->count) {
		return -1;
	} else if (YASL_GETINT(end_index) < -list->count || YASL_GETINT(end_index) > list->count) {
		return -1;
	}

	int64_t start = YASL_GETINT(start_index) < 0 ? YASL_GETINT(start_index) + list->count : YASL_GETINT(
		start_index);
	int64_t end =
		YASL_GETINT(end_index) < 0 ? YASL_GETINT(end_index) + list->count : YASL_GETINT(end_index);

	if (start > end) {
		return -1;
	}

	struct RC_UserData *new_list = ls_new_sized(end - start);

	for (int64_t i = start; i < end; i++) {
		ls_append(new_list->data, list->items[i]); // = list->items[i];
		inc_ref(list->items + i);
	}

	vm_push(S->vm, YASL_LIST(new_list));

	return 0;
}

int list_clear(struct YASL_State *S) {
	ASSERT_TYPE(S->vm, Y_LIST, "list.clear");
	struct List *list = vm_poplist(S->vm);
	for (int64_t i = 0; i < list->count; i++) {
		dec_ref(list->items + i);
	}
	list->count = 0;
	list->size = LS_BASESIZE;
	list->items = realloc(list->items, sizeof(struct YASL_Object)*list->size);
	vm_pushundef(S->vm);
	return 0;
}