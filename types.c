/*
Alisp atomic types.
Methods for creating and manipulating atomic objects.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

/* Standard objects. */
atom_t nilobj = {{}, NIL, 0, 1};
atom_t list_emptyobj = {{}, LIST_EMPTY, 0, 1};
atom_t eolistobj = {{}, EOLIST, 0, 1};

// ---------------------------------------------------------------------- 
// Primitive data

/* Make a number. */
atom_t* num_new(double x) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.num = (double*)malloc(sizeof(double));
    *a->val.num = x;
    a->type = NUMBER;
    a->bindings = 0;
    return a;
}

/* Make a symbol. */
atom_t* sym_new(const char* s) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.sym = (char*)malloc(strlen(s) + 1);
    strcpy(a->val.sym, s);
    a->type = SYMBOL;
    a->bindings = 0;
    return a;
}


// ---------------------------------------------------------------------- 
// Operators

/* Print. */
atom_t* op_print_new() {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = PRINT;
    a->bindings = 0;
    return a;
}

/* Print line. */
atom_t* op_println_new() {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = PRINTLN;
    a->bindings = 0;
    return a;
}

/* Math unary. */
atom_t* op_math1_new(double (*op)(double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math1 = op;
    a->type = STD_OP;
    a->optype = MATH1;
    a->bindings = 0;
    return a;
}

/* Math unary, mutates argument. */
atom_t* op_math1m_new(double (*op)(double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math1 = op;
    a->type = STD_OP;
    a->optype = MATH1_M;
    a->bindings = 0;
    return a;
}

/* Math binary. */
atom_t* op_math2_new(double (*op)(double, double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math2 = op;
    a->type = STD_OP;
    a->optype = MATH2;
    a->bindings = 0;
    return a;
}

/* Math binary, reduces operator over arguments. */
atom_t* op_math2r_new(double (*op)(double, double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math2 = op;
    a->type = STD_OP;
    a->optype = MATH2_R;
    a->bindings = 0;
    return a;
}

/* Relation. */
atom_t* op_rel_new(double (*op)(double, double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math2 = op;
    a->type = STD_OP;
    a->optype = REL;
    a->bindings = 0;
    return a;
}


// ---------------------------------------------------------------------- 
// Functions

/* Make a function. */
atom_t* func_new(atom_t* params, atom_t* body, edict_t* env) {
    if (params == NULL || body == NULL || env == NULL) {
        printf("\x1b[95m" "Fatal error: function creation failed!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    
    // Make a closure
    closure_t* closure = malloc(sizeof(closure_t));
    closure->params = params;
    closure->body = body;
    closure->env = env;
    closure->lock = 0;  // mutex to prevent recurrent deallocation
    ++params->bindings;
    ++body->bindings;
    
    // Make a function
    atom_t* function = malloc(sizeof(atom_t));
    function->val.closure = closure;
    function->type = FUNCTION;
    function->bindings = 0;
    
    // Tether every environment up the chain, starting from parent
    for (edict_t* penv = env->parent; penv && penv != global_env; penv = penv->parent)
        ++penv->tethers;

    return function;
}


// ---------------------------------------------------------------------- 
// Lists

/*
--------------------------------------
lst_new
    
    Make a list.
--------------------------------------
*/
atom_t* lst_new() {
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
lst_add

    Add item to list.
--------------------------------------
*/
void lst_add(atom_t* list, atom_t* item) {
    lst_assert(list);
    if (!item) {
        printf("\x1b[95m" "Fatal error: lst_add: NULL item!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    unsigned sz = lst_len(list);
    if (sz == lst_maxlen(list)) 
        lst_expand(list);
    list->val.lst[sz] = item;
    ++item->bindings;
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


// ---------------------------------------------------------------------- 
// Common object methods

/*
--------------------------------------
safe_atom_del

    Deallocate object.
--------------------------------------
*/
void safe_atom_del(void** p_addr) {
    if (p_addr == NULL || *p_addr == NULL)
        return;

    atom_t* a = *((atom_t**)(p_addr));
    if (a->type == NIL || a->type == LIST_EMPTY || a->type == EOLIST)
        return;

    if (a->bindings)
        return;  // object is bound

#ifdef DEBUG
char* se = tostr(a);
#endif

    // LIST
    if (a->type == LIST) {
        for (atom_t** item = a->val.lst;; ++item) {
            if ((*item)->type != LIST_EMPTY && (*item)->type != EOLIST) {
                --(*item)->bindings;
                atom_del(*item);
            }
            else
                break;
        }
        safe_free(a->val.lst);
    // NUMBER & SYMBOL
    } else if (a->type == NUMBER || a->type == SYMBOL) {
        safe_free(a->val.num);
    // FUNCTION
    } else if (a->type == FUNCTION) {

#ifdef DEBUG
printf("....  atom_del:      Deallocating %s\n", se);
#endif
        a->val.closure->lock = 1;  // lock current function

        // Deallocate tethered ghosts
        edict_t* tmp;
        for (edict_t* penv = a->val.closure->env->parent; penv && penv != global_env;) {
            --penv->tethers;
            if (penv->ghost && !penv->lock) {
                tmp = penv;
                penv = penv->parent;
                edict_del(tmp);  // banish ghosts
            } else
                penv = penv->parent;
        }

        // If function's home environment is a ghost -- banish it
        tmp = a->val.closure->env;
        if (tmp -> ghost && !tmp->lock && tmp != global_env)
            edict_del(tmp);

        // Deallocate closure
        --a->val.closure->params->bindings;
        --a->val.closure->body->bindings;
        atom_del(a->val.closure->params);
        atom_del(a->val.closure->body);
        safe_free(a->val.closure);
    }

    // Deallocate object
    safe_memory_free(p_addr);

#ifdef DEBUG
printf("....  atom_del:      %s -- deleted\n", se);
safe_free(se);
#endif

}

/*
--------------------------------------
tostr

    Return a string representing an object.
--------------------------------------
*/
char* tostr(atom_t* a) {
    if (!a)
        return NULL;

    if (a->type == SYMBOL) {
        char* s = (char*)malloc(strlen(a->val.sym) + 1);
        strcpy(s, a->val.sym);
        return s;
    }

    if (a->type == LIST)
        return lst_tostr(a, 1);

    char tmp[64];

    if (a->type == NUMBER)
        sprintf(tmp, "%g", *a->val.num);
    else if (a->type == NIL)
        strcpy(tmp, "<Null object>");
    else if (a->type == LIST_EMPTY)
        strcpy(tmp, "<List-empty object>");
    else if (a->type == EOLIST)
        strcpy(tmp, "<End-of-list object>");
    else if (a->type == STD_OP)
        strcpy(tmp, "<Operator>");
    else if (a->type == FUNCTION)
        sprintf(tmp, "<Function at 0x%lx>", (size_t)(a));
    else
        sprintf(tmp, "<Object at 0x%lx>", (size_t)(a));

    char* s = (char*)malloc(strlen(tmp) + 1);
    strcpy(s, tmp);
    return s;
}

/*
--------------------------------------
atom_copy

    Return a copy of an object. Shallow copy in case of a list.
--------------------------------------
*/
atom_t* atom_copy(atom_t* a) {
    if (!a)
        return NULL;
    else if (a->type == STD_OP || a->type == FUNCTION) {
        char* o = tostr(a);
        errmsg("Semantic", "copying failed: protected object", o, o);
        safe_free(o);
        return NULL;
    }

    if (a->type == NIL)        return &nilobj;
    if (a->type == LIST_EMPTY) return &list_emptyobj;
    if (a->type == EOLIST)     return &eolistobj;

    // Copy number, symbol or list
    atom_t* b;
    switch(a->type) {
    case  NUMBER:
        b = num_new(*a->val.num);
        return b;
    case  SYMBOL:
        b = sym_new(a->val.sym);
        return b;
    case  LIST:
        b = lst_new();
        unsigned i;
        for (i = 0; a->val.lst[i]->type != LIST_EMPTY && a->val.lst[i]->type != EOLIST; ++i)
            lst_add(b, a->val.lst[i]);
        return b;
    default:
        errmsg("Semantic", "copying failed: unrecognized object type", NULL, NULL);
        return NULL;
    }
}

/* Return string with the type of an object. */
char* type(atom_t* a) {
    switch(a->type) {
    case  0: return "NIL";
    case  1: return "NUMBER";
    case  2: return "SYMBOL";
    case  3: return "LIST";
    case  4: return "LIST_EMPTY";
    case  5: return "EOLIST";
    case  6: return "STD_OP";
    case  7: return "FUNCTION";
    default: return "UNRECOGNIZED";
    }
}


