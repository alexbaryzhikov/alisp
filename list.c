/*
List: a collection of objects.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alisp.h"

/*
--------------------------------------
list
    
    Make a list.
--------------------------------------
*/
atom_t* list() {

    // Make a list
    list_t* l = malloc(sizeof(list_t));
    l->bindlist = NULL;
    l->lock = 0;
    l->len = 0;
    l->maxlen = 2;
    l->items = malloc(l->maxlen * sizeof(atom_t*));

    // Make a list object
    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.list = l;
    obj->type = LIST;
    obj->bindings = 0;

    return obj;
}

/*
--------------------------------------
list_del
    
    Deallocate a list.
--------------------------------------
*/
void list_del(atom_t* obj) {
    list_assert(obj, "list_del");

#ifdef DEBUG
char* dbg_s = atom_tostr(obj);
printf("....  list_del:                Deallocating %s\n", dbg_s);
safe_free(dbg_s);
#endif

    list_t* l = obj->val.list;
    atom_t* item;
    l->lock = 1;  // lock this object

    // Deallocate bound objects
    for (int i = 0; i < l->len; ++i) {
        item = l->items[i];
        if (!(atom_is_container(item) && item->val.list->lock)) {
            atom_unbind(item, obj);
            atom_del(item);  // free item object
        }
    }

    // Deallocate the rest
    safe_free(l->items);  // free items
    list_free(l->bindlist);
    safe_free(l);         // free list
    safe_free(obj);       // free object
}

/*
--------------------------------------
list_cp

    Copy a list (subroutine for atom_cp).
--------------------------------------
*/
atom_t* list_cp(atom_t* obj, atom_t* objects, atom_t* copies) {
    int i = list_idx(objects, obj);
    if (i != -1)
        return copies->val.list->items[i];  // object already has a copy

    atom_t* copy = list();
    list_add_h(objects, obj);
    list_add_h(copies, copy);

    atom_t** items = obj->val.list->items;
    for (i = 0; i < list_len(obj); ++i)
        list_add(copy, atom_cp(items[i], objects, copies));
    return copy;
}

/*
--------------------------------------
list_insert

    Insert item to list at given index.
--------------------------------------
*/
void list_insert(atom_t* obj, int index, atom_t* item, char bind) {
    list_assert(obj, "list_ins");
    if (!item) {
        printf("\x1b[95m" "Fatal error: list_ins: bad item!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    list_t* l = obj->val.list;

    // Allocate more space if necessary
    if (l->len == l->maxlen) {
        l->maxlen *= 1.5;
        l->items = realloc(l->items, l->maxlen * sizeof(atom_t*));
    }

    if (index > l->len || index < 0)
        // insert item at the end of the list
        index = l->len;
    else
        // shift items to free space
        memmove(l->items + index + 1, l->items + index, (l->len - index) * sizeof(atom_t*));

    // Insert item
    l->items[index] = item;
    ++l->len;
    if (bind)
        atom_bind(item, obj);
}

/*
--------------------------------------
list_idx

    Return an index of an item in list, or -1.
--------------------------------------
*/
int list_idx(atom_t* obj, atom_t* item) {
    list_assert(obj, "list_idx");
    if (!item) {
        printf("\x1b[95m" "Fatal error: list_idx: bad item!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < list_len(obj); ++i)
        if (obj->val.list->items[i] == item)
            break;
    if (i == list_len(obj))
        i = -1;
    return i;
}

/*
--------------------------------------
list_lookup

    Object lookup in a SORTED list.
    Returns 1 if object is found, 0 otherwise. Leaves index in *idx.
--------------------------------------
*/
int list_lookup(atom_t* obj, atom_t* item, int* idx) {
    list_assert(obj, "list_lookup");
    *idx = 0;

    if (!list_len(obj))  // list is empty
        return 0;

    atom_t** items = obj->val.list->items;
    int hit = 0, left = 1, right = list_len(obj);

    while (left <= right) {
        *idx = left + ((right - left) >> 1);
        if (items[*idx-1] > item) {
            if ((right = *idx - 1) < left) {
                (*idx)--;
                break;
            }
        } else if (items[*idx-1] < item) {
            left = *idx + 1;
        } else {
            hit = 1;
            (*idx)--;
            break;
        }
    }

    return hit;
}

/*
--------------------------------------
list_remove

    Remove item at index from the list.
--------------------------------------
*/
void list_remove(atom_t* obj, int index, char unbind) {
    list_assert(obj, "list_rem");
    if (index >= list_len(obj) || index < 0) {
        printf("\x1b[95m" "Fatal error: list_rem: index is out of range!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    list_t* l = obj->val.list;

    if (unbind) {
        atom_unbind(l->items[index], obj);
        atom_del(l->items[index]);
    }

    memmove(l->items + index, l->items + index + 1, (l->len - index - 1) * sizeof(atom_t*));
    --l->len;
}

/*
--------------------------------------
list_tostr

    Make a stiring representing list.
--------------------------------------
*/
char* list_tostr(atom_t* obj, int depth) {
    list_assert(obj, "list_tostr");
    atom_t** items = obj->val.list->items;
    char* o;
    char buf[1024];
    char ellipsis = 0;
    strcpy(buf, "(");

    for (int i = 0; i < list_len(obj); ++i) {

        if (ellipsis || strlen(buf) > 1000) {
            strcat(buf, " ... ");
            break;
        }

        o = atom_tostring(items[i], depth ? depth - 1 : depth);
        if (strlen(buf) + strlen(o) > 1000)
            ellipsis = 1;
        else
            strcat(buf, o);
        safe_free(o);

        if (i < list_len(obj) - 1)
            strcat(buf, " ");
    }

    strcat(buf, ")");
    o = malloc(strlen(buf) + 1);
    strcpy(o, buf);
    return o;
}

/*
--------------------------------------
list_print

    Print a list.
--------------------------------------
*/
void list_print(atom_t* obj, int depth) {
    list_assert(obj, "list_print");
    atom_t** items = obj->val.list->items;
    char* o;
    printf("(");
    for (int i = 0; i < list_len(obj); ++i) {

        o = atom_tostring(items[i], depth ? depth - 1 : depth);
        printf("%s", o);
        safe_free(o);

        if (i < list_len(obj) - 1)
            putchar(' ');
    }
    printf(")\n");
}

/*
--------------------------------------
list_assert

    Assert that object exists and is of LIST type.
--------------------------------------
*/
void list_assert(atom_t* obj, const char* fname) {
    if (!(obj && obj->type == LIST)) {
        printf("\x1b[95m" "Fatal error: %s: not a list!\n" "\x1b[0m", fname);
        exit(EXIT_FAILURE);
    }
}

/*
--------------------------------------
list_len

    Return list length.
--------------------------------------
*/
int list_len(atom_t* obj) {
    return obj->val.list->len;
}

/*
--------------------------------------
list_maxlen

    Return list maximum length.
--------------------------------------
*/
int list_maxlen(atom_t* obj) {
    return obj->val.list->maxlen;
}

/*
--------------------------------------
list_free

    Deallocate list object, ignore items.
--------------------------------------
*/
void list_free(atom_t* obj) {
    if (obj) {
        safe_free(obj->val.list->items);  // free items
        safe_free(obj->val.list);         // free list
        safe_free(obj);                   // free object
    }
}
