/*
Operators.
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alisp.h>


// ---------------------------------------------------------------------- 
// Output

/* Print. */
atom_t* op_print() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = PRINT;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Print line. */
atom_t* op_println() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = PRINTLN;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}


// ---------------------------------------------------------------------- 
// Math and relation

/* Math unary. */
atom_t* op_math1(double (*op)(double)) {
    operator_t* o = malloc(sizeof(operator_t));
    o->val.math1 = op;
    o->type = MATH1;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Math unary, mutates argument. */
atom_t* op_math1m(double (*op)(double)) {
    operator_t* o = malloc(sizeof(operator_t));
    o->val.math1 = op;
    o->type = MATH1_M;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Math binary. */
atom_t* op_math2(double (*op)(double, double)) {
    operator_t* o = malloc(sizeof(operator_t));
    o->val.math2 = op;
    o->type = MATH2;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Math binary, reduces operator over arguments. */
atom_t* op_math2r(double (*op)(double, double)) {
    operator_t* o = malloc(sizeof(operator_t));
    o->val.math2 = op;
    o->type = MATH2_R;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Relation. */
atom_t* op_rel(double (*op)(char, void*, void*)) {
    operator_t* o = malloc(sizeof(operator_t));
    o->val.rel = op;
    o->type = REL;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}


// ---------------------------------------------------------------------- 
// Utility

/* Return a copy of an object. */
atom_t* op_copy() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = COPY;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Return type of an object. */
atom_t* op_type() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = TYPE;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}


// ---------------------------------------------------------------------- 
// List

/* Create list. */
atom_t* op_list() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_NEW;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Get list element/sublist. */
atom_t* op_list_get() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_GET;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Assign a value to list element. */
atom_t* op_list_set() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_SET;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Return list length. */
atom_t* op_list_len() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_LEN;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Add element to list. */
atom_t* op_list_add() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_ADD;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Insert element to list. */
atom_t* op_list_ins() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_INS;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Delete element from list. */
atom_t* op_list_rem() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_REM;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}

/* Merge lists. */
atom_t* op_list_merge() {
    operator_t* o = malloc(sizeof(operator_t));
    o->type = LIST_MERGE;

    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.oper = o;
    obj->type = STD_OP;
    obj->bindings = 0;
    return obj;
}


// ---------------------------------------------------------------------- 
// TODO: dictionary
// TODO: "for" loop -- iterable/generator based
// TODO: "while" loop -- condition based


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
double op_eq(char type, void* a, void* b) {
    return type == NUMBER ? *(double*)a == *(double*)b : strcmp((char*)a, (char*)b) == 0; }

double op_ne(char type, void* a, void* b) {
    return type == NUMBER ? *(double*)a != *(double*)b : strcmp((char*)a, (char*)b) != 0; }

double op_lt(char type, void* a, void* b) {
    return type == NUMBER ? *(double*)a  < *(double*)b : strcmp((char*)a, (char*)b)  < 0; }

double op_gt(char type, void* a, void* b) {
    return type == NUMBER ? *(double*)a  > *(double*)b : strcmp((char*)a, (char*)b)  > 0; }

double op_le(char type, void* a, void* b) {
    return type == NUMBER ? *(double*)a <= *(double*)b : strcmp((char*)a, (char*)b) <= 0; }

double op_ge(char type, void* a, void* b) {
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

