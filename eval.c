/*
Evaluate an expression in an environment.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

/*
--------------------------------------
eval

    Evaluate an expression in an environment.
--------------------------------------
*/
atom_t* eval(atom_t* expr, atom_t* env, atom_t** ret) {

#ifdef DEBUG
char* dbg_s = atom_tostr(expr);
printf("....  eval:                    %s\n", dbg_s);
safe_free(dbg_s);
#endif

    active_env = env;

    if (!(expr && env)) {
        printf("\x1b[95m" "Fatal error: eval: bad argument(s)!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    // -------------------------------------
    // Primitive expressions

    if (expr->type == NUMBER)
        return expr;                            // number
    
    if (expr->type == SYMBOL) {
        
        if (expr->val.sym[0] == '"')
            return expr;                        // quoted string
        
        atom_t* v = dict_get(env, expr->val.sym);
        if (v)
            return v;                           // variable

        char* o = atom_tostr(expr);
        errmsg("Semantic", "undefined variable", o, o);
        safe_free(o);
        return NULL;
    }

    // -------------------------------------
    // Compound expressions

    if (expr->type == LIST) {
        atom_t** items = expr->val.list->items;
        int elen = lst_len(expr);

        // -------------------------------------
        // empty list       ()
        if (elen == 0)
            return lst();

        int is_sym = items[0]->type == SYMBOL;

        // -------------------------------------
        // cond             (cond (clause expr)... [(else expr)])
        if (is_sym && streq(items[0]->val.sym, "cond")) {
            if (elen < 2) {
                errmsg("Syntax", "poorly formed branching: (cond (clause expr)... [(else expr)])",
                    NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else {
                for (int i = 1; i < elen; ++i)
                    if (items[i]->type != LIST || lst_len(items[i]) < 2) {
                        char* o = atom_tostr(items[i]);
                        errmsg("Syntax", "poorly formed conditional: (test expr [expr...])", o, o);
                        safe_free(o);
                        return NULL;
                    }
            }

            // Iterate through clauses until test passes or 'else' encountered
            for (int i = 1; i < elen; ++i) {
                int clen = lst_len(items[i]);
                atom_t** clause = items[i]->val.list->items;
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
                    active_env = env;
                    atom_del(body);
                    return v;
                }
                // Evaluate test. If it's true -- evaluate body
                test = eval(test, env, NULL);
                active_env = env;
                if (!test)
                    return NULL;
                if (!(test->type == NIL ||
                     (test->type == NUMBER && *test->val.num == 0) ||
                     (test->type == SYMBOL && strlen(test->val.sym) == 0) ||
                     (test->type == LIST && lst_len(test) == 0))) {
                    atom_del(test);
                    atom_t* v = eval(body, env, ret);
                    active_env = env;
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
        } else if (is_sym && streq(items[0]->val.sym, "if")) {
            if (elen < 3 || elen > 4) {
                errmsg("Syntax", "poorly formed branching: (if test pro [con])", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            atom_t* test = eval(items[1], env, NULL);
            active_env = env;
            if (!test)
                return NULL;
            if (test->type == NIL ||
               (test->type == NUMBER && *test->val.num == 0) ||
               (test->type == SYMBOL && strlen(test->val.sym) == 0) ||
               (test->type == LIST && lst_len(test) == 0)) {
                atom_del(test);
                if (elen == 4)
                    return eval(items[3], env, ret);
                return &nilobj;
            } else {
                atom_del(test);
                return eval(items[2], env, ret);
            }

        // -------------------------------------
        // def              (def var [expr])
        } else if (is_sym && streq(items[0]->val.sym, "def")) {
            if (elen < 2 || elen > 3) {
                errmsg("Syntax", "poorly formed definition: (def var [expr])", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else if (items[1]->type != SYMBOL) {
                errmsg("Semantic", "argument is not a variable name", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else {
                atom_t* e = dict_find(env, items[1]->val.sym);
                if (e && e == env) {
                    errmsg("Semantic", "variable has already been defined", NULL, NULL);
                    lst_print(expr, 0);
                    return NULL;
                }
            }
            // TODO: check for reserved symbols
            if (elen == 2) {
                dict_add(env, items[1]->val.sym, &nilobj);
                return &nilobj;
            } else {
                atom_t* v = eval(items[2], env, NULL);
                active_env = env;
                if (!v)
                    return NULL;
                dict_add(env, items[1]->val.sym, v);
                return v;
            }

        // -------------------------------------
        // =                (= var expr)
        } else if (is_sym && streq(items[0]->val.sym, "=")) {
            if (elen != 3) {
                errmsg("Syntax", "poorly formed assignment: (= var expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else if (items[1]->type != SYMBOL) {
                errmsg("Semantic", "argument is not a variable name", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            // TODO: check for reserved symbols
            atom_t* e = dict_find(env, items[1]->val.sym);
            if (!e) {
                char* o = atom_tostr(items[1]);
                errmsg("Semantic", "undefined variable", o, o);
                safe_free(o);
                return NULL;
            }
            atom_t* v = eval(items[2], env, NULL);
            active_env = env;
            if (!v)
                return NULL;
            dict_add(e, items[1]->val.sym, v);
            return v;
        
        // -------------------------------------
        // null?            (null? expr)
        } else if (is_sym && streq(items[0]->val.sym, "null?")) {
            if (elen != 2) {
                errmsg("Syntax", "poorly formed expression: (null? expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            atom_t* v = eval(items[1], env, NULL);
            active_env = env;
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
        } else if (is_sym && streq(items[0]->val.sym, "func")) {
            if (elen < 3 || items[1]->type != LIST) {
                errmsg("Syntax", "poorly formed function definition: (func ([var ...]) body)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            // Check that all parameters are indeed symbols
            int i;
            for (i = 0; i < lst_len(items[1]); ++i)
                if (items[1]->val.list->items[i]->type != SYMBOL) {
                    errmsg("Semantic", "parameter is not a variable name", NULL, NULL);
                    lst_print(expr, 0);
                    return NULL;
                }
            // Make function body
            atom_t* body = lst();
            lst_add(body, sym("block"));
            for (i = 2; i < elen; ++i)
                lst_add(body, items[i]);
            
            atom_t* v = func(items[1], body, env);
            atom_del(body);
            return v;

        // -------------------------------------
        // block            (block expr [expr ...])
        } else if (is_sym && streq(items[0]->val.sym, "block")) {
            atom_t* block_ret = NULL;
            atom_t* last_v = NULL;
            for (int i = 1; !block_ret && i < elen; ++i) {
                if (last_v)
                    atom_del(last_v);
                if(!(last_v = eval(items[i], env, &block_ret)))
                    return NULL;  // some eval encountered an error
                active_env = env;
            }
            if (!block_ret)
                return last_v;  // value of the last expression in a block (default)
            atom_del(last_v);
            return block_ret;   // value of explicit return statement

        // -------------------------------------
        // ret              (ret expr)
        } else if (is_sym && streq(items[0]->val.sym, "ret")) {
            // Evaluate items[1] and pass it to ret
            if (elen != 2) {
                errmsg("Syntax", "poorly formed return statement: (ret expr)", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            } else if (!ret) {
                errmsg("Semantic", "stray return statement", NULL, NULL);
                lst_print(expr, 0);
                return NULL;
            }
            return *ret = eval(items[1], env, NULL);

        // -------------------------------------
        // apply procedure to arguments         (proc [arg ...])
        } else {

#ifdef DEBUG
printf("....  eval:                    Evaluating procedure and arguments\n");
#endif

            // Evaluate procedure
            atom_t* proc = eval(items[0], env, NULL);
            active_env = env;
            if (!proc)
                return NULL;

            // Evaluate arguments
            atom_t* args = lst();
            atom_t* v;
            for (int i = 1; i < elen; ++i) {
                v = eval(items[i], env, NULL);
                active_env = env;
                if (v) {
                    lst_add(args, v);
                } else {
                    atom_del(proc);
                    atom_del(args);
                    return NULL;
                }
            }

#ifdef DEBUG
dbg_s = atom_tostr(proc);
char* dbg_s2 = atom_tostr(args);
printf("....  eval:                    --> apply %s to %s\n", dbg_s, dbg_s2);
safe_free(dbg_s);
safe_free(dbg_s2);
#endif
            // Apply
            v = apply(expr, proc, args);
            active_env = env;

#ifdef DEBUG
dbg_s = v ? atom_tostr(v) : "NULL";
printf("....  eval:                    %s <-- apply\n", dbg_s);
printf("....  eval:                    Deallocating procedure and arguments\n");
if (v) safe_free(dbg_s);
#endif

            if (v)
                atom_bind(v, env);  // protect returned value
            atom_del(proc);
            atom_del(args);
            if (v)
                atom_unbind(v, env);

            return v;
        }
    }

    char* o = atom_tostr(expr);
    errmsg("Semantic", "unexpected object", o, o);
    safe_free(o);
    return NULL;
}

