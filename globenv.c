/*
Standard environment.
Routines for standard operators and procedures.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "alisp.h"

atom_t* global_env;
atom_t* active_env;

/* Create global environment. */
void globenv_init() {
    if (global_env) {
        printf("\x1b[95m" "Fatal error: globenv_init: global environment already exists!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
printf("....  globenv_init:            Creating global environment\n");
#endif

    global_env = dict(32, NULL);

    atom_t* trueobj = num(1);
    atom_t* falseobj = num(0);
    atom_t* eobj = num(M_E);
    atom_t* piobj = num(M_PI);

    /* Constants */
    dict_add(global_env, "NULL",  &nilobj);
    dict_add(global_env, "TRUE",  trueobj);
    dict_add(global_env, "FALSE", falseobj);
    dict_add(global_env, "E",     eobj);
    dict_add(global_env, "PI",    piobj);
    /* Output */
    dict_add(global_env, "print",   op_print());
    dict_add(global_env, "println", op_println());
    /* Arithmetic */
    dict_add(global_env, "+",    op_math2r(op_add));
    dict_add(global_env, "-",    op_math2r(op_sub));
    dict_add(global_env, "*",    op_math2r(op_mul));
    dict_add(global_env, "/",    op_math2r(op_div));
    dict_add(global_env, "\x25", op_math2r(op_fmod));  // %
    dict_add(global_env, "inc",  op_math1m(op_inc));
    dict_add(global_env, "dec",  op_math1m(op_dec));
    /* Relational */
    dict_add(global_env, "==", op_rel(op_eq));
    dict_add(global_env, "!=", op_rel(op_ne));
    dict_add(global_env, "<",  op_rel(op_lt));
    dict_add(global_env, ">",  op_rel(op_gt));
    dict_add(global_env, "<=", op_rel(op_le));
    dict_add(global_env, ">=", op_rel(op_ge));
    /* Logical */
    dict_add(global_env, "and", op_math2r(op_and));
    dict_add(global_env, "or",  op_math2r(op_or));
    dict_add(global_env, "not", op_math1(op_not));
    /* Bitwise */
    dict_add(global_env, "&",  op_math2r(op_band));
    dict_add(global_env, "|",  op_math2r(op_bor));
    dict_add(global_env, "^",  op_math2r(op_bxor));
    dict_add(global_env, "~",  op_math1(op_bnot));
    dict_add(global_env, "<<", op_math2r(op_blsh));
    dict_add(global_env, ">>", op_math2r(op_brsh));
    /* Math */
    dict_add(global_env, "acos",  op_math1(op_acos));
    dict_add(global_env, "asin",  op_math1(op_asin));
    dict_add(global_env, "atan",  op_math1(op_atan));
    dict_add(global_env, "atan2", op_math2(op_atan2));
    dict_add(global_env, "cos",   op_math1(op_cos));
    dict_add(global_env, "cosh",  op_math1(op_cosh));
    dict_add(global_env, "sin",   op_math1(op_sin));
    dict_add(global_env, "sinh",  op_math1(op_sinh));
    dict_add(global_env, "tanh",  op_math1(op_tanh));
    dict_add(global_env, "exp",   op_math1(op_exp));
    dict_add(global_env, "frexp", op_math1(op_frexp));
    dict_add(global_env, "ldexp", op_math2(op_ldexp));
    dict_add(global_env, "log",   op_math1(op_log));
    dict_add(global_env, "log10", op_math1(op_log10));
    dict_add(global_env, "frac",  op_math1(op_modf));
    dict_add(global_env, "pow",   op_math2(op_pow));
    dict_add(global_env, "sqrt",  op_math1(op_sqrt));
    dict_add(global_env, "ceil",  op_math1(op_ceil));
    dict_add(global_env, "abs",   op_math1(op_fabs));
    dict_add(global_env, "floor", op_math1(op_floor));
    /* Type */
    dict_add(global_env, "copy", op_copy());
    dict_add(global_env, "type", op_type());
    /* Lists */
    dict_add(global_env, "list",       op_list());
    dict_add(global_env, "list_get",   op_list_get());
    dict_add(global_env, "list_set",   op_list_set());
    dict_add(global_env, "list_len",   op_list_len());
    dict_add(global_env, "list_add",   op_list_add());
    dict_add(global_env, "list_ins",   op_list_ins());
    dict_add(global_env, "list_rem",   op_list_rem());
    dict_add(global_env, "list_merge", op_list_merge());
}

/* Deallocate global environment. */
void globenv_del() {
    if (!global_env) {
        printf("\x1b[95m" "Fatal error: globenv_del: global environment doesn't exist!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
printf("....  globenv_del:             Deallocating global environment\n");
#endif

    atom_del(global_env);
}

