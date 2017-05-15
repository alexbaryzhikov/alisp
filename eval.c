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
            return strip_quotes(expr);          // quoted string
        
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
            return &nilobj;

        int sym = item[0]->type == SYMBOL;

        // -------------------------------------
        // list             (list [elements...])
        if (sym && streq(item[0]->val.sym, "list")) {

            // TODO: list

            return &nilobj;

        // -------------------------------------
        // cond             (cond (clause expr)... [(else expr)])
        } else if (sym && streq(item[0]->val.sym, "cond")) {
            if (elen < 2) {
                errmsg("Syntax", "poorly formed branching: (cond (clause expr)... [(else expr)])",
                    NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }

            // TODO: cond

            return &nilobj;

        // -------------------------------------
        // if               (if test pro [con])
        } else if (sym && streq(item[0]->val.sym, "if")) {
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
               (test->type == SYMBOL && strlen(test->val.sym) == 0)) {
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
        } else if (sym && streq(item[0]->val.sym, "def")) {
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
        } else if (sym && streq(item[0]->val.sym, "=")) {
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
        } else if (sym && streq(item[0]->val.sym, "null?")) {
            if (elen != 2) {
                errmsg("Syntax", "poorly formed expression: (null? expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            atom_t* v = eval(item[1], env, NULL);
            if (!v)
                return NULL;
            if (v->type == NIL)
                return num_new(1);
            else {
                atom_del(v);
                return num_new(0);
            }
        
        // -------------------------------------
        // func             (func (params) body)
        } else if (sym && streq(item[0]->val.sym, "func")) {
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
            atom_t* body = lst_new();
            lst_add(body, sym_new("block"));
            for (i = 2; i < elen; ++i)
                lst_add(body, item[i]);
            return func_new(item[1], body, env);

        // -------------------------------------
        // block            (block expr [expr ...])
        } else if (sym && streq(item[0]->val.sym, "block")) {
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
        } else if (sym && streq(item[0]->val.sym, "ret")) {
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
            atom_t* args = lst_new();
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
        edict_t* fenv = edict_new(argc, proc->val.closure->env);
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
            s = tostr(argv[i]);
            printf("%s", s);
            safe_free(s);
        }
        return &nilobj;
    
    // -------------------------------------
    // println
    } else if (proc->optype == PRINTLN) {
        char* s;
        for (unsigned i = 0; i < argc; ++i) {
            s = tostr(argv[i]);
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
            return num_new(proc->val.op_math1(*argv[0]->val.num));
        } else {
            *argv[0]->val.num = proc->val.op_math1(*argv[0]->val.num);
            return num_new(*argv[0]->val.num);
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

        return num_new(proc->val.op_math2(*argv[0]->val.num, *argv[1]->val.num));

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
            return num_new(proc->val.op_math2(0, *argv[0]->val.num));

        // Multiple arguments: reduce
        double res = *argv[0]->val.num;
        for (int i = 1; i < argc; ++i)
            res = proc->val.op_math2(res, *argv[i]->val.num);
        return num_new(res);

    // -------------------------------------
    // relation
    } else if (proc->optype == REL) {
        if (argc == 0) {
            errmsg("Syntax", "no arguments", NULL, NULL);
            lst_print(expr, 0);
            return NULL;
        } else if (argc > 2 ) {
            errmsg("Syntax", "too many arguments", NULL, NULL);
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

        // One argument: treat as (arg, 0)
        if (argc == 1)
            return num_new(proc->val.op_math2(*argv[0]->val.num, 0));

        // Two arguments
        return num_new(proc->val.op_math2(*argv[0]->val.num, *argv[1]->val.num));
    }

    printf("\x1b[95m" "Fatal error: unknown operator!\n" "\x1b[0m");
    lst_print(expr, 0);
    exit(EXIT_FAILURE);
}

