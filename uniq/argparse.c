/* Ostermann's simple arg parsing example */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>


/* global flags set by command line arguments */
int aflag = 0;
int bflag = 0;
int cflag = 0;
char *file = NULL;
char *string = NULL;


/*
 * usage - A standard routine that I always use.  Prints an error message with
 * a variable number of arguments and then exits.
 */
static void
usage(
    char *progname,
    char *error,
    ...)
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


int
main(
    int argc,
    char *argv[])
{
    int i,j;

    /* parse the args */
    for (i=1; i < argc; ++i) {
	if (*argv[i] == '-') {
	    if (strcmp(argv[i],"-s") == 0) {
		/* the NEXT argument should be a string */
		if (i+1 >= argc)
		    usage(argv[0],"-s requires argument");
		string = argv[++i];
		continue;	/* iterate back to for (i=1... loop */
	    }

	    for (j=1; argv[i][j]; ++j) {
		switch (argv[i][j]) {
		  case 'a': ++aflag; break;
		  case 'b': ++bflag; break;
		  case 'c': ++cflag; break;
		  default:
		    usage(argv[0],"Illegal flag '%c'", argv[i][j]);
		}
	    }
	} else {		/* doesn't start with a '-' */
	    /* must be a file name argument (this needs error check) */
	    if (file)
		usage(argv[0],"Too many file names");
	    file = argv[i];
	}
    }

    if (!file)
	usage(argv[0],"I need a file name");

    printf("File name: '%s'\n", file);
    printf("string: '%s'\n", string?string:"<not supplied>");
    printf("aflag: %s\n", aflag?"on":"off");
    printf("bflag: %s\n", bflag?"on":"off");
    printf("cflag: %s\n", cflag?"on":"off");
}
