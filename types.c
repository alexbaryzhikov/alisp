/*
Alisp atomic types.
Methods for creating and manipulating atomic objects.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

atom_t nilobj = {{}, NIL, 0, 1};

// ---------------------------------------------------------------------- 
// Object constructors

/*
--------------------------------------
num

    Make a number.
--------------------------------------
*/
atom_t* num(double x) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.num = (double*)malloc(sizeof(double));
    *a->val.num = x;
    a->type = NUMBER;
    a->bindings = 0;
    return a;
}

/*
--------------------------------------
sym

    Make a symbol.
--------------------------------------
*/
atom_t* sym(const char* s) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.sym = (char*)malloc(strlen(s) + 1);
    strcpy(a->val.sym, s);
    a->type = SYMBOL;
    a->bindings = 0;
    return a;
}

/*
--------------------------------------
func

    Make a function.
--------------------------------------
*/
atom_t* func(atom_t* params, atom_t* body, edict_t* env) {
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
        b = num(*a->val.num);
        return b;
    case  SYMBOL:
        b = sym(a->val.sym);
        return b;
    case  LIST:
        b = lst();
        unsigned i;
        for (i = 0; a->val.lst[i]->type != LIST_EMPTY && a->val.lst[i]->type != EOLIST; ++i)
            lst_add(b, a->val.lst[i]);
        return b;
    default:
        errmsg("Semantic", "copying failed: unrecognized object type", NULL, NULL);
        return NULL;
    }
}

/*
--------------------------------------
type

    Return string with the type of an object.
--------------------------------------
*/
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


