/*
Apply a procedure to arguments.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

/*
--------------------------------------
apply

    Apply a procedure to arguments.
--------------------------------------
*/
atom_t* apply(atom_t* expr, atom_t* proc, atom_t* args) {

    int argc = lst_len(args);
    atom_t** argv = args->val.list->items;
    atom_t* env = active_env;

    // -------------------------------------
    // standard operator
    if (proc->type == STD_OP) {
        return apply_op(expr, proc, argc, argv);

    // -------------------------------------
    // function
    } else if (proc->type == FUNCTION) {
        if (argc != lst_len(proc->val.func->params)) {
            errmsg("Syntax", "wrong number of arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

#ifdef DEBUG
printf("....  apply:                   Creating function environment\n");
#endif

        // Create function environment
        atom_t* fenv = dict(argc, proc->val.func->env);
        atom_t** par = proc->val.func->params->val.list->items;
        for (int i = 0; i < argc; ++i)
            dict_add(fenv, par[i]->val.sym, argv[i]);

#ifdef DEBUG
char* dbg_s = atom_tostr(fenv);
char* dbg_s2 = atom_tostr(proc->val.func->body);
printf("....  apply:                   --> eval %s in %s\n", dbg_s2, dbg_s);
safe_free(dbg_s);
safe_free(dbg_s2);
#endif

        // Evaluate body in the environment
        atom_t* v = eval(proc->val.func->body, fenv, NULL);
        active_env = env;

#ifdef DEBUG
dbg_s = atom_tostr(v);
printf("....  apply:                   %s <-- eval\n", dbg_s);
printf("....  apply:                   Deallocating function environment\n");
safe_free(dbg_s);
#endif

        if (v)
            atom_bind(v, env);  // protect returned value
        atom_del(fenv);
        if (v)
            atom_unbind(v, env);
        return v;
    }

    char* o = atom_tostr(proc);
    errmsg("Semantic", "object is not callable", o, o);
    safe_free(o);
    return NULL;
}


/*
--------------------------------------
apply_op

    Apply a standard operator to arguments.
--------------------------------------
*/
atom_t* apply_op(atom_t* expr, atom_t* proc, int argc, atom_t** argv) {

    operator_t* oper = proc->val.oper;
    char optype = oper->type;

    // -------------------------------------
    // print, println
    if (optype == PRINT || optype == PRINTLN) {
        char* s;
        for (int i = 0; i < argc; ++i) {
            if (argv[i]->type != SYMBOL || argv[i]->val.sym[0] != '"')
                s = atom_tostr(argv[i]);
            else
                s = strip_quotes(argv[i]->val.sym);
            printf("%s", s);
            safe_free(s);
        }
        if (optype == PRINTLN)
            putchar('\n');
        return &nilobj;
    
    // -------------------------------------
    // math unary
    } else if (optype == MATH1 ||     // no mutation
               optype == MATH1_M ) {  // with mutation
        if (argc == 0) {
            errmsg("Syntax", "no arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argc > 1) {
            errmsg("Syntax", "too many arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != NUMBER) {
            errmsg("Semantic", "wrong type of argument", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

        if (optype == MATH1) {
            return num(oper->val.math1(*argv[0]->val.num));
        } else {
            *argv[0]->val.num = oper->val.math1(*argv[0]->val.num);
            return num(*argv[0]->val.num);
        }

    // -------------------------------------
    // math binary, strict
    } else if (optype == MATH2) {
        if (argc != 2) {
            errmsg("Syntax", "wrong number of arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != NUMBER || argv[1]->type != NUMBER) {
            errmsg("Semantic", "wrong type of argument", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

        return num(oper->val.math2(*argv[0]->val.num, *argv[1]->val.num));

    // -------------------------------------
    // math binary, with reduction
    } else if (optype == MATH2_R) {
        if (argc == 0) {
            errmsg("Syntax", "no arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

        int i;
        for (i = 0; i < argc; ++i)
            if (argv[i]->type != NUMBER) {
                errmsg("Semantic", "wrong type of argument", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
        
        // One argument: treat as (0, arg)
        if (argc == 1)
            return num(oper->val.math2(0, *argv[0]->val.num));

        // Multiple arguments: reduce
        double res = *argv[0]->val.num;
        for (int i = 1; i < argc; ++i)
            res = oper->val.math2(res, *argv[i]->val.num);
        return num(res);

    // -------------------------------------
    // relation
    } else if (optype == REL) {
        if (argc != 2 ) {
            errmsg("Syntax", "wrong number of arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if ((argv[0]->type != NUMBER || argv[1]->type != NUMBER) &&
                   (argv[0]->type != SYMBOL || argv[1]->type != SYMBOL)) {
            errmsg("Semantic", "wrong type of argument", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

        if (argv[0]->type == NUMBER)
            return num(oper->val.rel(NUMBER, argv[0]->val.num, argv[1]->val.num));
        else
            return num(oper->val.rel(SYMBOL, argv[0]->val.sym, argv[1]->val.sym));

    // -------------------------------------
    // copy             (copy object)
    } else if (optype == COPY) {
        if (argc != 1) {
            errmsg("Syntax", "wrong number of arguments: (copy object)", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        return atom_copy(argv[0]);

    // -------------------------------------
    // type             (type object)
    } else if (optype == TYPE) {
        if (argc != 1) {
            errmsg("Syntax", "wrong number of arguments: (type object)", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        char* s = add_quotes(atom_type(argv[0]));
        atom_t* v = sym(s);
        safe_free(s);
        return v;

    // -------------------------------------
    // list             (list [items...])
    } else if (optype == LIST_NEW) {
        atom_t* v = lst();
        // Assemble list
        for (int i = 0; i < argc; ++i)
            lst_add(v, argv[i]);
        return v;

    // -------------------------------------
    // list_get         (list_get list index [index2])
    } else if (optype == LIST_GET) {
        if (argc < 2 || argc > 3) {
            errmsg("Syntax", "wrong number of arguments: (list_get list index [index2])",
                NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != LIST) {
            errmsg("Semantic", "not a list", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        atom_t* list = argv[0];
        int llen = lst_len(list);
        // Return single item
        if (argc == 2) {  
            // Evaluate index
            atom_t* index = argv[1];
            if (index->type != NUMBER) {
                errmsg("Semantic", "index is not a number", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            int idx = (int)*index->val.num < 0 ? llen + (int)(*index->val.num) : 
                (int)(*index->val.num);
            if (idx < 0 || idx >= llen) {
                errmsg("Semantic", "index is out of range", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            return list->val.list->items[idx];
        }
        // Return list, that is sublist in range [index, index2)
        // Evaluate index
        atom_t* index = argv[1];
        if (index->type != NUMBER) {
            errmsg("Semantic", "index is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx = (int)*index->val.num < 0 ? llen + (int)(*index->val.num) :
            (int)(*index->val.num);
        if (idx < 0)
            idx = 0;
        // Evaluate index2
        atom_t* index2 = argv[2];
        if (index2->type != NUMBER) {
            errmsg("Semantic", "index2 is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx2 = (int)*index2->val.num < 0 ? llen + (int)(*index2->val.num) :
            (int)(*index2->val.num);
        if (idx2 > llen)
            idx2 = llen;
        // Assemble new list
        atom_t* v = lst();
        for (int i = idx; i < idx2; ++i)
            lst_add(v, list->val.list->items[i]);
        return v;

    // -------------------------------------
    // list_set         (list_set list index item)
    } else if (optype == LIST_SET) {
        if (argc != 3) {
            errmsg("Syntax", "wrong number of arguments: (list_set list index item)", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != LIST) {
            errmsg("Semantic", "not a list", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        atom_t* list = argv[0];
        atom_t* index = argv[1];
        atom_t* item = argv[2];
        if (index->type != NUMBER) {
            errmsg("Semantic", "index is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx = (int)*index->val.num < 0 ? lst_len(list) + (int)(*index->val.num) : 
            (int)(*index->val.num);
        if (idx < 0 || idx >= lst_len(list)) {
            errmsg("Semantic", "index is out of range", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        lst_rem(list, idx);
        lst_ins(list, idx, item);
        return item;

    // -------------------------------------
    // list_len         (list_len list)
    } else if (optype == LIST_LEN) {
        if (argc != 1) {
            errmsg("Syntax", "wrong number of arguments: (list_len list)", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != LIST) {
            errmsg("Semantic", "not a list", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        return num(lst_len(argv[0]));

    // -------------------------------------
    // list_add         (list_add list item [...])
    } else if (optype == LIST_ADD) {
        if (argc < 2) {
            errmsg("Syntax", "too few arguments: (list_add list item [...])", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != LIST) {
            errmsg("Semantic", "not a list", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        for (int i = 1; i < argc; ++i)
            lst_add(argv[0], argv[i]);
        return argv[0];

    // -------------------------------------
    // list_ins         (list_ins list index item)
    } else if (optype == LIST_INS) {
        if (argc != 3) {
            errmsg("Syntax", "wrong number of arguments: (list_ins list index item)", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != LIST) {
            errmsg("Semantic", "not a list", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int llen = lst_len(argv[0]);
        // Evaluate index
        atom_t* index = argv[1];
        if (index->type != NUMBER) {
            errmsg("Semantic", "index is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx = (int)*index->val.num < 0 ? llen + (int)(*index->val.num) : 
            (int)(*index->val.num);
        if (idx < 0)
            idx = 0;
        // Insert item to list
        lst_ins(argv[0], idx, argv[2]);
        return argv[0];

    // -------------------------------------
    // list_rem         (list_rem list index)
    } else if (optype == LIST_REM) {
        if (argc != 2) {
            errmsg("Syntax", "wrong number of arguments: (list_rem list index)", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != LIST) {
            errmsg("Semantic", "not a list", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int llen = lst_len(argv[0]);
        // Evaluate index
        atom_t* index = argv[1];
        if (index->type != NUMBER) {
            errmsg("Semantic", "index is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx = (int)*index->val.num < 0 ? llen + (int)(*index->val.num) : 
            (int)(*index->val.num);
        if (idx < 0 || idx >= llen) {
            errmsg("Semantic", "index is out of range", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        // Delete item from list
        lst_rem(argv[0], idx);
        return argv[0];

    // -------------------------------------
    // list_merge       (list_merge list1 list2 [...])
    } else if (optype == LIST_MERGE) {
        if (argc < 2) {
            errmsg("Syntax", "too few arguments: (list_merge list1 list2 [...])", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int i, j;
        for (i = 0; i < argc; ++i)
            if (argv[i]->type != LIST) {
                errmsg("Semantic", "not a list", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
        // Merge
        atom_t* v = lst();
        for (i = 0; i < argc; ++i)
            for (j = 0; j < lst_len(argv[i]); ++j)
                lst_add(v, argv[i]->val.list->items[j]);
        return v;
    }

    printf("\x1b[95m" "Fatal error: apply_op: unknown operator!\n" "\x1b[0m");
    lst_print(expr, 0);
    exit(EXIT_FAILURE);
}

