%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "defs.h"
#include "common.h"
#include "mips_inst.h"

#include "passe1.h"
#include "passe2.h"

/* Global variables */
/* A completer */
extern bool stop_after_syntax;
extern bool stop_after_verif;
extern char * infile;
extern char * outfile;

/* prototypes */
int yylex(void);
extern int yylineno;

void yyerror(node_t * program_root, char * s);
void analyse_tree(node_t root);
node_t make_node(node_nature nature, int nops, ...);
/* A completer */

%}

%parse-param { node_t * program_root }

%union {
    int32_t intval;
    char * strval;
    node_t ptr;
};


/* Definir les token ici avec leur associativite, dans le bon ordre */
%token TOK_VOID     TOK_INT     TOK_INTVAL  TOK_BOOL    TOK_TRUE    TOK_FALSE
%token TOK_IDENT    TOK_IF      TOK_ELSE    TOK_WHILE   TOK_FOR     TOK_PRINT
%token TOK_AFFECT   TOK_GE      TOK_LE      TOK_GT      TOK_LT      TOK_EQ
%token TOK_NE       TOK_PLUS    TOK_MINUS   TOK_MUL     TOK_DIV     TOK_MOD
%token TOK_UMINUS   TOK_SEMICOL TOK_COMMA   TOK_LPAR    TOK_RPAR    TOK_LACC
%token TOK_RACC     TOK_STRING  TOK_DO

%nonassoc TOK_THEN
%nonassoc TOK_ELSE

%right TOK_AFFECT
%left TOK_OR
%left TOK_AND
%left TOK_BOR
%left TOK_BXOR
%left TOK_BAND
%left TOK_EQ TOK_NE
%left TOK_GT TOK_LT TOK_GE TOK_LE
%left TOK_SRL TOK_SRA TOK_SLL

%left TOK_PLUS TOK_MINUS
%left TOK_MUL TOK_DIV TOK_MOD

%left TOK_UMINUS TOK_NOT TOK_BNOT

%type <intval> TOK_INTVAL
%type <strval> TOK_IDENT TOK_STRING

%type <ptr> program listdecl listdeclnonnull vardecl ident type listtypedecl decl maindecl
%type <ptr> listinst listinstnonnul inst block expr listparamprint paramprint
%%

/* Regles ici */
program:
       listdeclnonnull maindecl
        {
            $$ = make_node(NODE_PROGRAM, 2, $1, $2);
            *program_root = $$;
        }
        | maindecl
        {
            $$ = make_node(NODE_PROGRAM, 2, NULL, $1);
            *program_root = $$;
        }
        ;


listdecl: listdeclnonnull
          {
          }
          |
          {
            $$ = NULL;
          }
          ;

listdeclnonnull: vardecl
               {
               }
               | listdeclnonnull vardecl
               {
                 $$ = make_node(NODE_LIST, 2, $1, $2);
               }
               ;

vardecl: type listtypedecl TOK_SEMICOL
       {
         $$ = make_node(NODE_DECLS, 2, $1, $2);
       } ;

type      : TOK_INT
          {
            $$ = make_node(NODE_TYPE, 0);
            $$->type = TYPE_INT;
          }
          | TOK_BOOL
          {
            $$ = make_node(NODE_TYPE, 0);
            $$->type = TYPE_BOOL;
          }
          | TOK_VOID
          {
            $$ = make_node(NODE_TYPE, 0);
            $$->type = TYPE_VOID;
          }
          ;

listtypedecl: listtypedecl TOK_COMMA decl
            {
              $$ = make_node(NODE_LIST, 2, $1, $3);
            }
            |decl


decl      : ident
          {
            $$ = make_node(NODE_DECL, 2, $1, NULL);
          }
          | ident TOK_AFFECT expr
          {
            $$ = make_node(NODE_DECL, 2, $1, $3);
          }
          ;

maindecl  : type ident TOK_LPAR TOK_RPAR block
          {
          //  global = false;
            $$ = make_node(NODE_FUNC, 3, $1, $2, $5);
            $2->type = TYPE_VOID;
          };

listinst  : listinstnonnul
          |
          {
            $$ = NULL;
          }
          ;

listinstnonnul: inst
              | listinstnonnul inst
              {
                $$ = make_node(NODE_LIST, 2, $1, $2);
              }
              ;

inst      : expr TOK_SEMICOL
          | TOK_IF TOK_LPAR expr TOK_RPAR inst TOK_ELSE inst
          {
            $$ = make_node(NODE_IF, 3, $3, $5,$7);
          }
          | TOK_IF TOK_LPAR expr TOK_RPAR inst %prec TOK_THEN
          {
            $$ = make_node(NODE_IF, 2, $3, $5);
          }
          | TOK_WHILE TOK_LPAR expr TOK_RPAR inst
          {
            $$ = make_node(NODE_WHILE, 2, $3, $5);
          }
          | TOK_FOR TOK_LPAR expr TOK_SEMICOL expr TOK_SEMICOL expr TOK_RPAR inst
          {
            $$ = make_node(NODE_FOR, 4, $3, $5, $7, $9);
          }
          | TOK_DO inst TOK_WHILE TOK_LPAR expr TOK_RPAR TOK_SEMICOL
          {
            $$ = make_node(NODE_DOWHILE, 2, $2, $5);
          }
          | block
          | TOK_SEMICOL
          {
          }
          | TOK_PRINT TOK_LPAR listparamprint TOK_RPAR TOK_SEMICOL
          {
            $$ = make_node(NODE_PRINT, 1, $3);
          }
          ;

block     : TOK_LACC listdecl listinst TOK_RACC
          {
            $$ = make_node(NODE_BLOCK, 2, $2, $3);
          }
          ;

expr      : expr TOK_MUL expr
          {
            $$ = make_node(NODE_MUL, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_DIV expr
          {
            $$ = make_node(NODE_DIV, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_PLUS expr
          {
            $$ = make_node(NODE_PLUS, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_MINUS expr
          {
            $$ = make_node(NODE_MINUS, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_MOD expr
          {
            $$ = make_node(NODE_MOD, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_LT expr
          {
            $$ = make_node(NODE_LT, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_GT expr
          {
            $$ = make_node(NODE_GT, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | TOK_MINUS expr %prec TOK_UMINUS
          {
            $$ = make_node(NODE_UMINUS, 1, $2);
            $$->type = TYPE_INT;
          }
          | expr TOK_GE expr
          {
            $$ = make_node(NODE_GE, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_LE expr
          {
            $$ = make_node(NODE_LE, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_EQ expr
          {
            $$ = make_node(NODE_EQ, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_NE expr
          {
            $$ = make_node(NODE_NE, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_AND expr
          {
            $$ = make_node(NODE_AND, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_OR expr
          {
            $$ = make_node(NODE_OR, 2, $1, $3);
            $$->type = TYPE_BOOL;
          }
          | expr TOK_BAND expr
          {
            $$ = make_node(NODE_BAND, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_BOR expr
          {
            $$ = make_node(NODE_BOR, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_BXOR expr
          {
            $$ = make_node(NODE_BXOR, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_SRL expr
          {
            $$ = make_node(NODE_SRL, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_SRA expr
          {
            $$ = make_node(NODE_SRA, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | expr TOK_SLL expr
          {
            $$ = make_node(NODE_SLL, 2, $1, $3);
            $$->type = TYPE_INT;
          }
          | TOK_NOT expr
          {
            $$ = make_node(NODE_NOT, 1, $2);
            $$->type = TYPE_BOOL;
          }
          | TOK_BNOT expr
          {
            $$ = make_node(NODE_BNOT, 1, $2);
            $$->type = TYPE_INT;
          }
          | TOK_LPAR expr TOK_RPAR
          {
              $$=$2;
          }
          | ident TOK_AFFECT expr
          {
            $$ = make_node(NODE_AFFECT, 2, $1, $3);
          }
          | TOK_INTVAL
          {
            $$ = make_node(NODE_INTVAL, 0);
            $$->value = yylval.intval;
            $$->type = TYPE_INT;
          }
          | TOK_TRUE
          {
            $$ = make_node(NODE_BOOLVAL, 0);
            $$->value = 1;
            $$->type = TYPE_BOOL;
          }
          | TOK_FALSE
          {
            $$ = make_node(NODE_BOOLVAL, 0);
            $$->value = 0;
            $$->type = TYPE_BOOL;
          }
          | ident
          ;

listparamprint : listparamprint TOK_COMMA paramprint
                {
                  $$ = make_node(NODE_LIST, 2, $1, $3);
                }
               | paramprint
               {
               }
               ;

paramprint: ident
          | TOK_STRING
          {
            $$ = make_node(NODE_STRINGVAL, 0);
            $$->str = yylval.strval;
          }
          ;


ident     : TOK_IDENT
          {
            $$ = make_node(NODE_IDENT, 0);
            $$->ident = yylval.strval;
          }
          ;
%%

/* A completer et/ou remplacer avec d'autres fonctions */
node_t make_node(node_nature nature, int nops, ...) {
    va_list ap;
    node_t ret;

    ret = malloc(sizeof(node_s));
    ret->nature = nature;
    ret->nops = nops;
    ret->lineno = yylineno;
    ret->type = TYPE_NONE;
    ret->global_decl = global;
    ret->node_num = node_num;
    ret->stack_size = 0;

    node_num++;

    if(!nops) {
        ret->opr = NULL;
        return ret;
    }

    ret->opr = (node_t *)malloc(nops * sizeof(node_t));
    va_start(ap, nops);
    for(int i = 0; i < nops; i++) {
        ret->opr[i] = va_arg(ap, node_t);
    }
    va_end(ap);

    return ret;
}



/* A completer */
void analyse_tree(node_t root) {
    if (!stop_after_syntax) {
        // Appeler la passe 1
        verifs(root);
        if (!stop_after_verif) {
            create_program(); 
            // Appeler la passe 2
            asm_gen(root);
            dump_mips_program(outfile);
            free_program();
        }
        free_global_strings();
    }
}



void yyerror(node_t * program_root, char * s) {
    fprintf(stderr, "Error line %d: %s\n", yylineno, s);
    exit(1);
}

