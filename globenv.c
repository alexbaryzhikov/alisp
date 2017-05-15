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

    global_env = edict_new(32, NULL);

    atom_t* trueobj = num_new(1);
    atom_t* falseobj = num_new(0);
    atom_t* eobj = num_new(M_E);
    atom_t* piobj = num_new(M_PI);

    /* Constants */
    edict_add(global_env, "NULL", &nilobj);
    edict_add(global_env, "TRUE", trueobj);
    edict_add(global_env, "FALSE", falseobj);
    edict_add(global_env, "E", eobj);
    edict_add(global_env, "PI", piobj);
    /* Print */
    edict_add(global_env, "print", op_print_new());
    edict_add(global_env, "println", op_println_new());
    /* Arithmetic */
    edict_add(global_env, "+",    op_math2r_new(op_add));
    edict_add(global_env, "-",    op_math2r_new(op_sub));
    edict_add(global_env, "*",    op_math2r_new(op_mul));
    edict_add(global_env, "/",    op_math2r_new(op_div));
    edict_add(global_env, "\x25", op_math2r_new(op_fmod));  // %
    edict_add(global_env, "inc",  op_math1m_new(op_inc));
    edict_add(global_env, "dec",  op_math1m_new(op_dec));
    /* Relational */
    edict_add(global_env, "==", op_rel_new(op_eq));
    edict_add(global_env, "!=", op_rel_new(op_ne));
    edict_add(global_env, "<",  op_rel_new(op_lt));
    edict_add(global_env, ">",  op_rel_new(op_gt));
    edict_add(global_env, "<=", op_rel_new(op_le));
    edict_add(global_env, ">=", op_rel_new(op_ge));
    /* Logical */
    edict_add(global_env, "and", op_math2r_new(op_and));
    edict_add(global_env, "or",  op_math2r_new(op_or));
    edict_add(global_env, "not", op_math1_new(op_not));
    /* Bitwise */
    edict_add(global_env, "&",  op_math2r_new(op_band));
    edict_add(global_env, "|",  op_math2r_new(op_bor));
    edict_add(global_env, "^",  op_math2r_new(op_bxor));
    edict_add(global_env, "~",  op_math1_new(op_bnot));
    edict_add(global_env, "<<", op_math2r_new(op_blsh));
    edict_add(global_env, ">>", op_math2r_new(op_brsh));
    /* Math */
    edict_add(global_env, "acos",  op_math1_new(op_acos));
    edict_add(global_env, "asin",  op_math1_new(op_asin));
    edict_add(global_env, "atan",  op_math1_new(op_atan));
    edict_add(global_env, "atan2", op_math2_new(op_atan2));
    edict_add(global_env, "cos",   op_math1_new(op_cos));
    edict_add(global_env, "cosh",  op_math1_new(op_cosh));
    edict_add(global_env, "sin",   op_math1_new(op_sin));
    edict_add(global_env, "sinh",  op_math1_new(op_sinh));
    edict_add(global_env, "tanh",  op_math1_new(op_tanh));
    edict_add(global_env, "exp",   op_math1_new(op_exp));
    edict_add(global_env, "frexp", op_math1_new(op_frexp));
    edict_add(global_env, "ldexp", op_math2_new(op_ldexp));
    edict_add(global_env, "log",   op_math1_new(op_log));
    edict_add(global_env, "log10", op_math1_new(op_log10));
    edict_add(global_env, "frac",  op_math1_new(op_modf));
    edict_add(global_env, "pow",   op_math2_new(op_pow));
    edict_add(global_env, "sqrt",  op_math1_new(op_sqrt));
    edict_add(global_env, "ceil",  op_math1_new(op_ceil));
    edict_add(global_env, "abs",  op_math1_new(op_fabs));
    edict_add(global_env, "floor", op_math1_new(op_floor));
}

/* Deallocate global environment. */
void globenv_del() {

#ifdef DEBUG
printf("....  globenv_del:   Deallocating global environment\n");
#endif

    edict_del(global_env);  
}


// ---------------------------------------------------------------------- 
// Operators

/* Arithmetic */
double op_add(double a, double b) { return a + b; }
double op_sub(double a, double b) { return a - b; }
double op_mul(double a, double b) { return a * b; }
double op_div(double a, double b) { return a / b; }
double op_inc(double a)           { return a + 1.0; }
double op_dec(double a)           { return a - 1.0; }

/* Relational */
double op_eq(double a, double b) { return a == b; }
double op_ne(double a, double b) { return a != b; }
double op_lt(double a, double b) { return a < b; }
double op_gt(double a, double b) { return a > b; }
double op_le(double a, double b) { return a <= b; }
double op_ge(double a, double b) { return a >= b; }

/* Logical */
double op_and(double a, double b) { return a && b; }
double op_or (double a, double b) { return a || b; }
double op_not(double a)           { return !a; }

/* Bitwise */
double op_band(double a, double b) { return (double)((long)a & (long)b); }
double op_bor (double a, double b) { return (double)((long)a | (long)b); }
double op_bxor(double a, double b) { return (double)((long)a ^ (long)b); }
double op_bnot(double a)           { return (double)(~(long)a); }
double op_blsh(double a, double b) { return (double)((long)a << (long)b); }
double op_brsh(double a, double b) { return (double)((long)a >> (long)b); }

/* Math */
double op_acos (double x) { return acos(x); }
double op_asin (double x) { return asin(x); }
double op_atan (double x) { return atan(x); }
double op_atan2(double y, double x) { return atan2(y, x); }
double op_cos  (double x) { return cos(x); }
double op_cosh (double x) { return cosh(x); }
double op_sin  (double x) { return sin(x); }
double op_sinh (double x) { return sinh(x); }
double op_tanh (double x) { return tanh(x); }
double op_exp  (double x) { return exp(x); }
double op_frexp(double x) { int tmp; return frexp(x, &tmp); }
double op_ldexp(double x, double y) { return ldexp(x, (int)y); }
double op_log  (double x) { return log(x); }
double op_log10(double x) { return log10(x); }
double op_modf (double x) { double tmp; return modf(x, &tmp); }
double op_pow  (double x, double y) { return pow(x, y); }
double op_sqrt (double x) { return sqrt(x); }
double op_ceil (double x) { return ceil(x); }
double op_fabs (double x) { return fabs(x); }
double op_floor(double x) { return floor(x); }
double op_fmod (double x, double y) { return fmod(x, y); }

