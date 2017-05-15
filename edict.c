/*
Environment Dictionary:
  - keys are strings, values are doubles
  - has link to a parent
  - looks up keys in itself, then enclosing scope, and further up the parenting chain
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

/* klookup_t: results of key lookup in a list of sorted keys. */
typedef struct {
    int miss;
    unsigned idx;
} klookup_t;

void key_lookup(char**, char*, unsigned, klookup_t*);
klookup_t klookup(edict_t*, char*);

/*
--------------------------------------
edict_new

    Create a new dictionary.
--------------------------------------
*/
edict_t* edict_new(unsigned size, edict_t* parent) {
    unsigned n = 1;
    while (n < size)
        n <<= 1;  // pick pow-of-2 n that is greater or equal to size
    edict_t* d = (edict_t*)malloc(sizeof(edict_t));
    d->keys = (char**)malloc(n * sizeof(char*));      // mem for n key pointers
    memset(d->keys, 0, n * sizeof(char*));
    d->vals = (atom_t**)malloc(n * sizeof(atom_t*));  // mem for n object pointers
    memset(d->vals, 0, n * sizeof(atom_t*));
    d->maxlen = n;
    d->len = 0;
    d->parent = parent;
    d->tethers = 0;  // number of closures that include this environment
    d->ghost = 0;    // environmnent become ghost after function returns, but tethers remain
    d->lock = 0;     // mutex to prevent recurrent deallocation
    return d;
}

/*
--------------------------------------
edict_add

    Add a pair (key, value) to the dictionary.
--------------------------------------
*/
void edict_add(edict_t* d, char* key, atom_t* value) {
    if (!d || !strlen(key) || !value)
        return;

    // allocate more space if needed
    if (d->len == d->maxlen) {
        d->maxlen <<= 1;
        d->keys = (char**)realloc(d->keys, d->maxlen * sizeof(char*));
        memset(d->keys + d->len, 0, d->len * sizeof(char*));
        d->vals = (atom_t**)realloc(d->vals, d->maxlen * sizeof(atom_t*));
        memset(d->vals + d->len, 0, d->len * sizeof(atom_t*));
    }

    // search for position to insert the key, so the list remains sorted
    klookup_t kl = klookup(d, key);
    
    // insert the key
    if (kl.miss) {
        // offset keys and values to make a free spot
        for (unsigned i = d->len; i > kl.idx; i--) {
            d->keys[i] = d->keys[i-1];
            d->vals[i] = d->vals[i-1];
        }
        ++d->len;
        // insert new key
        d->keys[kl.idx] = (char*)malloc(strlen(key) + 1);
        strcpy(d->keys[kl.idx], key);
        // insert new value
        d->vals[kl.idx] = value;
        ++value->bindings;

    } else if (d->vals[kl.idx] != value) {
        // delete old value
        --d->vals[kl.idx]->bindings;
        atom_del(d->vals[kl.idx]);
        // insert new value
        d->vals[kl.idx] = value;
        ++value->bindings;
    }
}

/*
--------------------------------------
edict_get

    Get a value associated with a key.
--------------------------------------
*/
atom_t* edict_get(edict_t* d, char* key) {
    klookup_t kl = klookup(d, key);
    if (kl.miss) {
        if (d->parent)
            return edict_get(d->parent, key);
        else
            return NULL;
    } else
        return d->vals[kl.idx];
}

/*
--------------------------------------
edict_find

    Find a dictionary containing the given key.
--------------------------------------
*/
edict_t* edict_find(edict_t* d, char* key) {
    klookup_t kl = klookup(d, key);
    if (kl.miss) {
        if (d->parent)
            return edict_find(d->parent, key);
        else
            return NULL;
    } else
        return d;
}

/*
--------------------------------------
edict_del

    Deallocate a dictionary.
--------------------------------------
*/
void edict_del(edict_t* d) {
    if (d == NULL)
        return;

#ifdef DEBUG
char* se = edict_tostr(d);
printf("....  edict_del:     Deallocating %s\n", se);
#endif

    unsigned i;

    // If d is a ghost, then check tethers and bound functions
    if (d->ghost) {
        if (d->tethers) {

#ifdef DEBUG
printf("....  edict_del:     Tethered, aborting.\n");
safe_free(se);
#endif

            return;
        }
        for (i = 0; i < d->len; ++i)
            if (d->vals[i]->type == FUNCTION && d->vals[i]->bindings) {

#ifdef DEBUG
printf("....  edict_del:     Contains exported function, aborting.\n");
safe_free(se);
#endif
                return;
            }

    } else {  // not a ghost
        
        // Remove bindings from functions
        for (i = 0; i < d->len; ++i)
            if (d->vals[i]->type == FUNCTION)
                --d->vals[i]->bindings;

        // Check tethers
        if (d->tethers) {
            d->ghost = 1;  // become a ghost

#ifdef DEBUG
printf("....  edict_del:     %s becomes a ghost (tethered)\n", se);
safe_free(se);
#endif

            return;
        }

        // Check bound functions
        for (i = 0; i < d->len; ++i)
            if (d->vals[i]->type == FUNCTION && d->vals[i]->bindings) {
                d->ghost = 1;  // become a ghost

#ifdef DEBUG
printf("....  edict_del:     %s becomes a ghost (contains exported function)\n", se);
safe_free(se);
#endif

                return;
            }
    }

    d->lock = 1;        // lock this environment

    // Deallocate bound objects
    for (i = 0; i < d->len; ++i) {
        safe_free(d->keys[i]);      // key string
        if (d->vals[i]->type != FUNCTION) {
            --d->vals[i]->bindings;
            atom_del(d->vals[i]);   // value object
        } else if (!d->vals[i]->val.closure->lock) {
            atom_del(d->vals[i]);   // value function
        }
    }

    // Deallocate contents and object
    safe_free(d->keys);              // free keys
    safe_free(d->vals);              // free vals
    safe_free(d);                    // free edict

#ifdef DEBUG
printf("....  edict_del:     %s -- deleted\n", se);
safe_free(se);
#endif

}

/*
--------------------------------------
edict_tostr

    Create a string representing a dictionary.
--------------------------------------
*/
char* edict_tostr(edict_t* d) {
    char buf[1024];
    char* o;
    strcpy(buf, "{");
    for (unsigned i = 0; i < d->len; i++) {
        if (d->vals[i]->type == STD_OP)
            continue;
        o = tostr(d->vals[i]);
        if (strlen(buf) + strlen(o) + 10 > 1024) {
            strcat(buf, " ... ");
            safe_free(o);
            break;
        } else {
            strcat(buf, d->keys[i]);
            strcat(buf, " : ");
            strcat(buf, o);
        }
        safe_free(o);
        if (i < d->len - 1)
            strcat(buf, ", ");
    }
    strcat(buf, "}");
    o = malloc(strlen(buf) + 1);
    strcpy(o, buf);
    return o;
}

/*
--------------------------------------
edict_print

    Print all key:value pairs in a dictionary.
--------------------------------------
*/
void edict_print(edict_t* d) {
    if (!d)
        return;
    printf("{\n");
    for (unsigned i = 0; i < d->len; i++) {
        if (d->vals[i]->type != STD_OP) {
            char* o = tostr(d->vals[i]);
            printf("  %s : %s\n", d->keys[i], o);
            safe_free(o);
        }
    }
    printf("}\n");
}


// ---------------------------------------------------------------------- 
// Auxiliary

/* key_lookup: key lookup in a list of sorted keys.
   Returns miss = {true if key miss}, idx = {position where key is or has to be}. */
void key_lookup(char** keys, char* key, unsigned len, klookup_t* res) {
    res->miss = 1, res->idx = 0;
    
    if (!len)
        return;  // empty list

    unsigned left = 1;
    unsigned right = len;

    while (left <= right) {
        res->idx = left + ((right - left) >> 1);
        if ((res->miss = strcmp(key, keys[res->idx-1])) < 0) {
            if ((right = res->idx - 1) < left) {
                --res->idx;
                break;
            }
        } else if (res->miss > 0) {
            left = res->idx + 1;
        } else {
            --res->idx;
            break;
        }
    }
}

/* klookup macro: d - dictionary, k - key. */
klookup_t klookup(edict_t* d, char* k) {
    klookup_t x; key_lookup(d->keys, k, d->len, &x); return x;
}

