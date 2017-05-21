/*
Evaluation of expressions.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>


// ---------------------------------------------------------------------- 
//
// Eval
//
// ---------------------------------------------------------------------- 

/*
--------------------------------------
eval

    Evaluate an expression in an environment.
--------------------------------------
*/
atom_t* eval(atom_t* expr, edict_t* env, atom_t** ret) {

#ifdef DEBUG
char* se = tostr(expr);
printf("....  eval:          %s\n", se);
safe_free(se);
#endif

    if (!expr || !env) {
        printf("\x1b[95m" "Fatal error: bad eval arguments!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    // -------------------------------------
    // Primitive expressions

    if (expr->type == NUMBER)
        return expr;                            // number
    
    if (expr->type == SYMBOL) {
        
        if (expr->val.sym[0] == '"')
            return expr;                        // quoted string
        
        atom_t* v = edict_get(env, expr->val.sym);
        if (v)
            return v;                           // variable

        char* o = tostr(expr);
        errmsg("Semantic", "undefined variable", o, o);
        safe_free(o);
        return NULL;
    }

    // -------------------------------------
    // Compound expressions

    if (expr->type == LIST) {
        atom_t** item = expr->val.lst;
        unsigned elen = lst_len(expr);

        // -------------------------------------
        // empty list       ()
        if (elen == 0)
            return lst();

        int is_sym = item[0]->type == SYMBOL;

        // -------------------------------------
        // cond             (cond (clause expr)... [(else expr)])
        if (is_sym && streq(item[0]->val.sym, "cond")) {
            if (elen < 2) {
                errmsg("Syntax", "poorly formed branching: (cond (clause expr)... [(else expr)])",
                    NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else {
                for (int i = 1; i < elen; ++i)
                    if (item[i]->type != LIST || lst_len(item[i]) < 2) {
                        char* o = tostr(item[i]);
                        errmsg("Syntax", "poorly formed conditional: (test expr [expr...])", o, o);
                        safe_free(o);
                        return NULL;
                    }
            }

            // Iterate through clauses until test passes or 'else' encountered
            for (int i = 1; i < elen; ++i) {
                int clen = lst_len(item[i]);
                atom_t** clause = item[i]->val.lst;
                atom_t* test = clause[0];
                // Assemble clause body
                atom_t* body;
                if (clen > 2) {     // provide a block for clause body
                    body = lst();
                    lst_add(body, sym("block"));
                    for (int j = 1; j < clen; ++j)
                        lst_add(body, clause[j]);
                } else              // clause body is a single expression
                    body = clause[1];
                // Check if 'else' is encountered
                if (test->type == SYMBOL && streq(test->val.sym, "else")) {
                    atom_t* v = eval(body, env, ret);
                    atom_del(body);
                    return v;
                }
                // Evaluate test. If it's true -- evaluate body
                test = eval(test, env, NULL);
                if (!test)
                    return NULL;
                if (!(test->type == NIL ||
                     (test->type == NUMBER && *test->val.num == 0) ||
                     (test->type == SYMBOL && strlen(test->val.sym) == 0) ||
                     (test->type == LIST && lst_len(test) == 0))) {
                    atom_del(test);
                    atom_t* v = eval(body, env, ret);
                    atom_del(body);
                    return v;
                } else {
                    atom_del(test);
                    atom_del(body);
                }
            }

            return &nilobj;  // all clauses failed

        // -------------------------------------
        // if               (if test pro [con])
        } else if (is_sym && streq(item[0]->val.sym, "if")) {
            if (elen < 3 || elen > 4) {
                errmsg("Syntax", "poorly formed branching: (if test pro [con])", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            atom_t* test = eval(item[1], env, NULL);
            if (!test)
                return NULL;
            if (test->type == NIL ||
               (test->type == NUMBER && *test->val.num == 0) ||
               (test->type == SYMBOL && strlen(test->val.sym) == 0) ||
               (test->type == LIST && lst_len(test) == 0)) {
                atom_del(test);
                if (elen == 4)
                    return eval(item[3], env, ret);
                return &nilobj;
            } else {
                atom_del(test);
                return eval(item[2], env, ret);
            }

        // -------------------------------------
        // def              (def var [expr])
        } else if (is_sym && streq(item[0]->val.sym, "def")) {
            if (elen < 2 || elen > 3) {
                errmsg("Syntax", "poorly formed definition: (def var [expr])", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else if (item[1]->type != SYMBOL) {
                errmsg("Semantic", "argument is not a variable name", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else {
                edict_t* e = edict_find(env, item[1]->val.sym);
                if (e && e == env) {
                    errmsg("Semantic", "variable has already been defined", NULL, NULL);
                    lst_print(expr, 0);
                    return NULL;
                }
            }
            // TODO: check for reserved symbols
            if (elen == 2) {
                edict_add(env, item[1]->val.sym, &nilobj);
                return &nilobj;
            } else {
                atom_t* v = eval(item[2], env, NULL);
                if (!v)
                    return NULL;
                edict_add(env, item[1]->val.sym, v);
                return v;
            }

        // -------------------------------------
        // =                (= var expr)
        } else if (is_sym && streq(item[0]->val.sym, "=")) {
            if (elen != 3) {
                errmsg("Syntax", "poorly formed assignment: (= var expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else if (item[1]->type != SYMBOL) {
                errmsg("Semantic", "argument is not a variable name", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            // TODO: check for reserved symbols
            edict_t* e = edict_find(env, item[1]->val.sym);
            if (!e) {
                char* o = tostr(item[1]);
                errmsg("Semantic", "undefined variable", o, o);
                safe_free(o);
                return NULL;
            }
            atom_t* v = eval(item[2], env, NULL);
            if (!v)
                return NULL;
            edict_add(e, item[1]->val.sym, v);
            return v;
        
        // -------------------------------------
        // null?            (null? expr)
        } else if (is_sym && streq(item[0]->val.sym, "null?")) {
            if (elen != 2) {
                errmsg("Syntax", "poorly formed expression: (null? expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            atom_t* v = eval(item[1], env, NULL);
            if (!v)
                return NULL;
            if (v->type == NIL)
                return num(1);
            else {
                atom_del(v);
                return num(0);
            }
        
        // -------------------------------------
        // func             (func (params) body)
        } else if (is_sym && streq(item[0]->val.sym, "func")) {
            if (elen < 3 || item[1]->type != LIST) {
                errmsg("Syntax", "poorly formed function definition: (func ([var ...]) body)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            // Check that all parameters are indeed symbols
            int i;
            for (i = 0; i < lst_len(item[1]); ++i)
                if (item[1]->val.lst[i]->type != SYMBOL) {
                    errmsg("Semantic", "parameter is not a variable name", NULL, NULL);
                    lst_print(expr, 0);
                    return NULL;
                }
            // Make a function body list
            atom_t* body = lst();
            lst_add(body, sym("block"));
            for (i = 2; i < elen; ++i)
                lst_add(body, item[i]);
            return func(item[1], body, env);

        // -------------------------------------
        // block            (block expr [expr ...])
        } else if (is_sym && streq(item[0]->val.sym, "block")) {
            atom_t* block_ret = NULL;
            atom_t* last_v = NULL;
            while (!block_ret && (*(++item))->type != LIST_EMPTY && (*item)->type != EOLIST) {
                atom_del(last_v);
                if(!(last_v = eval(*item, env, &block_ret)))
                    return NULL;  // some eval encountered an error
            }
            if (!block_ret)
                return last_v;  // value of the last expression in a block (default)
            atom_del(last_v);
            return block_ret;   // value of explicit return statement

        // -------------------------------------
        // ret              (ret expr)
        } else if (is_sym && streq(item[0]->val.sym, "ret")) {
            // Evaluate item[1] and pass it to ret
            if (elen != 2) {
                errmsg("Syntax", "poorly formed return statement: (ret expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else if (!ret) {
                errmsg("Semantic", "stray return statement", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            return *ret = eval(item[1], env, NULL);

        // -------------------------------------
        // apply procedure to arguments         (proc [arg ...])
        } else {

#ifdef DEBUG
printf("....  eval:          Evaluating procedure and arguments\n");
#endif

            // Evaluate procedure
            atom_t* proc = eval(item[0], env, NULL);
            if (!proc)
                return NULL;

            // Evaluate arguments
            atom_t* args = lst();
            atom_t* v;
            if (elen > 1) {
                for (unsigned i = 1; i < elen; ++i)
                    if ((v = eval(item[i], env, NULL))) {
                        lst_add(args, v);
                    } else {
                        atom_del(proc);
                        atom_del(args);
                        return NULL;
                    }
            }

#ifdef DEBUG
se = tostr(proc);
char* sa = tostr(args);
printf("....  eval:          --> apply %s to %s\n", se, sa);
safe_free(se);
safe_free(sa);
#endif
            // Apply
            v = apply(expr, proc, args);

#ifdef DEBUG
se = tostr(v);
printf("....  eval:          %s <-- apply\n", se);
printf("....  eval:          Deallocating procedure and arguments\n");
safe_free(se);
#endif

            if (v) ++v->bindings;  // protect returned value
            atom_del(proc);
            atom_del(args);
            if (v) --v->bindings;
            return v;
        }
    }

    char* o = tostr(expr);
    errmsg("Semantic", "unexpected object", o, o);
    safe_free(o);
    return NULL;
}


// ---------------------------------------------------------------------- 
//
// Apply
//
// ---------------------------------------------------------------------- 

/*
--------------------------------------
apply

    Apply procedure to arguments.
--------------------------------------
*/
atom_t* apply(atom_t* expr, atom_t* proc, atom_t* args) {

    int argc = lst_len(args);
    atom_t** argv = args->val.lst;

    // -------------------------------------
    // standard operator
    if (proc->type == STD_OP) {
        return apply_op(expr, proc, argc, argv);

    // -------------------------------------
    // function
    } else if (proc->type == FUNCTION) {
        if (argc != lst_len(proc->val.closure->params)) {
            errmsg("Syntax", "wrong number of arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

#ifdef DEBUG
printf("....  apply:         Creating function environment\n");
#endif

        // Create function environment
        edict_t* fenv = edict(argc, proc->val.closure->env);
        atom_t** par = proc->val.closure->params->val.lst;
        for (int i = 0; i < argc; ++i)
            edict_add(fenv, par[i]->val.sym, argv[i]);

#ifdef DEBUG
char* se = edict_tostr(fenv);
char* sa = tostr(proc->val.closure->body);
printf("....  apply:         --> eval %s in %s\n", sa, se);
safe_free(se);
safe_free(sa);
#endif

        // Evaluate body in the environment
        atom_t* v = eval(proc->val.closure->body, fenv, NULL);
        if (v) edict_add(fenv, "_ret", v);  // holds value returned by the function

#ifdef DEBUG
se = tostr(v);
printf("....  apply:         %s <-- eval\n", se);
printf("....  apply:         Deallocating function environment\n");
safe_free(se);
#endif

        if (v) ++v->bindings;   // protect returned value
        edict_del(fenv);        // either delete or make a ghost
        if (v) --v->bindings;
        return v;
    }

    char* o = tostr(proc);
    errmsg("Semantic", "object is not callable", o, o);
    safe_free(o);
    return NULL;
}


/*
--------------------------------------
apply_op

    Apply standard operator to arguments.
--------------------------------------
*/
atom_t* apply_op(atom_t* expr, atom_t* proc, int argc, atom_t** argv) {
    // -------------------------------------
    // print
    if (proc->optype == PRINT) {
        char* s;
        for (unsigned i = 0; i < argc; ++i) {
            if (argv[i]->type != SYMBOL || argv[i]->val.sym[0] != '"')
                s = tostr(argv[i]);
            else
                s = strip_quotes(argv[i]->val.sym);
            printf("%s", s);
            safe_free(s);
        }
        return &nilobj;
    
    // -------------------------------------
    // println
    } else if (proc->optype == PRINTLN) {
        char* s;
        for (unsigned i = 0; i < argc; ++i) {
            if (argv[i]->type != SYMBOL || argv[i]->val.sym[0] != '"')
                s = tostr(argv[i]);
            else
                s = strip_quotes(argv[i]->val.sym);
            printf("%s", s);
            safe_free(s);
        }
        putchar('\n');
        return &nilobj;
    
    // -------------------------------------
    // math unary
    } else if (proc->optype == MATH1 ||     // no mutation
               proc->optype == MATH1_M ) {  // with mutation
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

        if (proc->optype == MATH1) {
            return num(proc->val.op_math1(*argv[0]->val.num));
        } else {
            *argv[0]->val.num = proc->val.op_math1(*argv[0]->val.num);
            return num(*argv[0]->val.num);
        }

    // -------------------------------------
    // math binary, strict
    } else if (proc->optype == MATH2) {
        if (argc != 2) {
            errmsg("Syntax", "wrong number of arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argv[0]->type != NUMBER || argv[1]->type != NUMBER) {
            errmsg("Semantic", "wrong type of argument", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }

        return num(proc->val.op_math2(*argv[0]->val.num, *argv[1]->val.num));

    // -------------------------------------
    // math binary, with reduction
    } else if (proc->optype == MATH2_R) {
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
            return num(proc->val.op_math2(0, *argv[0]->val.num));

        // Multiple arguments: reduce
        double res = *argv[0]->val.num;
        for (int i = 1; i < argc; ++i)
            res = proc->val.op_math2(res, *argv[i]->val.num);
        return num(res);

    // -------------------------------------
    // relation
    } else if (proc->optype == REL) {
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
            return num(proc->val.op_rel(NUMBER, argv[0]->val.num, argv[1]->val.num));
        else
            return num(proc->val.op_rel(SYMBOL, argv[0]->val.sym, argv[1]->val.sym));

    // -------------------------------------
    // list             (list [elements...])
    } else if (proc->optype == LIST_NEW) {
        atom_t* v = lst();
        // Assemble list
        for (int i = 0; i < argc; ++i)
            lst_add(v, argv[i]);
        return v;

    // -------------------------------------
    // list_get         (list_get list index [index2])
    } else if (proc->optype == LIST_GET) {
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
        // Return single element
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
            return list->val.lst[idx];
        }
        // Return list, that is sublist in range [index, index2)
        // Evaluate index
        atom_t* index = argv[1];
        if (index->type != NUMBER) {
            errmsg("Semantic", "index is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx  = (int)*index->val.num  < 0 ? llen + (int)(*index->val.num) : 
            (int)(*index->val.num);
        // Evaluate index2
        atom_t* index2 = argv[2];
        if (index2->type != NUMBER) {
            errmsg("Semantic", "index2 is not a number", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        }
        int idx2 = (int)*index2->val.num < 0 ? llen + (int)(*index2->val.num) :
            (int)(*index2->val.num);
        // Assemble new list
        atom_t* v = lst();
        for (int i = idx; i < idx2; ++i)
            if (i >= 0 && i < llen)
                lst_add(v, list->val.lst[i]);
        return v;

    // -------------------------------------
    // list_len         (list_len list)
    } else if (proc->optype == LIST_LEN) {
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
    // list_add         (list_add list element [...])
    } else if (proc->optype == LIST_ADD) {
        if (argc < 2) {
            errmsg("Syntax", "too few arguments: (list_add list element [...])", NULL, NULL);
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
    // list_ins         (list_ins list index element)
    } else if (proc->optype == LIST_INS) {
        if (argc != 3) {
            errmsg("Syntax", "wrong number of arguments: (list_ins list index element)", NULL, NULL);
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
        // Insert element to list
        lst_ins(argv[0], idx, argv[2]);
        return argv[0];

    // -------------------------------------
    // list_del         (list_del list index)
    } else if (proc->optype == LIST_DEL) {
        if (argc != 2) {
            errmsg("Syntax", "wrong number of arguments: (list_del list index)", NULL, NULL);
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
        // Delete element from list
        lst_del(argv[0], idx);
        return argv[0];

    // -------------------------------------
    // list_merge       (list_merge list1 list2 [...])
    } else if (proc->optype == LIST_MERGE) {
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
                lst_add(v, argv[i]->val.lst[j]);
        return v;
    }

    printf("\x1b[95m" "Fatal error: unknown operator!\n" "\x1b[0m");
    lst_print(expr, 0);
    exit(EXIT_FAILURE);
}

