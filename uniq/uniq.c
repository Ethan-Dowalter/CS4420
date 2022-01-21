/**
 *        @file: uniq.c
 *      @author: Ethan Dowalter
 *        @date: September 3, 2021
 *       @brief: A replica of Linux's uniq command using only system calls
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>

/* global flags set by command line arguments */
int cflag = 0;
int dflag = 0;
int iflag = 0;
char *file = NULL;
char *string = NULL;

/* global flag to see if at end of file */
int eof = 0;


///My subroutines

/*
 *  getChar - returns a char at a given index in a file
 */
char getChar(int fd, int index);

/*
 *  nextLine - returns index of the start of the next line in a file
 */
int nextLine(int fd, int index);

/*
 *  printLine - prints a line and how many times it appears starting at index from a file to standard output
 */
void printLine(int fd, int index, int count);


/*
 * usage - A standard routine that I always use.  Prints an error message with
 * a variable number of arguments and then exits.
 */
static void usage(char *progname, char *error, ...)
{
    va_list ap;
    
    va_start(ap, error);
    if (error) {
	vfprintf(stderr,error,ap);
	fprintf(stderr,"\n");
    }
    va_end(ap);

    fprintf(stderr,"usage: %s [-[abc]] [-s string] file\n", progname);

    exit(-1);
}


///Main


int main(int argc, char *argv[])
{
    int i, j;

    /* parse the args */
    for (i=1; i < argc; ++i)
    {
        if (*argv[i] == '-')
        {
            if (strcmp(argv[i],"-s") == 0)
            {
                /* the NEXT argument should be a string */
                if (i+1 >= argc)
                    usage(argv[0],"-s requires argument");
                string = argv[++i];
                continue;	/* iterate back to for (i=1... loop */
            }

            for (j=1; argv[i][j]; ++j)
            {
                switch (argv[i][j])
                {
                    case 'c': ++cflag; break;
                    case 'd': ++dflag; break;
                    case 'i': ++iflag; break;
                    default:
                        usage(argv[0],"Illegal flag '%c'", argv[i][j]);
                }
            }
        }
        else
        {	/* doesn't start with a '-' */
            /* must be a file name argument (this needs error check) */
            if (file) usage(argv[0],"Too many file names");
            file = argv[i];
        }
    }

    if (!file) usage(argv[0],"I need a file name");

    ///Debugging
    if (dflag > 0)
    {
        printf("File name: '%s'\n", file);
        printf("string: '%s'\n", string?string:"<not supplied>");
        printf("cflag: %s\n", cflag?"on":"off");
        printf("dflag: %s\n", dflag?"on":"off");
        printf("iflag: %s\n", iflag?"on":"off");
    }

    ///Open the file
    int fd = open(file, O_RDONLY);
    if (dflag > 0) printf("filedesc : %d\n", fd);
    if (fd == -1)
    {
        perror("File doesn't exist!!");
        exit(-1);
    }
    
    ///Initialize the indices
    int index1 = 0;                             //variable to track index in the first line for comparisons
    int index2 = nextLine(fd, index1);          //variable to track index in the second line for comparisons
    int indexLastUniqueLine = 0;                //variable to track index of the last seen unique line

    ///Debugging
    if (dflag > 0)
    {
        printf("index1: %d\n", index1);
        printf("index2: %d\n", index2);
        // printLine(fd, index1);
        // printLine(fd, index2);
        // printLine(fd, nextLine(fd, index2));
    }

    ///Do the thing (main loop of program for comparing chars on consecutive lines)
    int count = 1;              //variable to count number of repeated lines for -c option
    while(index2 != -1)         // -1 means it got to the end of the file
    {
        while (getChar(fd, index1) == getChar(fd, index2) && !eof)
        {
            index1++;
            index2++;
            if (cflag > 0 && getChar(fd, index1) == '\n' && getChar(fd, index2) == '\n')
            {
                count++;        //make sure to keep count for when cflag is on
            }
            
        }

        ///Debugging
        if (dflag > 0) printf("%d %d %d\n", index1, index2, indexLastUniqueLine);
        
        ///Once we run into a mismatch, print the line, move the indices down, and reset the count
        printLine(fd, indexLastUniqueLine, count);
        index1 = nextLine(fd, index1);
        index2 = nextLine(fd, index2);
        indexLastUniqueLine = index1;
        count = 1;
    }

    ///Debugging
    if (dflag > 0) printf("\n%d %d %d\n\n", index1, index2, indexLastUniqueLine);

    ///Print the last line if necessary
    if (index1 != -1)
    {
        printLine(fd, indexLastUniqueLine, count);
    }

    ///Close the file
    if (close(fd) < 0)
    {
        perror("\nExit failure. Could not close file.\n\n");
        exit(1);        //exit failure!
    }

    if (dflag > 0) printf("\nSuccess\n\n");

    exit(0);            //exit success!
}


///Function definitions


char getChar(int fd, int index)
{
    char ch;                                    //for reading in characters, one at a time
    int len = lseek(fd, index, SEEK_SET);       //move the fd to the index
    if (len == index - 1)                       //check for errors
    {
        perror("Help! I couldn't move the index properly!! (1)");
        exit(-1);
    }

    len = read(fd, &ch, 1);     //read the char at the index
    if (len == -1)              // -1 means it couldn't read
    {
        perror("Can't read!!!");
        exit(-1);
    }
    if (len == 0)               // 0 means end of file
    {
        // perror("Index for character is not within the file!!!");
        eof = 1;                //Set end of file flag
        return 127;             //ASCII 127 is the delete code,
                                //which I think is safe to use as a special case since it could never be in a file???
                                //I tried making the function a void so I wouldn't have to return a char in this case,
                                //but I had a lot of trouble trying to figure out how to pass a char by reference.
                                //So this is the solution I came up with: using a character that could never be read from a file
    }

    if (iflag > 0)              //case insensitive option for -i
    {
        ch = toupper(ch);
    }

    return ch;           
}

int nextLine(int fd, int index)
{
    int newLineIndex = index;                   //initialize to index, then increment until new line is found
    int len = lseek(fd, index, SEEK_SET);       //move the fd to the index
    if (len == index - 1)                       //check for errors
    {
        perror("Help! I couldn't move the index properly!! (2)");
        exit(-1);
    }

    char ch;                            //for reading in characters, one at a time
    while(1)
    {
        ///Read one char at a time until error, end of file, or new line
        len = read(fd, &ch, 1);         //read the char
        if (len == -1)                  // -1 means it couldn't read
        {
            perror("Can't read!!!");
            exit(-1);
        }
        if (len == 0) return -1;        // 0 means end of file, so return -1 for special case

        newLineIndex++;                 //increment index every read

        if (ch == '\n')                 //once end of line is reached, add 1 to get to start of next line and return
        {
            ///Do another read to make sure not end of file
            len = read(fd, &ch, 1);
            if (len == -1)              // -1 means it couldn't read
            {
                perror("Can't read!!!");
                exit(-1);
            }
            if (len == 0) return -1;    // 0 means end of file, so return -1 for special case

            return newLineIndex++;      // Normal case
        }
    }
}

void printLine(int fd, int index, int count)
{
    int len = lseek(fd, index, SEEK_SET);       //move the fd to the index
    if (len == index - 1)                       //check for errors
    {
        perror("Help! I couldn't move the index properly!! (3)");
        exit(-1);
    }
    
    char ch = '\t';                 //initialize ch first;
    
    ///Special case for cflag
    if (cflag > 0)                  //cflag to include number of lines
    {
        if (write(1, &ch, 1) != 1)
        {
            perror("Can't write!!!");
            exit(-1);
        }
        char str[12] = "";
        sprintf(str, "%d", count);
        if (write(1, str, sizeof(str) - 1) == -1)
        {
            perror("Can't write!!!!!!");
            exit(-1);
        }
        ch = ' ';
        if (write(1, &ch, 1) != 1)
        {
            perror("Can't write!!!");
            exit(-1);
        }
    }    

    ///Print until end of line
    while (ch != '\n')
    {
        len = read(fd, &ch, 1);     //read the char
        if (len == -1)              // -1 means it couldn't read
        {
            perror("Can't read!!!");
            exit(-1);
        }
        if (len == 0) return;       // 0 means end of file, so we're done

        ///Print char to screen
        if (write(1, &ch, 1) != 1)
        {
            perror("Can't write!!!");
            exit(-1);
        }
    }
}