#ifndef _PASSE1_H_
#define _PASSE1_H_

#include "defs.h"
#include "common.h"
#include "utils/env.h"
#include "utils/registers.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


/* Global variables */
int node_num;
bool global;
char error_msg[NB_ELEM_ALPHABET];
int nb_temporary;
node_t root_tree;
int stack_size ;
node_type cur_type;
int nbReg;
extern int yylineno;

void verifs(node_t node);
extern node_t make_node(node_nature nature, int nops, ...);
extern void yyerror(node_t * program_root, char * s);

#endif
