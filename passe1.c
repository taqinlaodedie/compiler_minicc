#include "passe1.h"
#include "defs.h"

void check_NODE_DECL(node_t node);
void check_NODE_FUNC(node_t node);
void check_NODE_IDENT(node_t node);
void check_NODE_AFFECT(node_t node);
void check_NODE_IF(node_t node);
void check_NODE_WHILE(node_t node);
void check_NODE_FOR(node_t node);
void check_NODE_NODE_DOWHILE(node_t node);
void check_NODE_DECLS(node_t node);
void check_arithOpr_b(node_t node);
void check_arithOpr_u(node_t node);
void check_logicalOpr_b(node_t node);
void check_logicalOpr_u(node_t node);
void push_stack(node_t node);
void pop_stack();
void search_tree(node_t node);
void verifs(node_t node);

void verifs(node_t node) {
  if(node) {
    yylineno = node->lineno;

    switch (node->nature) {
        case NODE_PROGRAM:
          push_global_context();
          break;

        case NODE_BLOCK:
          push_context();
          break;

        case NODE_FUNC:
          global=false;
          reset_env_current_offset();
          reset_temporary_max_offset();
          set_max_registers(nbReg);
          break;

        case NODE_DECLS:
          cur_type = node->opr[0]->type;
          break;

        case NODE_DECL:
          check_NODE_DECL(node);
          break;

        case NODE_IDENT:
          check_NODE_IDENT(node);
          break;

        case NODE_STRINGVAL:
          node->offset = add_string(node->str);
          break;
    }

    search_tree(node);

    switch (node->nature) {
        case NODE_BLOCK:
          pop_context();
        break;

        case NODE_FUNC:
          check_NODE_FUNC(node);
        break;

        case NODE_DECLS:
          check_NODE_DECLS(node);
          break;

        case NODE_AFFECT:
          check_NODE_AFFECT(node);
          pop_stack();
          break;

        case NODE_PLUS:
        case NODE_MUL:
        case NODE_DIV:
        case NODE_MINUS:
        case NODE_MOD:
        case NODE_BAND:
        case NODE_BOR:
        case NODE_BXOR:
        case NODE_SRL:
        case NODE_SRA:
        case NODE_SLL:
          check_arithOpr_b(node);
          push_stack(node);
          break;

        case NODE_AND:
        case NODE_OR:
        case NODE_LT:
        case NODE_GT:
        case NODE_GE:
        case NODE_LE:
        case NODE_EQ:
        case NODE_NE:
          check_logicalOpr_b(node);
          pop_stack();
          break;


        case NODE_NOT:
          check_logicalOpr_u(node);
          pop_stack();
          break;

        case NODE_BNOT:
        case NODE_UMINUS:
          check_arithOpr_u(node);
          push_stack(node);
          break;

        case NODE_IF:
          check_NODE_IF(node);
          break;

        case NODE_WHILE:
          check_NODE_WHILE(node);
          break;

        case NODE_FOR:
          check_NODE_FOR(node);
          break;

        case NODE_DOWHILE:
          check_NODE_NODE_DOWHILE(node);
          break;
    }
  }
}

// Declarer ou pas
void check_NODE_DECLS(node_t node) {
  if(node->opr[0]->type == TYPE_VOID) {
    sprintf(error_msg, "Unable to define variable of type \"void\"");
    yyerror(&node, error_msg);
  }
}

// Detecter si la variable est declarer
void check_NODE_DECL(node_t node) {
  int offest = env_add_element(node->opr[0]->ident, node->opr[0], 4);

  if(offest != -1) {
    node->opr[0]->offset = offest;
    node->opr[0]->type = cur_type;
    node->opr[0]->global_decl = global;
  }
  else {
    sprintf(error_msg, "The variable %s has been defined", node->opr[0]->ident);
    yyerror(&node, error_msg);
  }

  // global
  if(node->opr[1] != NULL) {
    if(node->opr[1]->nature == NODE_IDENT) {
        node_t res = (node_t)get_decl_node(node->opr[1]->ident);
        if(res != NULL) {
        if(cur_type != res->type) {
          sprintf(error_msg, "Type mismatch");
          yyerror(&node, error_msg);
        }
        node->opr[0]->value = res->value;
      }
    }
    else if(node->opr[1]->nature == NODE_INTVAL||node->opr[1]->nature == NODE_BOOLVAL) {
      if(cur_type != node->opr[1]->type) {
        sprintf(error_msg, "Type mismatch");
        yyerror(&node, error_msg);
      }
      node->opr[0]->value = node->opr[1]->value;
    }
    else if(node->opr[1]->nature == NODE_AFFECT) {
        if(cur_type!=node->opr[1]->opr[1]->type) {
          sprintf(error_msg, "Type mismatch");
          yyerror(&node, error_msg);
        }
    }
    else {
      if(cur_type != node->opr[1]->type) {
        sprintf(error_msg, "Type mismatch");
        yyerror(&node, error_msg);
      }
    }
  }

  else {
    if(node->opr[0]->global_decl){
      if(cur_type == TYPE_INT)
        node->opr[1] = make_node(NODE_INTVAL,0);
      else if(cur_type == TYPE_BOOL)
        node->opr[1] = make_node(NODE_BOOLVAL,0);
      node->opr[1]->value = 0;
      }
    }
}

// Tas et offset de main
void check_NODE_FUNC(node_t node){
  node->offset = get_env_current_offset();
  node->stack_size = node->offset + get_temporary_max_offset();
}

void push_stack(node_t node){
  if(node->opr[0]->nature == NODE_IDENT || node->opr[0]->nature == NODE_INTVAL
    || node->opr[0]->nature == NODE_BOOLVAL) {
    if(reg_available()) {
      push_temporary_virtual();
      nb_temporary++;
      release_reg();
      allocate_reg();
    }
    else{
      allocate_reg();
    }
  }
}

void pop_stack() {
  for(int i = 0; i < nb_temporary; i++) {
    pop_temporary_virtual();
  }

  nb_temporary = 0;

  while(get_current_reg() != NUM_ARCH_REGS) {
    release_reg();
  }
}

// Parcours une arbre
void search_tree(node_t node){
  if (node->nops > 0) {
    int i;
    for (i = 0; i < node->nops; i++) {
      verifs(node->opr[i]);
    }
  }
}

// Detecter si IDENT est declare
void check_NODE_IDENT(node_t node) {
  if (strcmp(node->ident, "main")!=0) {
    node_t res = (node_t)get_decl_node(node->ident);
    if (res == NULL) {
      sprintf(error_msg, "Variable '%s' not defined", node->ident);
      yyerror(&node, error_msg);
    }
    else {
      node->type = res->type;
      node-> decl_node = res;
    }
  }
  else{
    if(node->type!=TYPE_VOID){
      sprintf(error_msg, "Unable to define a variable named \"main\"");
      yyerror(&node, error_msg);
    }
  }
}


/*
* @summary:  Check if operands are of the same type
* @param[in]: node_t of nature NODE_PLUS, NODE_MINUS...
* @return: nothing
*/
void check_arithOpr_b(node_t node){
  if(node->opr[0]->type !=TYPE_INT || node->opr[1]->type !=TYPE_INT) {
    sprintf(error_msg, "Arithmetic operators type error.");
    yyerror(&node, error_msg);
  }
}

/*
* @summary: Check if opera are of the same type
* @param[in]: node_t of nature NODE_NOT...
* @return: nothing
*/
void check_logicalOpr_b(node_t node) {
  bool check=false;
  switch(node->opr[0]->type){
    case TYPE_INT:
      if(node->opr[1]->type==TYPE_INT)
        check=true;
      break;

    case TYPE_BOOL:
      if(node->opr[1]->type==TYPE_BOOL)
        check=true;
      break;
  }

  if(check==false){
    sprintf(error_msg, "Logical operators type error.");
    yyerror(&node, error_msg);
  }
}


void check_arithOpr_u(node_t node) {
  if(node->opr[0]->type!= TYPE_INT){
    sprintf(error_msg, "Operator type error.");
    yyerror(&node, error_msg);
  }
}

void check_logicalOpr_u(node_t node) {
  if(node->opr[0]->type!= TYPE_BOOL){
    sprintf(error_msg, "Operator type error.");
    yyerror(&node, error_msg);
  }
}

/*
* @summary:  Check if type in both sides of the equal are the same
* @param[in]: node_t of nature NODE_AFFECT
* @return: nothing
*/
void check_NODE_AFFECT(node_t node) {
  if(node->opr[1]->nature!=NODE_AFFECT) {
    if(node->opr[0]->type!=node->opr[1]->type) {
      sprintf(error_msg, "Type mismatch");
      yyerror(&node, error_msg);
    }
  }
  else {
    if(node->opr[0]->type!=node->opr[1]->opr[1]->type) {
      sprintf(error_msg, "Type mismatch");
      yyerror(&node, error_msg);
    }
  }
}

void check_NODE_IF(node_t node) {
  if(node->opr[0]->type!=TYPE_BOOL) {
    sprintf(error_msg, "The type of condition in IF is wrong");
    yyerror(&node, error_msg);
  }
}

void check_NODE_WHILE(node_t node) {
  if(node->opr[0]->type!=TYPE_BOOL) {
    sprintf(error_msg, "The type of condition in WHILE is wrong");
    yyerror(&node, error_msg);
  }
}

void check_NODE_FOR(node_t node) {
  if(node->opr[1]->type!=TYPE_BOOL) {
    sprintf(error_msg, "The type of condition in FOR is wrong");
    yyerror(&node, error_msg);
  }
}

void check_NODE_NODE_DOWHILE(node_t node) {
  if(node->opr[1]->type!=TYPE_BOOL) {
    sprintf(error_msg, "The type of condition in DOWHILE is wrong");
    yyerror(&node, error_msg);
  }
}
