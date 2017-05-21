/*
Standard environment.
Routines for standard operators and procedures.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alisp.h>

edict_t* global_env = NULL;

/* Create global environment. */
void globenv_init() {
    if (global_env) {
        printf("\x1b[95m" "Fatal error: global environment initialization failed!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
printf("....  globenv_init:  Creating global environment\n");
#endif

    global_env = edict(32, NULL);

    atom_t* trueobj = num(1);
    atom_t* falseobj = num(0);
    atom_t* eobj = num(M_E);
    atom_t* piobj = num(M_PI);

    /* Constants */
    edict_add(global_env, "NULL", &nilobj);
    edict_add(global_env, "TRUE", trueobj);
    edict_add(global_env, "FALSE", falseobj);
    edict_add(global_env, "E", eobj);
    edict_add(global_env, "PI", piobj);
    /* Print */
    edict_add(global_env, "print", op_print());
    edict_add(global_env, "println", op_println());
    /* Arithmetic */
    edict_add(global_env, "+",    op_math2r(op_add));
    edict_add(global_env, "-",    op_math2r(op_sub));
    edict_add(global_env, "*",    op_math2r(op_mul));
    edict_add(global_env, "/",    op_math2r(op_div));
    edict_add(global_env, "\x25", op_math2r(op_fmod));  // %
    edict_add(global_env, "inc",  op_math1m(op_inc));
    edict_add(global_env, "dec",  op_math1m(op_dec));
    /* Relational */
    edict_add(global_env, "==", op_rel(op_eq));
    edict_add(global_env, "!=", op_rel(op_ne));
    edict_add(global_env, "<",  op_rel(op_lt));
    edict_add(global_env, ">",  op_rel(op_gt));
    edict_add(global_env, "<=", op_rel(op_le));
    edict_add(global_env, ">=", op_rel(op_ge));
    /* Logical */
    edict_add(global_env, "and", op_math2r(op_and));
    edict_add(global_env, "or",  op_math2r(op_or));
    edict_add(global_env, "not", op_math1(op_not));
    /* Bitwise */
    edict_add(global_env, "&",  op_math2r(op_band));
    edict_add(global_env, "|",  op_math2r(op_bor));
    edict_add(global_env, "^",  op_math2r(op_bxor));
    edict_add(global_env, "~",  op_math1(op_bnot));
    edict_add(global_env, "<<", op_math2r(op_blsh));
    edict_add(global_env, ">>", op_math2r(op_brsh));
    /* Math */
    edict_add(global_env, "acos",  op_math1(op_acos));
    edict_add(global_env, "asin",  op_math1(op_asin));
    edict_add(global_env, "atan",  op_math1(op_atan));
    edict_add(global_env, "atan2", op_math2(op_atan2));
    edict_add(global_env, "cos",   op_math1(op_cos));
    edict_add(global_env, "cosh",  op_math1(op_cosh));
    edict_add(global_env, "sin",   op_math1(op_sin));
    edict_add(global_env, "sinh",  op_math1(op_sinh));
    edict_add(global_env, "tanh",  op_math1(op_tanh));
    edict_add(global_env, "exp",   op_math1(op_exp));
    edict_add(global_env, "frexp", op_math1(op_frexp));
    edict_add(global_env, "ldexp", op_math2(op_ldexp));
    edict_add(global_env, "log",   op_math1(op_log));
    edict_add(global_env, "log10", op_math1(op_log10));
    edict_add(global_env, "frac",  op_math1(op_modf));
    edict_add(global_env, "pow",   op_math2(op_pow));
    edict_add(global_env, "sqrt",  op_math1(op_sqrt));
    edict_add(global_env, "ceil",  op_math1(op_ceil));
    edict_add(global_env, "abs",  op_math1(op_fabs));
    edict_add(global_env, "floor", op_math1(op_floor));
    /* Lists */
    edict_add(global_env, "list", op_list());
    edict_add(global_env, "list_get", op_list_get());
    edict_add(global_env, "list_len", op_list_len());
    edict_add(global_env, "list_add", op_list_add());
    edict_add(global_env, "list_ins", op_list_ins());
    edict_add(global_env, "list_del", op_list_del());
    edict_add(global_env, "list_merge", op_list_merge());
}

/* Deallocate global environment. */
void globenv_del() {

#ifdef DEBUG
printf("....  globenv_del:   Deallocating global environment\n");
#endif

    edict_del(global_env);  
}

