/*
Operators.
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alisp.h>

// ---------------------------------------------------------------------- 
// Operator object constructors

/* Print. */
atom_t* op_print() {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = PRINT;
    a->bindings = 0;
    return a;
}

/* Print line. */
atom_t* op_println() {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = PRINTLN;
    a->bindings = 0;
    return a;
}

/* Math unary. */
atom_t* op_math1(double (*op)(double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math1 = op;
    a->type = STD_OP;
    a->optype = MATH1;
    a->bindings = 0;
    return a;
}

/* Math unary, mutates argument. */
atom_t* op_math1m(double (*op)(double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math1 = op;
    a->type = STD_OP;
    a->optype = MATH1_M;
    a->bindings = 0;
    return a;
}

/* Math binary. */
atom_t* op_math2(double (*op)(double, double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math2 = op;
    a->type = STD_OP;
    a->optype = MATH2;
    a->bindings = 0;
    return a;
}

/* Math binary, reduces operator over arguments. */
atom_t* op_math2r(double (*op)(double, double)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_math2 = op;
    a->type = STD_OP;
    a->optype = MATH2_R;
    a->bindings = 0;
    return a;
}

/* Relation. */
atom_t* op_rel(double (*op)(int, void*, void*)) {
    atom_t* a = (atom_t*)malloc(sizeof(atom_t));
    a->val.op_rel = op;
    a->type = STD_OP;
    a->optype = REL;
    a->bindings = 0;
    return a;
}

/* Create list. */
atom_t* op_list() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_NEW;
    a->bindings = 0;
    return a;
}

/* Get list element/sublist. */
atom_t* op_list_get() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_GET;
    a->bindings = 0;
    return a;
}

/* Return list length. */
atom_t* op_list_len() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_LEN;
    a->bindings = 0;
    return a;
}

/* Add element to list. */
atom_t* op_list_add() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_ADD;
    a->bindings = 0;
    return a;
}

/* Insert element to list. */
atom_t* op_list_ins() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_INS;
    a->bindings = 0;
    return a;
}

/* Delete element from list. */
atom_t* op_list_del() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_DEL;
    a->bindings = 0;
    return a;
}

/* Merge lists. */
atom_t* op_list_merge() {
    atom_t* a = malloc(sizeof(atom_t));
    a->type = STD_OP;
    a->optype = LIST_MERGE;
    a->bindings = 0;
    return a;
}


// ---------------------------------------------------------------------- 
// Operator functions

/* Arithmetic */
double op_add(double a, double b) { return a + b; }
double op_sub(double a, double b) { return a - b; }
double op_mul(double a, double b) { return a * b; }
double op_div(double a, double b) { return a / b; }
double op_inc(double a)           { return a + 1.0; }
double op_dec(double a)           { return a - 1.0; }

/* Relational */
double op_eq(int type, void* a, void* b) {
    return type == NUMBER ? *(double*)a == *(double*)b : strcmp((char*)a, (char*)b) == 0; }

double op_ne(int type, void* a, void* b) {
    return type == NUMBER ? *(double*)a != *(double*)b : strcmp((char*)a, (char*)b) != 0; }

double op_lt(int type, void* a, void* b) {
    return type == NUMBER ? *(double*)a  < *(double*)b : strcmp((char*)a, (char*)b)  < 0; }

double op_gt(int type, void* a, void* b) {
    return type == NUMBER ? *(double*)a  > *(double*)b : strcmp((char*)a, (char*)b)  > 0; }

double op_le(int type, void* a, void* b) {
    return type == NUMBER ? *(double*)a <= *(double*)b : strcmp((char*)a, (char*)b) <= 0; }

double op_ge(int type, void* a, void* b) {
    return type == NUMBER ? *(double*)a >= *(double*)b : strcmp((char*)a, (char*)b) >= 0; }

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

