/**
 *        @file: bash.h
 *      @author: Ethan Dowalter
 *        @date: October 20, 2021
 *       @brief: header file with some struct definitions for bash shell program
 */

/*
 * Ostermann's shell header file
 */



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

    int abs_path;           /* boolean: is command an absolute pathname? */

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

struct metacmdlist {
    struct metacmd *metacmd;
    struct metacmdlist *next;
};

struct metacmd {
    struct command *cmd;
    struct loopwhile *whileloop;
    struct loopfor *forloop;
    char *var_assign;
};

struct loopwhile {
    struct command *condition;
    struct metacmdlist *body;
};

struct loopfor {
    char *variable;
    struct args *loopargs;
    struct metacmdlist *body;
};

struct var {
    char *name;
    char *value;
    int is_exported;
};

/* externals */
extern int yydebug;
extern struct var vars[];


/* global routine decls */
void doline(struct command *pass, int line_num, int errorflag, int DEBUG, int *exec_fail);
int yyparse(void);
void AssignVar(char *string);
char *mygetenv(char *name);
