/*
Utilities.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>


// ---------------------------------------------------------------------- 
// Messages

/* Display repl intro message. */
void intromsg() {
    printf("Alisp interpreter 0.1, May 2017.\n"
           "Type $help to view list of commands.\n");
}

/* Display help message. */
void helpmsg() {
    printf("Alisp interpreter.\n"
           "Usage:\n"
           "    alisp                   REPL mode.\n"
           "    alisp script            Run script from file.\n"
           "    alisp script -i         Run script from file and stay in REPL.\n");
}

/* Display status message. */
void statusmsg(int status, char* msg) {
#ifdef REPORT_STATUS
    if (status)
        printf("[  " "\x1b[92m" "OK" "\x1b[0m" "  ] %s\n", msg);
    else
        printf("[ " "\x1b[91m" "FAIL" "\x1b[0m" " ] %s\n", msg);
#endif
}

/* Display error message.
type - error type, msg - error text, p - error position, s - context string. */
void errmsg(const char* type, const char* msg, const char* p, const char* s) {
    if (p && s) {
        char err[256];
        char pre[256];
        int i, errpos;
        
        for (errpos = 0; p != s && *(--p) != '\n'; ++errpos);
        if (*p == '\n')
            ++p;
        
        for (i = 0; i < 256 && p[i] != '\0' && p[i] != '\n'; ++i) {
            err[i] = p[i];
            if (i < errpos) {
                if (p[i] == '\t')
                    pre[i] = '\t';
                else
                    pre[i] = ' ';
            }
        }
        err[i] = '\0', pre[errpos] = '\0';
        
        printf("\x1b[91m" "%s error: " "\x1b[0m" "%s\n%s\n", type, msg, err);
        printf("%s" "\x1b[92m" "^\n" "\x1b[0m", pre);
    } else
        printf("\x1b[91m" "%s error: " "\x1b[0m" "%s\n", type, msg);
}


// ---------------------------------------------------------------------- 
// Memory

/* Safe memory deallocation.
Guarantees that there's a pointer and it points to something.
Doesn't guarantee that it hasn't been already freed. */
void safe_memory_free(void** p_addr) {
    if (p_addr != NULL && *p_addr != NULL) {
        free(*p_addr);
        *p_addr = NULL;
    }
}


// ---------------------------------------------------------------------- 
// Strings

/* Return symbolic string with quotes stripped. */
atom_t* strip_quotes(atom_t* a) {
    size_t alen = strlen(a->val.sym);
    char buf[alen - 1];
    strncpy(buf, a->val.sym + 1, alen - 2);
    buf[alen - 2] = '\0';
    return sym_new(buf);
}

/* Return true if strings are equal, false otherwise. */
int streq(const char* s1, const char* s2) {
    return !strcmp(s1, s2);
}

