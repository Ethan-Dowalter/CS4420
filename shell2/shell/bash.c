/**
 *        @file: bash.c
 *      @author: Ethan Dowalter
 *        @date: October 20, 2021
 *       @brief: main program for my bash shell
 */

/*
 * Headstart for Ostermann's shell project
 *
 * Shawn Ostermann -- Sept 9, 2021
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "parser.h"
#include "bash.h"

/// Global pipes to be shared by successive calls of doline
// int fd_pipe1[2] = {0, 1};
// int fd_pipe2[2];

void PrintStuff(struct command *pcmd, int line_num);
void export(struct command *pcmd, char *argv[], int DEBUG);


int main(int argc, char *argv[]){
    yydebug = 1;  /* turn on ugly YACC debugging */
    yydebug = 0;  /* turn off ugly YACC debugging */

    /// Initialize some special variables
    vars[0].name = "HOME";
    vars[0].value = getenv("HOME");
    vars[1].name = "PROMPT";
    vars[1].value = "Give me purpose > ";
    vars[2].name = "DEBUG";
    vars[2].value = "";
    vars[3].name = "PATH";
    vars[3].value = (char *)malloc(strlen(getenv("PATH")));
    memcpy(vars[3].value, getenv("PATH"), strlen(getenv("PATH")));
    // printf("path : %s\n", vars[3].value);
    if (isatty(0) == 1) printf("%s", vars[1].value);    //test whether program is referring to terminal

    /* parse the input file */
    yyparse();

    exit(0);
}



void doline(struct command *pcmd, int line_num, int errorflag, int DEBUG, int *exec_fail){
    /// Only doline if there were no errors or exec fails
    if(errorflag == 0 && *exec_fail == 0)
    {
        /// Load args in a NULL terminated array from a linked list for execv calls
        char *argv[pcmd->argc + 1];         //array for the args
        argv[0] = pcmd->command;            //first one has to be the command
        struct args *current = (struct args *) malloc(sizeof(struct args));
        current = pcmd->pargs;
        int index = 1;
        while (current != NULL)             //walk through linked list, adding args to array
        {
            argv[index] = current->arg;
            index++;
            current = current->next;
        }
        argv[index] = NULL;                 //NULL terminate

        if (strcmp(pcmd->command, "export") == 0)
        {
            export(pcmd, argv, DEBUG);
            PrintStuff(pcmd, line_num);
            return;
        }

        /// Load the PATH environment variable into an array
        char *path = (char *)malloc(strlen(vars[3].value));
        memcpy(path, vars[3].value, strlen(vars[3].value));

        int path_count = 1;         //number of tokens stored in path variable, separated by colons
        for (int i = 0; i < strlen(path); i++)
        {
            if (path[i] == ':')
            {
                path_count++;
            }
        }

        char *path_array[path_count];       //array of all paths in PATH variable
        path_array[0] = strtok(path, ":");      //split up path into individual tokens
        for (int i = 1; i < path_count; i++)
        {
            path_array[i] = strtok(NULL, ":");
        }

        char *my_path_array[path_count];    //new array of all paths in PATH variable, but with bigger size to append command
        for (int i = 0; i < path_count; i++)   //copy first one
        {
            my_path_array[i] = (char*)malloc(strlen(path));
            memcpy(my_path_array[i], path_array[i], strlen(path_array[i]));
        }
        char *command = (char*)malloc(strlen(pcmd->command) + 1);
        if (pcmd->abs_path == 1)                //if absolute path is given, no need to append command to path
        {
            memcpy(command, pcmd->command, strlen(pcmd->command));
            // command = pcmd->command;
        }
        else                                    //append command onto each of the paths
        {
            command[0] = '/';
            strcat(command, pcmd->command);
            for (int i = 0; i < path_count; i++)
            {
                strcat(my_path_array[i], command);
            }
        }
        
        if (DEBUG == 1)
        {
            printf("command : %s\n", command);
            // printf("path  : %s\n", path);
            printf("path count : %d\n", path_count);
            for (int i = 0; i < path_count; i++)
            {
                printf("my_path_array[%d] : %s\n", i, my_path_array[i]);
            }
            printf("abs_path : %d\n", pcmd->abs_path);
        }

        /// Search PATH variable to locate valid command
        if (pcmd->abs_path == 0)
        {
            index = 0;
            while (access(my_path_array[index], F_OK) != 0 && index < path_count)
            {
                index++;
            }
            if (index == path_count)
            {
                perror("Could not find command");
            }
            else
            {
                if (DEBUG == 1) printf("my_path_array[%d] : %s\n", index, my_path_array[index]);
                command = my_path_array[index];
            }
        }

        /// Pipe stuff
        // int fd_pipe_in = fd_pipe1[0];
        // int fd_pipe_out = 1;
        // if (pipe(fd_pipe2) == -1)
        // {
        //     perror("Pipe error!");
        //     return;
        // }
        // if (pcmd->nextpipe != NULL)
        // {
        //     fd_pipe_out = fd_pipe2[1];
        // }
        // printf("take1\n");
        // printf(" in : %d     out : %d\n", fd_pipe_in, fd_pipe_out);
        // printf("in2 : %d    out2 : %d\n", fd_pipe1[0], fd_pipe1[1]);
        // printf("in3 : %d    out3 : %d\n", fd_pipe2[0], fd_pipe2[1]);
        
        /// Execute command
        pid_t pid = fork();

        /* child */
        if (pid == 0)
        {
            /// File redirection
            int fd_stdin, fd_stdout, fd_stderr;
            if (strcmp(pcmd->infile, "<undirected>") != 0)
            {
                fd_stdin = open(pcmd->infile,O_RDONLY);
                dup2(fd_stdin,0);
                close(fd_stdin);
            }
            // else
            // {
            //     if (fd_pipe_in != 0)
            //     {
            //         printf("test in!!!!\n");
            //         dup2(fd_pipe_in, 0);
            //         close(fd_pipe_in);
            //     }
            // }

            if (strcmp(pcmd->outfile, "<undirected>") != 0)
            {
                if (pcmd->output_append == 1)
                {
                    fd_stdout = open(pcmd->outfile, O_WRONLY|O_APPEND);
                }
                else
                {
                    fd_stdout = open(pcmd->outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                }
                dup2(fd_stdout,1);
                close(fd_stdout);
            }
            // else
            // {
            //     if (fd_pipe_out != 1)
            //     {
            //         printf("test out!!!!\n");
            //         dup2(fd_pipe_out, 1);
            //         close(fd_pipe_out);
            //     }
            // }

            if (strcmp(pcmd->errfile, "<undirected>") != 0)
            {
                if (pcmd->error_append == 1)
                {
                    fd_stderr = open(pcmd->errfile, O_WRONLY|O_APPEND);
                }
                else
                {
                    fd_stderr = open(pcmd->errfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                }
                dup2(fd_stderr,2);
                close(fd_stderr);
            }

            execv(command, argv);
            perror(command);
            _exit(1);
        }

        /// Check if child returned with error or not
        int status;
        waitpid(pid, &status, 0);
        if (DEBUG == 1) printf("exec status : %d\n", status);
        int exit_status = WEXITSTATUS(status);       
        if (DEBUG == 1) printf("Exit status of the child was %d\n", exit_status);
        if (exit_status == 1)
        {
            int temp = 1;
            *exec_fail = temp;
            if (DEBUG == 1) printf("exec_fail2 : %d\n", *exec_fail);
        }

        /// Move the pipe over one
        // fd_pipe1[0] = fd_pipe2[0];
        // fd_pipe1[1] = fd_pipe2[1];
        // fd_pipe2[0] = -1;
        // fd_pipe2[1] = -1;

        // printf("take2\n");
        // printf(" in : %d     out : %d\n", fd_pipe_in, fd_pipe_out);
        // printf("in2 : %d    out2 : %d\n", fd_pipe1[0], fd_pipe1[1]);
        // printf("in3 : %d    out3 : %d\n", fd_pipe2[0], fd_pipe2[1]);

        // if (pcmd->nextpipe != NULL)
        // {
        //     doline(pcmd->nextpipe, line_num, errorflag, DEBUG, exec_fail);
        // }
        // /// Reset global pipes
        // fd_pipe1[0] = 0;
        // fd_pipe1[1] = 1;

        if (DEBUG == 1)
        {
            PrintStuff(pcmd, line_num);
        }
    }
}

void PrintStuff(struct command *pcmd, int line_num){
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

void export(struct command *pcmd, char *argv[], int DEBUG){
    if (DEBUG == 1) printf("export : %d\n", pcmd->argc);
    if (DEBUG == 1) printf("export : %s\n", argv[pcmd->argc]);
    if (DEBUG == 1) printf("export : %zu\n", strlen(argv[pcmd->argc]));
    if (DEBUG == 1) fflush(stdout);   
    if (putenv(argv[pcmd->argc]) != 0)
    {
        perror("Couldn't export variable2");
    }
}