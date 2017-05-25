#ifndef ALISP_H
#define ALISP_H

// #define DEBUG

// ---------------------------------------------------------------------- 
// atom.c 

/* Types of atomic objects */
enum { NIL, NUMBER, SYMBOL, LIST, DICTIONARY, FUNCTION, STD_OP };

/* Standard operator types */
enum { PRINT, PRINTLN, MATH1, MATH1_M, MATH2, MATH2_R, REL, COPY, TYPE,
       LIST_NEW, LIST_GET, LIST_SET, LIST_LEN, LIST_ADD, LIST_INS, LIST_REM, LIST_MERGE };

typedef struct Atom atom_t;

/* List */
typedef struct List {
    atom_t*  bindlist;
    char     lock;
    int      len;
    int      maxlen;
    atom_t** items;
} list_t;

/* Dictionary */
typedef struct Dictionary {
    atom_t*  bindlist;
    char     lock;
    int      len;
    int      maxlen;
    char**   keys;
    atom_t** vals;
    atom_t*  parent;
} dict_t;

/* Function */
typedef struct Function {
    atom_t* bindlist;
    char    lock;
    atom_t* params;
    atom_t* body;
    atom_t* env;
} function_t;

/* Operator */
typedef struct Operator {
    union {
        double (*math1)(double);
        double (*math2)(double, double);
        double (*rel)(char, void*, void*);
    } val;
    char type;
} operator_t;

/* Atomic object */
typedef struct Atom {
    union {
        double*     num;
        char*       sym;
        list_t*     list;
        dict_t*     dict;
        function_t* func;
        operator_t* oper;
    } val;
    char     type;
    unsigned bindings;
} atom_t;

extern atom_t nilobj;

atom_t* num(double);
atom_t* sym(const char*);
atom_t* func(atom_t*, atom_t*, atom_t*);
void    atom_del(atom_t*);
char*   atom_tostr(atom_t*);
atom_t* atom_copy(atom_t*);
atom_t* atom_cp(atom_t*, atom_t*, atom_t*);
char*   atom_type(atom_t*);
void    atom_bind(atom_t*, atom_t*);
void    atom_unbind(atom_t*, atom_t*);
int     atom_bound_in(atom_t*, atom_t*);
void    atom_get_owners(atom_t*, atom_t*);
int     atom_is_container(atom_t*);
void    assert_arg(atom_t*, const char*);


// ---------------------------------------------------------------------- 
// list.c

atom_t* lst(void);
void    lst_del(atom_t*);
atom_t* lst_cp(atom_t*, atom_t*, atom_t*);
void    lst_insert(atom_t*, int, atom_t*, char);
int     lst_idx(atom_t*, atom_t*);
void    lst_remove(atom_t*, int, char);
char*   lst_tostr(atom_t*, int);
void    lst_print(atom_t*, int);
void    lst_pr(atom_t*, int);
void    lst_assert(atom_t*, const char*);
int     lst_len(atom_t*);
int     lst_maxlen(atom_t*);
void    lst_free(atom_t*);

#define lst_ins(list, idx, item)    lst_insert(list, idx, item, 1)
#define lst_add(list, item)         lst_insert(list,  -1, item, 1)
#define lst_add_nobind(list, item)  lst_insert(list,  -1, item, 0)
#define lst_rem(list, idx)          lst_remove(list, idx, 1)
#define lst_rem_nounbind(list, idx) lst_remove(list, idx, 0)


// ---------------------------------------------------------------------- 
// dict.c

/* Results of key lookup */
typedef struct {
    int miss;
    int idx;
} lookup_t;

atom_t*  dict(int, atom_t*);
void     dict_del(atom_t*);
atom_t*  dict_cp(atom_t*, atom_t*, atom_t*);
void     dict_add(atom_t*, char*, atom_t*);
atom_t*  dict_get(atom_t*, char*);
atom_t*  dict_find(atom_t*, char*);
char*    dict_tostr(atom_t*);
void     dict_print(atom_t*);
void     dict_assert(atom_t*, const char*);
void     key_lookup(char**, char*, int, lookup_t*);
lookup_t dict_lookup(dict_t*, char*);


// ---------------------------------------------------------------------- 
// globenv.c

extern atom_t* global_env;
extern atom_t* active_env;

void globenv_init(void);
void globenv_del(void);


// ---------------------------------------------------------------------- 
// main.c 

extern char* input;

void script(const char*);
void repl(void);
void magic(void);


// ---------------------------------------------------------------------- 
// eval.c apply.c

atom_t* eval(atom_t*, atom_t*, atom_t**);
atom_t* apply(atom_t*, atom_t*, atom_t*);
atom_t* apply_op(atom_t*, atom_t*, int, atom_t**);


// ---------------------------------------------------------------------- 
// parser.c

#define DELIM       "()"        // delimiters
#define RESERVED    "\"#$"      // can't be used in symbolic names

/* Token. */
typedef struct {
    char*       val;
    const char* pos;
} token_t;

/* Parse */
atom_t* parse(void);
atom_t* read_from_tokens(void);
atom_t* make_atom(token_t*);

/* Tokenize */
void     tokenize(void);
token_t* tok_new(char*, const char*);
unsigned tok_len(void);
void     tokens_del(void);


// ---------------------------------------------------------------------- 
// utils.c

/* Messages */
void intromsg(void);
void helpmsg(void);
void errmsg(const char*, const char*, const char*, const char*);

/* Memory */
void safe_memory_free(void**);
#define safe_free(ptr) safe_memory_free((void**) &(ptr))

/* Strings */
char* strip_quotes(const char*);
char* add_quotes(const char*);
int   streq(const char*, const char*);


// ---------------------------------------------------------------------- 
// operators.c

atom_t* op_print();
atom_t* op_println();
atom_t* op_math1(double (*op)(double));
atom_t* op_math1m(double (*op)(double));
atom_t* op_math2(double (*op)(double, double));
atom_t* op_math2r(double (*op)(double, double));
atom_t* op_rel(double (*op)(char, void*, void*));
atom_t* op_copy();
atom_t* op_type();
atom_t* op_list();
atom_t* op_list_get();
atom_t* op_list_set();
atom_t* op_list_len();
atom_t* op_list_add();
atom_t* op_list_ins();
atom_t* op_list_rem();
atom_t* op_list_merge();

double op_add(double, double);
double op_sub(double, double);
double op_mul(double, double);
double op_div(double, double);
double op_mod(double, double);
double op_inc(double);
double op_dec(double);
double op_eq(char, void*, void*);
double op_ne(char, void*, void*);
double op_lt(char, void*, void*);
double op_gt(char, void*, void*);
double op_le(char, void*, void*);
double op_ge(char, void*, void*);
double op_and(double, double);
double op_or(double, double);
double op_not(double);
double op_band(double, double);
double op_bor(double, double);
double op_bxor(double, double);
double op_bnot(double);
double op_blsh(double, double);
double op_brsh(double, double);
double op_acos(double);
double op_asin(double);
double op_atan(double);
double op_atan2(double, double);
double op_cos(double);
double op_cosh(double);
double op_sin(double);
double op_sinh(double);
double op_tanh(double);
double op_exp(double);
double op_frexp(double);
double op_ldexp(double, double);
double op_log(double);
double op_log10(double);
double op_modf(double);
double op_pow(double, double);
double op_sqrt(double);
double op_ceil(double);
double op_fabs(double);
double op_floor(double);
double op_fmod(double, double);


# endif

