#ifndef ALISP_H
#define ALISP_H

// #define DEBUG
// #define REPORT_STATUS
// #define LIST_PRINT_EXPANDED

// ---------------------------------------------------------------------- 
// types.c 

/* Types of atomic objects */
enum { NIL, NUMBER, SYMBOL, LIST, LIST_EMPTY, EOLIST, STD_OP, FUNCTION };

/* Standard operator types */
enum { PRINT, PRINTLN, MATH1, MATH1_M, MATH2, MATH2_R, REL };

typedef struct Atom  atom_t;
typedef struct Edict edict_t;

/* Closure */
typedef struct {
    atom_t*  params;
    atom_t*  body;
    edict_t* env;
    char     lock;
} closure_t;

/* Atomic object */
typedef struct Atom {
    union {
        double*    num;
        char*      sym;
        atom_t**   lst;
        double     (*op_math1)(double);
        double     (*op_math2)(double, double);
        closure_t* closure;
    } val;
    char type;
    char optype;
    unsigned bindings;
} atom_t;

extern atom_t nilobj;
extern atom_t list_emptyobj;
extern atom_t eolistobj;

/* Numbers and symbols */
atom_t*  num_new(double);
atom_t*  sym_new(const char*);

/* Operators */
atom_t*  op_print_new();
atom_t*  op_println_new();
atom_t*  op_math1_new(double (*op)(double));
atom_t*  op_math1m_new(double (*op)(double));
atom_t*  op_math2_new(double (*op)(double, double));
atom_t*  op_math2r_new(double (*op)(double, double));
atom_t*  op_rel_new(double (*op)(double, double));

/* Functions */
atom_t*  func_new(atom_t*, atom_t*, edict_t*);

/* Lists */
atom_t*  lst_new(void);
void     lst_add(atom_t*, atom_t*);
char*    lst_tostr(atom_t*, int);
void     lst_print(atom_t*, int);

void     lst_assert(atom_t*);
unsigned lst_len(atom_t*);
unsigned lst_maxlen(atom_t*);
void     lst_expand(atom_t*);
void     lst_pr_lin(atom_t*, int);
void     lst_pr_exp(atom_t*, int, int);

/* Common methods */
void     safe_atom_del(void**);
char*    tostr(atom_t*);
atom_t*  atom_copy(atom_t*);
char*    type(atom_t*);

#define atom_del(ptr) safe_atom_del((void**) &(ptr))


// ---------------------------------------------------------------------- 
// edict.c

/* Environment dictionary */
typedef struct Edict {
    char**   keys;
    atom_t** vals;
    unsigned maxlen;
    unsigned len;
    edict_t* parent;
    unsigned tethers;
    char     ghost;
    char     lock;
} edict_t;

edict_t* edict_new(unsigned, edict_t*);
void     edict_add(edict_t*, char*, atom_t*);
atom_t*  edict_get(edict_t*, char*);
edict_t* edict_find(edict_t*, char*);
void     edict_del(edict_t*);
char*    edict_tostr(edict_t*);
void     edict_print(edict_t*);


// ---------------------------------------------------------------------- 
// globenv.c

extern edict_t* global_env;

void globenv_init(void);
void globenv_del(void);

double op_add(double, double);
double op_sub(double, double);
double op_mul(double, double);
double op_div(double, double);
double op_mod(double, double);
double op_inc(double);
double op_dec(double);
double op_eq(double, double);
double op_ne(double, double);
double op_lt(double, double);
double op_gt(double, double);
double op_le(double, double);
double op_ge(double, double);
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


// ---------------------------------------------------------------------- 
// main.c 

extern char* input;

void script(const char*);
void repl(void);
void magic(void);


// ---------------------------------------------------------------------- 
// eval.c

atom_t* eval(atom_t*, edict_t*, atom_t**);
atom_t* apply(atom_t*, atom_t*, atom_t*);
atom_t* apply_op(atom_t*, atom_t*, int, atom_t**);


// ---------------------------------------------------------------------- 
// parser.c

#define DELIM       "()"        // delimiters
#define RESERVED    "\"#$"      // can't be used in symbolic names

/* Token. */
typedef struct {
    char* val;
    const char* pos;
} token_t;

/* Parse */
atom_t*  parse(void);
atom_t*  read_from_tokens(void);
atom_t*  make_atom(token_t*);

/* Tokenize */
void     tokenize(void);
token_t* tok_new(char*, const char*);
unsigned tok_len(void);
void     tokens_del(void);


// ---------------------------------------------------------------------- 
// utils.c

/* Messages */
void    intromsg(void);
void    helpmsg(void);
void    statusmsg(int, char*);
void    errmsg(const char*, const char*, const char*, const char*);

/* Memory */
void    safe_memory_free(void**);
#define safe_free(ptr) safe_memory_free((void**) &(ptr))

/* Strings */
atom_t* strip_quotes(atom_t*);
int     streq(const char*, const char*);


#endif
