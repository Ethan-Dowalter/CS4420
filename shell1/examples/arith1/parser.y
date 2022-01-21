%{
#include <stdio.h>
#include "calc.h"

/* some internal files generated by bison */
int yylex();
void yyerror(const char *s,...);
void yywarn(const char *s,...);


/* include debugging code, in case we want it */
#define YYDEBUG 1


/* structure for an operator chain */
struct opchain {
    int number;
    int operator;
    struct opchain *next;
};


%}


%union	{ /* the types that we use in the tokens */
    int number;
    int operator;
    struct assignment *pass;
    struct opchain *pop;
}



%token EOLN PLUS MINUS TIMES DIVIDE
%token <number> NUMBER  
  
%type <pass> expr 
%type <pop> rhs
%type <operator> oper
  

%% 	/* beginning of the parsing rules	*/
input	: lines
	|
  	;

lines	: oneline EOLN
	| oneline EOLN lines
	;

oneline : expr
		{ doline($1); }
	| error
	/* if we got an error on the line, don't call the C program */
	;

expr	: rhs
	  {
	      struct assignment *pass;
	      struct opchain *pop;

	      pass = (struct assignment *)
		  malloc(sizeof(struct assignment));
	      /* un-build the operator chain */
	      for (pop = $1; pop; pop = pop->next) {
		  pass->numbers[pass->nops] = pop->number;
		  pass->operators[pass->nops] = pop->operator;
		  ++pass->nops;
	      }
	      $$ = pass;
	  }


/* right hand side of an assignment */
rhs	: NUMBER
          {
	      $$ = (struct opchain *) malloc(sizeof(struct opchain));
	      $$->number = $1;
	  }
	| NUMBER oper rhs
          {
	      $$ = (struct opchain *) malloc(sizeof(struct opchain));
	      $$->operator = $2;
	      $$->number = $1;
	      $$->next = $3;
	  }
	;


/* one of the 4 operators we understand */
oper	: PLUS		{ $$ = PLUS;}
	| MINUS		{ $$ = MINUS;}
	| TIMES		{ $$ = TIMES;}
	| DIVIDE	{ $$ = DIVIDE;}
	;

%%

void yyerror(const char *s,...)
{
	fprintf(stderr, "parser: Bad syntax (%s)\n", s);
}
