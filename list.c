/*
List: a collection of objects.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

atom_t list_emptyobj = {{}, LIST_EMPTY, 0, 1};
atom_t eolistobj = {{}, EOLIST, 0, 1};

/*
--------------------------------------
lst
    
    Make a list.
--------------------------------------
*/
atom_t* lst() {
    atom_t* list = (atom_t*)malloc(sizeof(atom_t));
    list->val.lst = (atom_t**)malloc(4 * sizeof(atom_t*));
    unsigned i;
    for (i = 0; i < 3; ++i)
        list->val.lst[i] = &list_emptyobj;  // fill list with LIST_EMPTY objects
    list->val.lst[i] = &eolistobj;          // terminate list with EOLIST object
    list->type = LIST;
    list->bindings = 0;
    return list;
}

/*
--------------------------------------
lst_ins

    Insert item to list at given index.
--------------------------------------
*/
void lst_ins(atom_t* list, int index, atom_t* item) {
    lst_assert(list);
    if (!item) {
        printf("\x1b[95m" "Fatal error: lst_ins: NULL item!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    unsigned sz = lst_len(list);
    if (sz == lst_maxlen(list)) 
        lst_expand(list);
    if (index >= sz || index < 0)
        // insert item at the end of the list
        index = sz;
    else
        // shift items to free space
        memmove(list->val.lst + index + 1, list->val.lst + index, (sz - index + 1) * sizeof(atom_t*));
    list->val.lst[index] = item;
    ++item->bindings;
}

/*
--------------------------------------
lst_del

    Insert item to list at given index.
--------------------------------------
*/
void lst_del(atom_t* list, int index) {
    lst_assert(list);
    unsigned sz = lst_len(list);
    if (index >= sz || index < 0) {
        printf("\x1b[95m" "Fatal error: lst_del: index is out of range!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    atom_t* item = list->val.lst[index];
    --item->bindings;
    atom_del(item);
    memmove(list->val.lst + index, list->val.lst + index + 1, (sz - index - 1) * sizeof(atom_t*));
    list->val.lst[sz-1] = &list_emptyobj;
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
    atom_t** item;
    strcpy(buf, "(");
    for (item = list->val.lst; (*item)->type != LIST_EMPTY && (*item)->type != EOLIST; ++item) {
        if ((*item)->type == LIST) {
            if (strlen(buf) + 10 > 1024) {
                strcat(buf, " ... ");
                break;
            } else if (depth) {
                o = lst_tostr(*item, depth - 1);
                if (strlen(buf) + strlen(o) + 10 > 1024) {
                    strcat(buf, " ... ");
                    safe_free(o);
                    break;
                } else {
                    strcat(buf, o);
                    safe_free(o);
                }
            } else {
                strcat(buf, "(...)");
            }
        } else {
            o = tostr(*item);
            if (strlen(buf) + strlen(o) + 10 > 1024) {
                strcat(buf, " ... ");
                safe_free(o);
                break;
            } else {
                strcat(buf, o);
                safe_free(o);
            }
        }
        if (item[1]->type != LIST_EMPTY && item[1]->type != EOLIST)
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

    Print list macro.
--------------------------------------
*/
void lst_print(atom_t* list, int recur) {
    lst_assert(list);
#ifndef LIST_PRINT_EXPANDED
    lst_pr_lin(list, recur);
    putchar('\n');
#else
    lst_pr_exp(list, 0, recur);
#endif
}

/* Assert that object is of LIST type. */
void lst_assert(atom_t* list) {
    if (list == NULL || list->type != LIST) {
        printf("\x1b[95m" "Fatal error: list assertion failed!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
}

/* Return list length. */
unsigned lst_len(atom_t* list) {
    lst_assert(list);
    unsigned i;
    for (i = 0; list->val.lst[i]->type != LIST_EMPTY && list->val.lst[i]->type != EOLIST; ++i);
    return i;
}

/* Return list maximum lenght. */
unsigned lst_maxlen(atom_t* list) {
    lst_assert(list);
    unsigned i;
    for (i = 0; list->val.lst[i]->type != EOLIST; ++i);
    return i;
}

/* Double the max length of the list. */
void lst_expand(atom_t* list) {
    lst_assert(list);
    unsigned i, mlen;
    i = lst_maxlen(list);
    mlen = (i + 1) << 1;
    list->val.lst = (atom_t**)realloc(list->val.lst, mlen * sizeof(atom_t*));
    for (; i < mlen - 1; ++i)
        list->val.lst[i] = &list_emptyobj;  // fill new chunk with LIST_EMPTY objects
    list->val.lst[i] = &eolistobj;          // terminate list with EOLIST object
}

/* Print list as a continuous string. */
void lst_pr_lin(atom_t* list, int recur) {
    atom_t** item;
    char* o;
    printf("(");
    for (item = list->val.lst; (*item)->type != LIST_EMPTY && (*item)->type != EOLIST; ++item) {
        if ((*item)->type == LIST) {
            if (recur)
                lst_pr_lin(*item, recur);
            else
                printf("(...)");
        } else {
            o = tostr(*item);
            printf("%s", o);
            safe_free(o);
        }
        if (item[1]->type != LIST_EMPTY && item[1]->type != EOLIST)
            putchar(' ');
    }
    printf(")");
}

/* Print list expanded. */
void lst_pr_exp(atom_t* list, int ind, int recur) {
    if (!lst_len(list)) {
        printf("%*c()\n", ind + 1, '\0');
        return;
    }
    atom_t** item;
    char* o;
    printf("%*c(\n", ind + 1, '\0');
    for (item = list->val.lst; (*item)->type != LIST_EMPTY && (*item)->type != EOLIST; ++item) {
        if ((*item)->type == LIST) {
            if (recur)
                lst_pr_exp(*item, ind + 2, recur);
            else
                printf("%*c(...)\n", ind + 3, '\0');
        } else {
            o = tostr(*item);
            printf("%*c%s\n", ind + 3, '\0', o);
            safe_free(o);
        }
    }
    printf("%*c)\n", ind + 1, '\0');
}

