/*
Alisp source code parser.
Can be used for parsing source file, or input line in REPL mode.  Produces parse tree
coposed of atom list nodes as branches and other atom types as leaves.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <alisp.h>

token_t** tokens = NULL;        // list of tokens
token_t** tok = NULL;           // token pointer


// ---------------------------------------------------------------------- 
// Parse

/*
--------------------------------------
parse

    Take a string and return a parse tree.
--------------------------------------
*/
atom_t* parse() {
    tokenize();
    if (!tokens)                    // error during tokenizing
        return NULL;

    token_t** tok_end = tokens + tok_len();
    atom_t* item = read_from_tokens();

    if (!item) {                    // error during parsing
        tokens_del();
        return NULL;

    } else if (tok == tok_end) {    // a single item
        tokens_del();
        return item;

    } else {                        // multiple items
        atom_t* ptree;
        ptree = lst_new();
        lst_add(ptree, sym_new("block"));
        lst_add(ptree, item);

        while (tok != tok_end) {
            if ((item = read_from_tokens()))
                lst_add(ptree, item);
            else {
                atom_del(ptree);
                tokens_del();
                return NULL;
            }
        }
        tokens_del();
        return ptree;
    }
}

/* Read an expression from a sequence of tokens. */
atom_t* read_from_tokens() {
    if (tok == tokens + tok_len()) {
        errmsg("Syntax", "unexpected EOF while reading", NULL, NULL);
        return NULL;
    }
    token_t* token = *tok++;
    if (!strcmp(token->val, "(")) {
        atom_t* list = lst_new();
        atom_t* a;
        while (strcmp((*tok)->val, ")")) {
            if ((a = read_from_tokens()))
                lst_add(list, a);
            else
                return NULL;
        }
        ++tok;  // pop ')'
        return list;
    } else if (!strcmp(token->val, ")")) {
        errmsg("Syntax", "unexpected ')'", token->pos, input);
        return NULL;
    } else {
        return make_atom(token);
    }
}

/* Convert a token into an atomic object. */
atom_t* make_atom(token_t* token) {
    if (!strlen(token->val)) {
        errmsg("Syntax", "zero-length token", token->pos, input);
        return NULL;
    }
    char* t;
    double x = strtod(token->val, &t);
    if (*t == '\0') {
        return num_new(x);      // number
    } else if (x) {
        errmsg("Syntax", "invalid symbol", token->pos, input);
        return NULL;
    } else {
        return sym_new(token->val);   // symbol
    }
}


// ---------------------------------------------------------------------- 
// Tokenize

/*
--------------------------------------
tokenize

    Convert a string into a list of tokens.
--------------------------------------
*/
void tokenize() {
    tokens = NULL, tok = NULL;

    // Count the number of tokens and catch lexical errors
    unsigned n;
    const char* p;

    for (n = 0, p = input; *p != '\0'; ++n) {
        while (*p != '\0' && (isspace(*p) || *p == '#')) {        
            for (; isspace(*p); ++p);       // skip white space
            if (*p == '#')                  // skip comment
                for (; *p != '\n' && *p != '\0'; ++p);
        }
        if (*p == '\0')
            break;
        
        if (strchr(DELIM, *p)) {        // parenthesis
            ++p;
        } else if (*p == '"') {         // quoted string
            while (*(++p) != '\0' && (*p != '"' || *(p-1) == '\\'));
            if (*p == '"')
                p++;
        } else if (isgraph(*p)) {       // number or symbol
            while (*(++p) != '\0' && isgraph(*p) && !strchr(DELIM, *p))
                if (strchr(RESERVED, *p)) {
                    errmsg("Lexical", "invalid symbol", p, input);
                    return;
                }
        } else {                        // bad character
            errmsg("Lexical", "bad character", p, input);
            return;
        }
    }

    if (n == 0)
        return;

    // Create list of tokens
    tokens = (token_t**)malloc((n + 1) * sizeof(token_t*));
    const char* p0;
    int i;
    char* tmp;

    for (tok = tokens, p = input; *p != '\0'; ++tok) {
        while (*p != '\0' && (isspace(*p) || *p == '#')) {        
            for (; isspace(*p); ++p);       // skip white space
            if (*p == '#')                  // skip comment
                for (; *p != '\n' && *p != '\0'; ++p);
        }
        if (*p == '\0')
            break;
        
        if (strchr(DELIM, *p)) {        // parenthesis
            tmp = (char*)malloc(2);
            tmp[0] = *p, tmp[1] = '\0';
            *tok = tok_new(tmp, p++);

        } else if (*p == '"') {         // quoted string
            p0 = p;
            for (i = 1; *(++p) != '\0' && (*p != '"' || *(p-1) == '\\'); ++i);
            if (*p == '"')
                ++p, ++i;
            tmp = (char*)malloc(i + 1);
            memcpy(tmp, p0, i);
            tmp[i]= '\0';
            *tok = tok_new(tmp, p0);
        
        } else {                        // number or symbol
            p0 = p;
            for (i = 1; *(++p) != '\0' && isgraph(*p) && !strchr(DELIM, *p); ++i);
            tmp = (char*)malloc(i + 1);
            memcpy(tmp, p0, i);
            tmp[i]= '\0';
            *tok = tok_new(tmp, p0);
        }
    }

    // Terminate list with zero string
    tmp = (char*)malloc(1);
    *tmp = '\0';
    *tok = tok_new(tmp, NULL);

    tok = tokens;  // reset token pointer
}

/* Create a token. */
token_t* tok_new(char* v, const char* p) {
    token_t* t = (token_t*)malloc(sizeof(token_t));
    t->val = v, t->pos = p;
    return t;
}

/* Return length of token list. */
unsigned tok_len() {
    unsigned i;
    for (i = 0; *tokens[i]->val != '\0'; ++i);
    return i;
}

/* Deallocate tokens. */
void tokens_del() {
    for (tok = tokens; (*tok)->val[0] != '\0'; ++tok) {
        safe_free((*tok)->val);
        safe_free(*tok);
    }
    safe_free((*tok)->val);
    safe_free(*tok);
    safe_free(tokens);
}
