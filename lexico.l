%{
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "defs.h"
#include "common.h"

#include "y.tab.h"

void yyerror(char *);
int yyparse(node_t * program_root);
int fileno(FILE * stream);
void analyse_tree(node_t root);


char * infile = NULL;
char * outfile = "out.s";
bool stop_after_syntax = false;
bool stop_after_verif = false;
int trace_level = 0;
int nb_reg = 0;
/* A completer */

#if YYDEBUG
extern int yydebug;
#endif


%}

%option yylineno


LETTRE          [a-zA-Z]
/* A completer ; fausses definitions pour que lex ne fasse pas d'erreur */
CHIFFRE         [0-9]
IDF             {LETTRE}({LETTRE}|{CHIFFRE}|"_")*
ENTIER          {CHIFFRE}*
CAR             ([0x20-0x21]|[0x23-0x5b]|[0x5d-0x79])
CHAINE          "'"({IDF}|"\n")*"'"
COMMENTAIRE     "//".*$
%%

"void"          return TOK_VOID;
"int"           return TOK_INT;
"bool"          return TOK_BOOL;
"true"          return TOK_TRUE;
"false"         return TOK_FALSE;
"if"            return TOK_IF;
"else"          return TOK_ELSE;
"while"         return TOK_WHILE;
"for"           return TOK_FOR;
"print"         return TOK_PRINT;
"="             return TOK_AFFECT;
">="            return TOK_GE;
"<="            return TOK_LE;
">"             return TOK_GT;
"<"             return TOK_LT;
"=="            return TOK_EQ;
"!="            return TOK_NE;
"+"             return TOK_PLUS;
"-"             return TOK_MINUS;
"*"             return TOK_MUL;
"/"             return TOK_DIV;
"%"             return TOK_MOD;
";"             return TOK_SEMICOL;
","             return TOK_COMMA;
"("             return TOK_LPAR;
")"             return TOK_RPAR;
"{"             return TOK_LACC;
"}"             return TOK_RACC;
"do"            return TOK_DO;
"&&"            return TOK_AND;
"&"             return TOK_BAND;
"||"            return TOK_OR;
"|"             return TOK_BOR;
"^"             return TOK_BXOR;
"<<<"           return TOK_SLL;
">>>"           return TOK_SRL;
">>"            return TOK_SRA;
"!"             return TOK_NOT;
"~"             return TOK_BNOT;


{IDF} {
                yylval.strval = strdup(yytext);
                return TOK_IDENT;
}

{ENTIER} {
                /* A completer : affecter yylval.intval et faire des verifications */     
                if(strlen(yytext) < 10){}
                else {
                	if(strcmp("4294967295", yytext) >= 0){}
                	else{
                		fprintf(stderr, "Error line %d: to large int\n", yylineno);
                	}
                }           
                yylval.intval = atoi(yytext);
                return TOK_INTVAL;
}

{CHAINE} {
                /* A completer : affecter yylval.strval */
                yylval.strval = strdup(yytext);
                return TOK_STRING;
}

{COMMENTAIRE} {
}


[ \t\n]+

. {
                fprintf(stderr, "Error line %d: Lexical error\n", yylineno);
                exit(1);
}


%%

int yywrap(void) {
    return 1;
}


int main(int argc, char ** argv) {
    /* A completer */
    node_t program_root;
    parse_args(argc, argv);
    #if YYDEBUG
    yydebug = 1;
    #endif
    
    yyin = fopen(infile, "r");
    yyparse(&program_root);
    fclose(yyin);
    analyse_tree(program_root);
    //dump_tree(program_root, "out.dot");

    yylex_destroy();
    return 0;
}




