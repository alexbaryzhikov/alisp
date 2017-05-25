/*
List: a collection of objects.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

/*
--------------------------------------
lst
    
    Make a list.
--------------------------------------
*/
atom_t* lst() {

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
lst_del
    
    Deallocate a list.
--------------------------------------
*/
void lst_del(atom_t* obj) {
    lst_assert(obj, "lst_del");

#ifdef DEBUG
char* dbg_s = atom_tostr(obj);
printf("....  lst_del:                 Deallocating %s\n", dbg_s);
safe_free(dbg_s);
#endif

    int i;
    list_t* l = obj->val.list;
    atom_t* item;
    l->lock = 1;  // lock this object

    // Deallocate bound objects
    for (i = 0; i < l->len; ++i) {
        item = l->items[i];
        if (!(atom_is_container(item) && item->val.list->lock)) {
            atom_unbind(item, obj);
            atom_del(item);  // free item object
        }
    }

    // Deallocate the rest
    safe_free(l->items);  // free items
    lst_free(l->bindlist);
    safe_free(l);         // free list
    safe_free(obj);       // free object
}

/*
--------------------------------------
lst_cp

    Copy a list (subroutine for atom_cp).
--------------------------------------
*/
atom_t* lst_cp(atom_t* obj, atom_t* objects, atom_t* copies) {
    int i = lst_idx(objects, obj);
    if (i != -1)
        return copies->val.list->items[i];  // object already has a copy

    atom_t* copy = lst();
    lst_add_nobind(objects, obj);
    lst_add_nobind(copies, copy);

    atom_t** items = obj->val.list->items;
    for (i = 0; i < lst_len(obj); ++i)
        lst_add(copy, atom_cp(items[i], objects, copies));
    return copy;
}

/*
--------------------------------------
lst_insert

    Insert item to list at given index.
--------------------------------------
*/
void lst_insert(atom_t* list, int index, atom_t* item, char bind) {
    lst_assert(list, "lst_ins");
    if (!item) {
        printf("\x1b[95m" "Fatal error: lst_ins: bad item!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    list_t* l = list->val.list;

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
        atom_bind(item, list);
}

/*
--------------------------------------
lst_idx

    Return an index of an item in list, or -1.
--------------------------------------
*/
int lst_idx(atom_t* list, atom_t* item) {
    lst_assert(list, "lst_idx");
    if (!item) {
        printf("\x1b[95m" "Fatal error: lst_idx: bad item!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < lst_len(list); ++i)
        if (list->val.list->items[i] == item)
            break;
    if (i == lst_len(list))
        i = -1;
    return i;
}

/*
--------------------------------------
lst_remove

    Remove item at index from the list.
--------------------------------------
*/
void lst_remove(atom_t* list, int index, char unbind) {
    lst_assert(list, "lst_rem");
    if (index >= lst_len(list) || index < 0) {
        printf("\x1b[95m" "Fatal error: lst_rem: index is out of range!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    list_t* l = list->val.list;

    if (unbind) {
        atom_unbind(l->items[index], list);
        atom_del(l->items[index]);
    }

    memmove(l->items + index, l->items + index + 1, (l->len - index - 1) * sizeof(atom_t*));
    --l->len;
}

/*
--------------------------------------
lst_tostr

    Make a stiring representing list.
--------------------------------------
*/
char* lst_tostr(atom_t* list, int depth) {
    char buf[1024];
    char* o;
    char ellipsis = 0;
    atom_t** items = list->val.list->items;
    strcpy(buf, "(");

    for (int i = 0; i < lst_len(list); ++i) {

        if (ellipsis || strlen(buf) > 1000) {
            strcat(buf, " ... ");
            break;

        } else if (items[i]->type == LIST) {
            if (depth) {
                o = lst_tostr(items[i], depth - 1);
                if (strlen(buf) + strlen(o) > 1000)
                    ellipsis = 1;
                else
                    strcat(buf, o);
                safe_free(o);
            } else {
                strcat(buf, "(...)");
            }

        } else {
            o = atom_tostr(items[i]);
            if (strlen(buf) + strlen(o) > 1000)
                ellipsis = 1;
            else
                strcat(buf, o);
            safe_free(o);
        }

        if (i < lst_len(list) - 1)
            strcat(buf, " ");
    }

    strcat(buf, ")");
    o = malloc(strlen(buf) + 1);
    strcpy(o, buf);
    return o;
}

/*
--------------------------------------
lst_print

    Print a list.
--------------------------------------
*/
void lst_print(atom_t* list, int depth) {
    lst_assert(list, "lst_print");
    lst_pr(list, depth);
    putchar('\n');
}

/* Print list recursively. */
void lst_pr(atom_t* list, int depth) {
    atom_t** items = list->val.list->items;
    char* o;
    printf("(");
    for (int i = 0; i < lst_len(list); ++i) {
        if (items[i]->type == LIST) {
            if (depth)
                lst_pr(items[i], depth - 1);
            else
                printf("(...)");
        } else {
            o = atom_tostr(items[i]);
            printf("%s", o);
            safe_free(o);
        }
        if (i < lst_len(list) - 1)
            putchar(' ');
    }
    printf(")");
}


// ---------------------------------------------------------------------- 
// Auxiliary

/* Assert that object exists and is of LIST type. */
void lst_assert(atom_t* obj, const char* fname) {
    if (!(obj && obj->type == LIST)) {
        printf("\x1b[95m" "Fatal error: %s: not a list!\n" "\x1b[0m", fname);
        exit(EXIT_FAILURE);
    }
}

/* Return list length. */
int lst_len(atom_t* list) {
    return list->val.list->len;
}

/* Return list maximum lenght. */
int lst_maxlen(atom_t* list) {
    return list->val.list->maxlen;
}

/*
--------------------------------------
lst_free

    Deallocate list object, ignore items.
--------------------------------------
*/
void lst_free(atom_t* obj) {
    if (obj) {
        safe_free(obj->val.list->items);  // free items
        safe_free(obj->val.list);         // free list
        safe_free(obj);                   // free object
    }
}
