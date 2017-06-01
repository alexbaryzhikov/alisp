/*
Atomic object: basic Alisp data type.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

atom_t nilobj = {{}, NIL, 1};

/*
--------------------------------------
num

    Make a number.
--------------------------------------
*/
atom_t* num(double x) {
    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.num = malloc(sizeof(double));
    *obj->val.num = x;
    obj->type = NUMBER;
    obj->bindings = 0;
    return obj;
}

/*
--------------------------------------
sym

    Make a symbol.
--------------------------------------
*/
atom_t* sym(const char* s) {
    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.sym = malloc(strlen(s) + 1);
    strcpy(obj->val.sym, s);
    obj->type = SYMBOL;
    obj->bindings = 0;
    return obj;
}

/*
--------------------------------------
func

    Make a function.
--------------------------------------
*/
atom_t* func(atom_t* params, atom_t* body, atom_t* env) {
    if (!(params && body && env)) {
        printf("\x1b[95m" "Fatal error: func: bad argument(s)!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    
    // Make a function
    function_t* function = malloc(sizeof(function_t));
    function->bindlist = NULL;
    function->lock = 0;  // mutex to prevent recurrent deallocation
    function->params = atom_copy(params);
    function->body = atom_copy(body);
    function->env = env;

    // Make an object
    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.func = function;
    obj->type = FUNCTION;
    obj->bindings = 0;

    // Bind object to environment
    atom_bind(obj, env);

    // Bind members
    atom_bind(function->params, obj);
    atom_bind(function->body, obj);
    
    // Bind enclosing environments
    for (atom_t* e = env; e && e != global_env; e = e->val.dict->parent)
        atom_bind(e, obj);
    
    return obj;

}

/*
--------------------------------------
func_del

    Deallocate a function.
--------------------------------------
*/
void func_del(atom_t* obj) {
    if (!(obj && obj->type == FUNCTION)) {
        printf("\x1b[95m" "Fatal error: func_del: not a function!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
char* dbg_s = atom_tostr(obj);
printf("....  func_del:                Deallocating %s\n", dbg_s);
safe_free(dbg_s);
#endif

    function_t* f = obj->val.func;
    atom_t *env, *tmp;
    f->lock = 1;  // lock current object

    // Deallocate enclosing environments
    for (env = f->env; env && env != global_env;) {
        atom_unbind(env, obj);
        if (!env->val.dict->lock) {
            tmp = env;
            env = env->val.dict->parent;
            atom_del(tmp);
        } else
            env = env->val.dict->parent;
    }

    // Deallocate the rest
    atom_unbind(f->params, obj);
    atom_del(f->params);
    atom_unbind(f->body, obj);
    atom_del(f->body);
    lst_free(f->bindlist);
    safe_free(f);
    safe_free(obj);
}


/*
--------------------------------------
atom_del

    Deallocate an object.
--------------------------------------
*/
void atom_del(atom_t* a) {
    assert_arg(a, "atom_del");

#ifdef DEBUG
char* dbg_s = atom_tostr(a);
#endif

    // Check if object is bound
    if (a->type == NIL || (a->bindings && (!atom_is_container(a) || atom_bound_in(a, active_env))))
        return;

    if (a->type == NUMBER || a->type == SYMBOL) {
        safe_free(a->val.num);
        safe_free(a);

    } else if (a->type == LIST) {
        lst_del(a);

    } else if (a->type == DICTIONARY) {
        dict_del(a);

    } else if (a->type == FUNCTION) {
        func_del(a);
    
    } else if (a->type == STD_OP) {
        safe_free(a->val.oper);
        safe_free(a);
    }

#ifdef DEBUG
printf("....  atom_del:                %s -- deleted\n", dbg_s);
safe_free(dbg_s);
#endif

}

/*
--------------------------------------
atom_tostr

    Return a string representing an object.
--------------------------------------
*/
char* atom_tostr(atom_t* obj) {
    assert_arg(obj, "atom_tostr");

    char tmp[64];
    char* s;

    switch (obj->type) {
    
    case NIL:
        strcpy(tmp, "<Null object>");
        break;

    case NUMBER:
        sprintf(tmp, "%g", *obj->val.num);
        break;

    case SYMBOL:
        s = malloc(strlen(obj->val.sym) + 1);
        strcpy(s, obj->val.sym);
        return s;

    case LIST:
        return lst_tostr(obj, 1);

    case DICTIONARY:
        return dict_tostr(obj);

    case FUNCTION:
        sprintf(tmp, "<Function at 0x%lx>", (size_t)obj);
        break;

    case STD_OP:
        strcpy(tmp, "<Operator>");
        break;

    default:
        sprintf(tmp, "<Object at 0x%lx>", (size_t)obj);
        break;

    }
    
    s = malloc(strlen(tmp) + 1);
    strcpy(s, tmp);
    return s;
}

/*
--------------------------------------
atom_copy

    Return a deep copy of an object.
--------------------------------------
*/
atom_t* atom_copy(atom_t* obj) {
    assert_arg(obj, "atom_copy");
    atom_t* objects = lst();
    atom_t* copies = lst();
    atom_t* copy = atom_cp(obj, objects, copies);
    lst_free(objects);
    lst_free(copies);
    return copy;
}

/* Recursively copy object. */
atom_t* atom_cp(atom_t* obj, atom_t* objects, atom_t* copies) {
    switch(obj->type) {

    case NIL:
        return &nilobj;

    case NUMBER:
        return num(*obj->val.num);

    case SYMBOL:
        return sym(obj->val.sym);

    case LIST:
        return lst_cp(obj, objects, copies);

    case DICTIONARY:
        return dict_cp(obj, objects, copies);
    
    case FUNCTION:
        return func(obj->val.func->params, obj->val.func->body, obj->val.func->env);

    case STD_OP:
        errmsg("Semantic", "copying protected object", NULL, NULL);
        return NULL;

    default:
        errmsg("Semantic", "copying unrecognized object type", NULL, NULL);
        return NULL;
    }
}


/*
--------------------------------------
atom_type

    Return string with the type of an object.
--------------------------------------
*/
char* atom_type(atom_t* obj) {
    assert_arg(obj, "atom_type");
    switch(obj->type) {
    case  0: return "NIL";
    case  1: return "NUMBER";
    case  2: return "SYMBOL";
    case  3: return "LIST";
    case  4: return "DICTIONARY";
    case  5: return "FUNCTION";
    case  6: return "STD_OP";
    default: return "UNRECOGNIZED";
    }
}

/*
--------------------------------------
atom_bind

    Add a binding to an object.
--------------------------------------
*/
void atom_bind(atom_t* obj, atom_t* container) {
    ++obj->bindings;
    if (atom_is_container(obj)) {
        if (!obj->val.list->bindlist)
            obj->val.list->bindlist = lst();
        lst_add_nobind(obj->val.list->bindlist, container);
    }
}

/*
--------------------------------------
atom_unbind

    Remove a binding from an object.
--------------------------------------
*/
void atom_unbind(atom_t* obj, atom_t* container) {
    --obj->bindings;
    if (atom_is_container(obj) && obj->val.list->bindlist) {
        int idx = lst_idx(obj->val.list->bindlist, container);
        if (idx == -1) {
            printf("\x1b[95m" "Fatal error: unbind: container is not in bind list!\n" "\x1b[0m");
            exit(EXIT_FAILURE);
        }
        lst_rem_nounbind(obj->val.list->bindlist, idx);
    }
}

/*
--------------------------------------
atom_bound_in

    Search if an object is reachable via a chain of enclosed environments.
--------------------------------------
*/
int atom_bound_in(atom_t* obj, atom_t* env) {
    if (!obj || !atom_is_container(obj)) {
        printf("\x1b[95m" "Fatal error: bound_in: bad object!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    } else if (!env || env->type != DICTIONARY) {
        printf("\x1b[95m" "Fatal error: bound_in: bad environment!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    
    atom_t* owners = lst();
    atom_get_owners(obj, owners);
    int bound = 0;
    atom_t* e;
    
    for (int i = 0; !bound && i < owners->val.list->len; ++i)
        for (e = env; e; e = e->val.dict->parent)
            if (owners->val.list->items[i] == e) {
                bound = 1;
                break;
            }
    
    lst_free(owners);
    return bound;
}

/*
--------------------------------------
atom_get_owners

    Create a list of all objects through which a given object can be reached.
--------------------------------------
*/
void atom_get_owners(atom_t* obj, atom_t* ownlist) {
    if (!(obj && atom_is_container(obj))) {
        printf("\x1b[95m" "Fatal error: get_owners: bad object!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    } else if (!(ownlist && ownlist->type == LIST)) {
        printf("\x1b[95m" "Fatal error: get_owners: bad list!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    if (!obj->val.list->bindlist)
        return;
    list_t* bl = obj->val.list->bindlist->val.list;
    atom_t* item;

    for (int i = 0; i < bl->len; ++i) {
        item = bl->items[i];
        // char* s = atom_tostr(item);
        // printf(".... atom_get_owners: %s\n", s);
        // safe_free(s);
        if (lst_idx(ownlist, item) == -1) {
            lst_add_nobind(ownlist, item);   // add item itself
            atom_get_owners(item, ownlist);  // add item owners
        }
    }
}

/*
--------------------------------------
atom_is_container

    Check if an object is of container type.
--------------------------------------
*/
int atom_is_container(atom_t* obj) {
    return obj->type == LIST || obj->type == DICTIONARY || obj->type == FUNCTION;
}

/*
--------------------------------------
assert_arg

    Assert argument is not NULL.
--------------------------------------
*/
void assert_arg(atom_t* obj, const char* fname) {
    if (!obj) {
        printf("\x1b[95m" "Fatal error: %s: bad argument!\n" "\x1b[0m", fname);
        exit(EXIT_FAILURE);
    }
}
