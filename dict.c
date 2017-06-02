/*
Dictionary:
  - keys are strings, values are atomic objects
  - looks up keys in itself and in an chain of enclosing environments
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alisp.h"

/*
--------------------------------------
dict

    Create a new dictionary.
--------------------------------------
*/
atom_t* dict(int size, atom_t* parent) {
    
    // Make a dictionary
    dict_t* d = malloc(sizeof(dict_t));
    d->bindlist = NULL;
    d->lock = 0;     // mutex to prevent recurrent deallocation
    int n;
    for (n = 2; n < size; n <<= 1);  // pick pow-of-2 n that is greater or equal to size
    d->len = 0;
    d->maxlen = n;
    d->keys = malloc(n * sizeof(char*));    // mem for n key pointers
    d->vals = malloc(n * sizeof(atom_t*));  // mem for n object pointers
    d->parent = parent;
    
    // Make a dictionary object
    atom_t* obj = malloc(sizeof(atom_t));
    obj->val.dict = d;
    obj->type = DICTIONARY;
    obj->bindings = 0;

    return obj;
}

/*
--------------------------------------
dict_del

    Deallocate a dictionary.
--------------------------------------
*/
void dict_del(atom_t* obj) {
    dict_assert(obj, "dict_del");

#ifdef DEBUG
char* dbg_s = atom_tostr(obj);
printf("....  dict_del:                Deallocating %s\n", dbg_s);
safe_free(dbg_s);
#endif

    dict_t* d = obj->val.dict;
    atom_t* item;
    d->lock = 1;  // lock this object

    // Deallocate bound objects
    for (int i = 0; i < d->len; ++i) {
        safe_free(d->keys[i]);  // free key string
        item = d->vals[i];
        if (!(atom_is_container(item) && item->val.list->lock)) {
            atom_unbind(item, obj);
            atom_del(item);     // free value object
        }
    }

    // Deallocate the rest
    safe_free(d->keys);     // free keys
    safe_free(d->vals);     // free vals
    list_free(d->bindlist);
    safe_free(d);           // free dictionary
    safe_free(obj);         // free object
}

/*
--------------------------------------
dict_cp

    Copy a dictionary (subroutine for atom_cp).
--------------------------------------
*/
atom_t* dict_cp(atom_t* obj, atom_t* objects, atom_t* copies) {
    int i = list_idx(objects, obj);
    if (i != -1)
        return copies->val.list->items[i];  // object already has a copy

    atom_t* copy = dict(obj->val.dict->maxlen, obj->val.dict->parent);
    list_add_h(objects, obj);
    list_add_h(copies, copy);

    char** keys = obj->val.dict->keys;
    atom_t** vals = obj->val.dict->vals;
    for (int i = 0; i < obj->val.dict->len; ++i)
        dict_add(copy, keys[i], atom_cp(vals[i], objects, copies));
    return copy;
}

/*
--------------------------------------
dict_add

    Add a pair (key, value) to the dictionary.
--------------------------------------
*/
void dict_add(atom_t* dictionary, char* key, atom_t* value) {
    dict_assert(dictionary, "dict_add");
    if (!strlen(key) || !value) {
        printf("\x1b[95m" "Fatal error: dict_add: bad arguments!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }

    dict_t* d = dictionary->val.dict;

    // allocate more space if needed
    if (d->len == d->maxlen) {
        d->maxlen *= 1.5;
        d->keys = realloc(d->keys, d->maxlen * sizeof(char*));
        d->vals = realloc(d->vals, d->maxlen * sizeof(atom_t*));
    }

    // search for position to insert the key, so the list remains sorted
    int idx, hit = dict_lookup(dictionary, key, &idx);
    
    // insert the key
    if (!hit) {
        // offset keys and values to make a free spot
        memmove(d->keys + idx + 1, d->keys + idx, (d->len - idx) * sizeof(char*));
        memmove(d->vals + idx + 1, d->vals + idx, (d->len - idx) * sizeof(atom_t*));
        ++d->len;
        // insert new key
        d->keys[idx] = malloc(strlen(key) + 1);
        strcpy(d->keys[idx], key);
        // insert new value
        d->vals[idx] = value;
        atom_bind(value, dictionary);

    } else if (d->vals[idx] != value) {
        // delete old value
        atom_t* oldval = d->vals[idx];
        atom_unbind(oldval, dictionary);
        atom_del(oldval);
        // insert new value
        d->vals[idx] = value;
        atom_bind(value, dictionary);
    }
}

/*
--------------------------------------
dict_get

    Get a value associated with a key.
--------------------------------------
*/
atom_t* dict_get(atom_t* dictionary, char* key) {
    dict_assert(dictionary, "dict_get");
    if (!strlen(key)) {
        printf("\x1b[95m" "Fatal error: dict_get: bad key!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    dict_t* d = dictionary->val.dict;
    int idx, hit = dict_lookup(dictionary, key, &idx);
    if (hit) {
        return d->vals[idx];
    } else {
        if (d->parent)
            return dict_get(d->parent, key);
        else
            return NULL;
    }
}

/*
--------------------------------------
dict_find

    Find a dictionary containing the given key.
--------------------------------------
*/
atom_t* dict_find(atom_t* dictionary, char* key) {
    dict_assert(dictionary, "dict_find");
    if (!strlen(key)) {
        printf("\x1b[95m" "Fatal error: dict_find: bad key!\n" "\x1b[0m");
        exit(EXIT_FAILURE);
    }
    dict_t* d = dictionary->val.dict;
    int idx, hit = dict_lookup(dictionary, key, &idx);
    if (hit) {
        return dictionary;
    } else {
        if (d->parent)
            return dict_find(d->parent, key);
        else
            return NULL;
    }
}

/*
--------------------------------------
dict_lookup

    Key lookup in a dictionary.
    Returns 1 if key is found, 0 otherwise. Leaves index in *idx.
--------------------------------------
*/
int dict_lookup(atom_t* dictionary, char* key, int* idx) {
    dict_assert(dictionary, "dict_lookup");
    *idx = 0;
    int len = dictionary->val.dict->len;

    if (!len)  // dictionary is empty
        return 0;

    char** keys = dictionary->val.dict->keys;
    int tmp, hit = 0, left = 1, right = len;

    while (left <= right) {
        *idx = left + ((right - left) >> 1);
        tmp = strcmp(key, keys[*idx-1]);
        if (tmp < 0) {
            if ((right = *idx - 1) < left) {
                (*idx)--;
                break;
            }
        } else if (tmp > 0) {
            left = *idx + 1;
        } else {
            hit = 1;
            (*idx)--;
            break;
        }
    }

    return hit;
}

/*
--------------------------------------
dict_tostr

    Create a string representing a dictionary.
--------------------------------------
*/
char* dict_tostr(atom_t* dictionary, int depth) {
    dict_assert(dictionary, "dict_tostr");
    dict_t* d = dictionary->val.dict;
    char* o;
    char buf[1024];
    char ellipsis = 0;
    strcpy(buf, "{");
    for (int i = 0; i < d->len; ++i) {
        
        if (d->vals[i]->type == STD_OP) {
            if (i == d->len - 1)
                strcat(buf, " ... ");
            continue;
        }
        
        if (ellipsis || strlen(buf) > 1000) {
            strcat(buf, " ... ");
            break;
        }
        
        o = atom_tostring(d->vals[i], depth ? depth - 1 : depth);
        if (strlen(buf) + strlen(d->keys[i]) + strlen(o) > 1000) {
            ellipsis = 1;
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
dict_print

    Print all key:value pairs in a dictionary.
--------------------------------------
*/
void dict_print(atom_t* dictionary, int depth) {
    dict_assert(dictionary, "dict_print");
    dict_t* d = dictionary->val.dict;
    printf("{\n");
    for (int i = 0; i < d->len; i++) {
        if (d->vals[i]->type != STD_OP) {
            char* o = atom_tostring(d->vals[i], depth ? depth - 1 : depth);
            printf("  %s : %s\n", d->keys[i], o);
            safe_free(o);
        }
    }
    printf("}\n");
}

/*
--------------------------------------
dict_assert

    Assert that object exists and is of DICTIONARY type.
--------------------------------------
*/
void dict_assert(atom_t* obj, const char* fname) {
    if (!obj || obj->type != DICTIONARY) {
        printf("\x1b[95m" "Fatal error: %s: not a dictionary!\n" "\x1b[0m", fname);
        exit(EXIT_FAILURE);
    }
}
