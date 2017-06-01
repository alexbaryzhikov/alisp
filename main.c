/*
Alisp interpreter.
Can be run in REPL mode and/or to interpret a source file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisp.h>

char* input = NULL;

/* Main. */
int main(int argc, char* argv[]) {
    globenv_init();  // create global environment

    if (argc == 1) {
        intromsg();
        while (1)
            repl();

    } else if (argc == 2 && argv[1][0] != '-') {
        script(argv[1]);

    } else if (argc == 3 && argv[1][0] != '-' && streq(argv[2], "-i")) {
        script(argv[1]);
        while (1)
            repl();

    } else
        helpmsg();

    globenv_del();  // deallocate global environment
}

/*
--------------------------------------
script

    Run a script.
--------------------------------------
*/
void script(const char* filename) {
    if (!filename) {
        errmsg("Script", "no file name given", NULL, NULL);
        return;
    }

    FILE *f;
    if ((f = fopen(filename, "r")) == NULL) {
        errmsg("Script", "failed to open file", filename, filename);
        return;
    }

    // Get the size of the file
    fseek(f, 0L, SEEK_END);
    long bufsize = ftell(f);
    if (bufsize == -1) {
        errmsg("Script", "failed to read file", filename, filename);
        fclose(f);
        return;
    }

    safe_free(input);
    input = malloc(sizeof(char) * (bufsize + 1));

    // Read the entire file into memory
    fseek(f, 0L, SEEK_SET);
    size_t i = fread(input, sizeof(char), bufsize, f);
    if (ferror(f)) {
        errmsg("Script", "failed to read file", filename, filename);
        fclose(f);
        safe_free(input);
        return;
    } else
        input[i] = '\0';
    fclose(f);

    if (strlen(input) == 0) {
        safe_free(input);
        return;
    }

#ifdef DEBUG
printf("....  script:                  Building parse tree\n");
#endif

    // Parse
    atom_t* parse_tree = parse();

    if (!parse_tree) {
        safe_free(input);
        return;
    }
    atom_bind(parse_tree, global_env);

#ifdef DEBUG
printf("....  script:                  Evaluating parse tree\n");
#endif

    // Evaluate
    eval(parse_tree, global_env, NULL);

#ifdef DEBUG
printf("....  script:                  Deallocating parse tree\n");
#endif

    atom_unbind(parse_tree, global_env);
    atom_del(parse_tree);
    safe_free(input);
}

/*
--------------------------------------
repl

    Read-evaluate-print loop.
--------------------------------------
*/
void repl() {
    // Read input
    input = malloc(32);
    unsigned imax = 32;
    unsigned i;
    printf("~ ");

    for(i = 0; (input[i] = getchar()) != EOF && input[i] != '\n'; ++i)
        if (i == imax - 2)
            input = realloc(input, imax *= 2);
    input[i] = '\0';

    if (strlen(input) == 0) {
        safe_free(input);
        return;
    }

    // Magic commands
    if (input[0] == '$') {
        magic();
        if (input)
            safe_free(input);
        return;
    }

#ifdef DEBUG
printf("....  repl:                    Building parse tree\n");
#endif

    // Parse
    atom_t* parse_tree = parse();
    if (!parse_tree) {
        safe_free(input);
        return;
    }
    atom_bind(parse_tree, global_env);

#ifdef DEBUG
printf("....  repl:                    Evaluating parse tree\n");
#endif

    // Evaluate
    atom_t* val = eval(parse_tree, global_env, NULL);
    if (val && val->type != NIL) {
        char* o = atom_tostr(val);
        printf("%s\n", o);
        safe_free(o);
    }

#ifdef DEBUG
printf("....  repl:                    Deallocating parse tree\n");
#endif

    atom_unbind(parse_tree, global_env);
    if (val == parse_tree)
        val = NULL;
    else if (val)
        atom_del(val);
    atom_del(parse_tree);
    safe_free(input);
}

/*
--------------------------------------
magic
    
    Run a magic command.
--------------------------------------
*/
void magic() {
    if (streq(input+1, "help")) {
        printf("Magic commands:\n"
               "  $exit             Graceful exit from the interpreter.\n"
               "  $env              View global environment contents.\n"
               "  $run              Run script file.\n"
               "  $about            Info about the program.\n");

    } else if (streq(input + 1, "env")) {
        dict_print(global_env);

    } else if (streq(input + 1, "exit")) {
        safe_free(input);
        globenv_del();
        exit(EXIT_SUCCESS);

    } else if (!strncmp(input + 1, "run ", 4)) {
        if (strlen(input) > 5)
            script(input + 5);

    } else if (streq(input+1, "about")) {
        printf("Alisp interpreter by Alex Baryzhikov.\n");

    } else {
        printf("Unrecognized command: %s\n", input + 1);
    }
}

