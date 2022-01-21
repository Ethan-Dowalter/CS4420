/**
 *        @file: bash.h
 *      @author: Ethan Dowalter
 *        @date: September 27, 2021
 *       @brief: header file with some struct definitions for bash shell program
 */

/*
 * Ostermann's shell header file
 */


#define MAX_ARGS 100


/* This is the definition of a command */
struct command {
    char *command;
    int argc;
    struct args *pargs;
    char *infile;
    char *outfile;
    char *errfile;

    int output_append;		/* boolean: append stdout? */
    int error_append;		/* boolean: append stderr? */

    struct command *nextpipe;
};

struct args {
    char *arg;
    struct args *next;
};

struct io_redir {
    char *file;
    int redir_type;         /* 0 = stdin, 1 = stdout, 2 = stdout append, 3 = stderr, 4 = stderr append */
    struct io_redir *next;
};


/* externals */
extern int yydebug;


/* global routine decls */
void doline(struct command *pass, int line_num, int errorflag);
int yyparse(void);
