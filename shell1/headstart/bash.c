/**
 *        @file: bash.c
 *      @author: Ethan Dowalter
 *        @date: September 27, 2021
 *       @brief: main program for my bash shell (it just parses for now)
 */

/*
 * Headstart for Ostermann's shell project
 *
 * Shawn Ostermann -- Sept 9, 2021
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "bash.h"




int main(int argc, char *argv[]){
    yydebug = 1;  /* turn on ugly YACC debugging */
    yydebug = 0;  /* turn off ugly YACC debugging */

    /* parse the input file */
    yyparse();

    exit(0);
}



void doline(struct command *pcmd, int line_num, int errorflag){
    /// Only doline if there were no errors
    if(errorflag == 0)
    {
        // printf("Got a command\n");
        printf("========== line %d", line_num);
        printf(" ==========\n");

        int cmd_num = 1;        //keeps track of number of commands on the line
        do {
            printf("Command name: '%s", pcmd->command);
            printf("'\n");

            printf("    argv[0]: '%s", pcmd->command);
            printf("'\n");

            /// Walk through the linked list of args to print them
            struct args *current_arg = (struct args*) malloc(sizeof(struct args));
            current_arg = pcmd->pargs;
            for (int i = 1; i < pcmd->argc; i++)
            {
                printf("    argv[%d]: '%s", i, current_arg->arg);
                printf("'\n");
                current_arg = current_arg->next;
            }

            /// If there is more than one command, that means there are pipes
            if(cmd_num > 1)
            {
                printf("   stdin: 'PIPE%d", cmd_num - 1);
                printf("'\n");
            }
            else
            {
                printf("   stdin: '%s", pcmd->infile);
                printf("'\n");
            }

            if(pcmd->nextpipe != NULL)
            {
                printf("  stdout: 'PIPE%d", cmd_num);
                printf("'\n");
            }
            else
            {
                printf("  stdout: '%s", pcmd->outfile);
                printf("'");
                if(pcmd->output_append == 1) printf(" (append)");
                printf("\n");
            }
            
            printf("  stderr: '%s", pcmd->errfile);
            printf("'");
            if(pcmd->error_append == 1) printf(" (append)");
            printf("\n");

            pcmd = pcmd->nextpipe;

            // printf("Cmd : %d\n", cmd_num);
            cmd_num++;
        } while(pcmd != NULL);
    }
}
