/*
 * Simple arithmetic program as a lex/yacc example
 *
 * Shawn Ostermann -- Wed Feb  7, 1996
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "arith.h"

/* globals */


/* local routines */
static void doexpr(struct assignment *pass);



int
main(
     int argc,
     char *argv[])
{
    yydebug = 1;  /* turn on ugly YACC debugging */
    yydebug = 0;  /* turn off ugly YACC debugging */

    /* parse the input file */
    yyparse();

    exit(0);
}


static void
doexpr(
    struct assignment *pass)
{
    int i;
    int sum;
    int nextterm;

    printf("  Number of operations: %d\n", pass->nops);
    printf("  Question: '%s =", pass->variable);
    sum = pass->numbers[0];
    for (i=0; i < pass->nops; ++i) {
	printf(" %d", pass->numbers[i]);
	if (i+1 < pass->nops) {
	    nextterm = pass->numbers[i+1];
	    switch(pass->operators[i]) {
	      case PLUS:    printf(" +"); sum += nextterm; break;
	      case MINUS:   printf(" -"); sum -= nextterm; break;
	      case TIMES:   printf(" *"); sum *= nextterm; break;
	      case DIVIDE:  printf(" /"); sum /= nextterm; break;
	      default:  printf("? ");
	    }
	}
    }
    printf("'\n");
    printf("  answer is %d\n\n", sum);
}


void
doline(
    struct assignment *pass)
{
    printf("Read a line:\n");

    for (; pass; pass = pass->next) {
	doexpr(pass);
    }
}