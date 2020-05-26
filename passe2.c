#include "passe2.h"

int print_flag;
int if_flag;
int if_end;
int if_else_flag;
int for_flag;
int for_start;
int for_end;
int while_flag;
int while_start;
int while_end;
int dowhile_flag;
int dowhile_start;
int dowhile_end;
int num_block = 1;
int num_block_if = 0;
char label[100][20];

void asm_gen(node_t node);
void asm_exit(node_t node);
void asm_search_tree(node_t node);
void asm_NODE_FUNC(node_t node);
void asm_NODE_AFFECT(node_t node,int s);
void asm_NODE_DECL(node_t node,int s);
void asm_NODE_IF(node_t node,int s);
void asm_NODE_IDENT(node_t node,int s);
void asm_NODE_STRINGVAL(node_t node,int s);
void asm_NODE_FOR(node_t node,int s);
void asm_NODE_WHILE(node_t node,int s);
void asm_NODE_DOWHILE(node_t node,int s);
void asm_NODE_arithOpr(node_t node,int s);
void asm_NODE_arithOpr_u(node_t node,int s);
void asm_NODE_logicalOpr(node_t node,int s);
static void asm_switch_arithOpr(node_t node, int32_t num_reg_1, int32_t num_reg_2);
static void asm_switch_arithOpr_u(node_t node, int32_t num_reg_1);
static void asm_switch_logicalOpr(node_t node, int32_t num_reg_1, int32_t num_reg_2);
static void asm_global_local(node_t node, int32_t num_reg_1, int s);
static void asm_condition_instruction();

//generer du code asm
void asm_gen(node_t node) {
  if (node) {
    switch (node->nature) {

      case NODE_PROGRAM:
        create_data_sec_inst();
        break;

      case NODE_FUNC:
        asm_NODE_FUNC(node);
        break;

      case NODE_DECL:
      asm_NODE_DECL(node,0);
        break;

      case NODE_IDENT:
        asm_NODE_IDENT(node,0);
        break;

      case NODE_PRINT:
        print_flag = 1;
        break;

      case NODE_STRINGVAL:
        asm_NODE_STRINGVAL(node,0);
        break;

      case NODE_IF:
        asm_NODE_IF(node,0);
        break;

      case NODE_BLOCK:
        if(if_else_flag==1){
          num_block_if++;
          if(num_block_if==2)
            create_label_inst(if_end);
        }
        break;

      case NODE_FOR:
        asm_NODE_FOR(node,0);
        break;

      case NODE_WHILE:
        asm_NODE_WHILE(node,0);
        break;

      case NODE_DOWHILE:
        asm_NODE_DOWHILE(node,0);
        break;

      case NODE_AFFECT:
        asm_NODE_AFFECT(node,0);
        break;

      case NODE_AND:
      case NODE_OR:
      case NODE_LT:
      case NODE_GT:
      case NODE_GE:
      case NODE_LE:
      case NODE_EQ:
      case NODE_NE:
        asm_NODE_logicalOpr(node,0);
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
        asm_NODE_arithOpr(node,0);
        break;

      case NODE_UMINUS:
      case NODE_BNOT:
      case NODE_NOT:
        asm_NODE_arithOpr_u(node,0);
        break;
    }
    
    asm_search_tree(node);

    switch (node->nature) {

      case NODE_PROGRAM:
        asm_exit(node);
        break;


      case NODE_DECL:
        asm_NODE_DECL(node,1);
        break;

      case NODE_PRINT:
        print_flag = 0;
        break;

      case NODE_IF:
        asm_NODE_IF(node,1);
        break;

      case NODE_FOR:
        asm_NODE_FOR(node,1);
        break;

      case NODE_WHILE:
        asm_NODE_WHILE(node,1);
        break;
      case NODE_DOWHILE:
        asm_NODE_DOWHILE(node,1);
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
        asm_NODE_arithOpr(node,1);
        break;

      case NODE_AND:
      case NODE_OR:
      case NODE_LT:
      case NODE_GT:
      case NODE_GE:
      case NODE_LE:
      case NODE_EQ:
      case NODE_NE:
        asm_NODE_logicalOpr(node,1);
        break;

      case NODE_UMINUS:
      case NODE_BNOT:
      case NODE_NOT:
        asm_NODE_arithOpr_u(node,1);
        break;

      case NODE_AFFECT:
        asm_NODE_AFFECT(node,1);
        break;
    }
  }
}

//tete du programme main
void asm_NODE_FUNC(node_t node){
  int nb = get_global_strings_number();

  for (int i = 0; i < nb; i++) {
    sprintf(label[i], "_str%d", i);
    create_asciiz_inst(label[i], get_global_string(i));
  }
  create_text_sec_inst();
  create_label_str_inst("main");
  create_addiu_inst(29, 29,-node->offset);
}

//sortir du programme
void asm_exit(node_t node){
  create_addiu_inst(29, 29, node->opr[1]->offset);
  create_ori_inst(2, r0, 10);
  create_syscall_inst();
}

//declarations
void asm_NODE_DECL(node_t node, int s) {
  if(!s) {
    if (node->opr[0]->global_decl) {
      if(node->opr[1]->nature == NODE_IDENT)
        create_word_inst(node->opr[0]->ident, node->opr[1]->decl_node->value);
      else
        create_word_inst(node->opr[0]->ident, node->opr[1]->value);
    }
    else {
      if(node->opr[1] != NULL) {
        if(node->opr[1]->nature == NODE_IDENT) {
          create_lui_inst(get_current_reg(), data_start);
          create_lw_inst(get_current_reg(), node->opr[1]->decl_node->offset, get_current_reg());
          create_sw_inst(get_current_reg(), node->opr[0]->decl_node->offset, 29);
        }
        else if(node->opr[1]->nature == NODE_BOOLVAL || node->opr[1]->nature == NODE_INTVAL) {
          create_ori_inst(get_current_reg(), 0, node->opr[1]->value );
          create_sw_inst(get_current_reg(), node->opr[0]->offset, 29);
        }
      }
      else {
        create_sw_inst(29, node->opr[0]->offset, 29);
      }
    }
  }

  if(s == 1) {
    if(node->opr[1] != NULL) {
      switch(node->opr[1]->nature) {
        case NODE_IDENT:
        case NODE_BOOLVAL:
        case NODE_INTVAL:  
          break;
        default:
          create_sw_inst(get_current_reg(),node->opr[0]->offset, 29);
          break;
      }
    }
  }
}

//parcours l'arbre
void asm_search_tree(node_t node) {
  if(node->nops > 0 && node->nature != NODE_IDENT) {
    for(int i = 0; i < node->nops; i++) {
      if(node->nature != NODE_FOR || (node->nature == NODE_FOR && i != 2))
        asm_gen(node->opr[i]);
    }
  }
}


void asm_NODE_IDENT(node_t node,int s) {
  if(!s) {
    if(print_flag) {
      if(node->decl_node->global_decl == true) {
        create_lui_inst(4, data_start);
        create_lw_inst(4, node->decl_node->offset, 4);
        create_ori_inst(2, r0, 1);
        create_syscall_inst();
      }
      else {
        create_lw_inst(4, node->decl_node->offset, 29);
        create_ori_inst(2, r0, 1);
        create_syscall_inst();
      }
    }
  }
}

void asm_NODE_STRINGVAL(node_t node, int s) {
  if(!s) {
    if(print_flag) {
      create_lui_inst(4, data_start);
      create_ori_inst(4, 4, node->offset);
      create_ori_inst(2, r0, 4);
      create_syscall_inst();
    }
  }
}

void asm_NODE_IF(node_t node,int s) {
  if(!s) {
    if_flag=1;
    if_else_flag=1;
    if_end=num_block;
    num_block++;
  }
  if(s == 1) {
    if(num_block_if == 1)
      create_label_inst(if_end);
    num_block_if = 0;
  }
}

void asm_NODE_FOR(node_t node,int s) {
  if(!s) {
    for_flag = 1;
    for_start = num_block;
    num_block++;
    for_end = num_block;
    num_block++;
  }
  if(s == 1) {
    asm_NODE_arithOpr(node->opr[2]->opr[1],0);
    asm_NODE_AFFECT(node->opr[2],1);
    create_j_inst(for_start);
    create_label_inst(for_end);
  }
}

void asm_NODE_WHILE(node_t node,int s){
  if(!s) {
    while_flag=1;
    while_start = num_block;
    num_block++;
    while_end=num_block;
    num_block++;
  }
  if(s == 1) { 
    create_j_inst(while_start);
    create_label_inst(while_end);
  }
}

void asm_NODE_DOWHILE(node_t node,int s){
  if(!s) {
    dowhile_flag = 1;
    dowhile_start = num_block;
    num_block++;
    dowhile_end = num_block;
    num_block++;
    if(dowhile_flag == 1) { 
      create_label_inst(dowhile_start);
    }
  }
  if(s == 1) {
    create_j_inst(dowhile_start);
    create_label_inst(dowhile_end);
  }
}

void asm_NODE_AFFECT(node_t node,int s) {
  if(!s){
    if(node->opr[1]->nature == NODE_IDENT) {
      //ident
      create_lw_inst(get_current_reg(), node->opr[1]->decl_node->offset, 29);
      if(node->opr[0]->decl_node->global_decl==true) {
        int32_t num_reg_1=get_current_reg();
        allocate_reg();
        create_lui_inst(get_current_reg(), data_start);
        create_sw_inst(num_reg_1,node->opr[0]->decl_node->offset, get_current_reg());
        release_reg();
      }
      else
        create_sw_inst(get_current_reg(),node->opr[0]->offset, 29);
    }
    else if(node->opr[1]->nature == NODE_INTVAL ||node->opr[1]->nature == NODE_BOOLVAL ) {
      create_ori_inst(get_current_reg(),0, node->opr[1]->value);
      if(node->opr[0]->decl_node->global_decl == true) {
        int32_t num_reg_1 = get_current_reg();
        allocate_reg();
        create_lui_inst(get_current_reg(), data_start);
        create_sw_inst(num_reg_1,node->opr[0]->decl_node->offset, get_current_reg());
        release_reg();
      }
      else {
        create_sw_inst(get_current_reg(),node->opr[0]->decl_node->offset, 29);
      }
    }
    if(for_flag == 1){
      create_label_inst(for_start);
    }
  }

  else {
    if(node->opr[1]->nature != NODE_IDENT 
      && node->opr[1]->nature != NODE_INTVAL 
      && node->opr[1]->nature != NODE_BOOLVAL){
      if(node->opr[0]->decl_node->global_decl) {
        int32_t num_reg_1 = get_current_reg();
        allocate_reg();
        create_lui_inst(get_current_reg(), data_start);
        create_sw_inst(num_reg_1,node->opr[0]->decl_node->offset, get_current_reg());
        release_reg();
      }
      else {
        create_sw_inst(get_current_reg(),node->opr[0]->decl_node->offset, 29);
      }
    }
  }
}

void asm_NODE_arithOpr(node_t node,int s) {
  int32_t num_reg_1;
  int32_t num_reg_2;
  if(s == 0) {
    //ident+int ident+ident int+ident int+int
    if(node->opr[0]->nature == NODE_IDENT) {
      num_reg_1 = get_current_reg();

      if(node->opr[1]->nature == NODE_IDENT) {
        //ident+ident
        //si var est global
        asm_global_local(node, num_reg_1, 0);
        allocate_reg();
        num_reg_2 = get_current_reg();
        release_reg();
        asm_global_local(node, num_reg_2, 1);
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }

      else if(node->opr[1]->nature == NODE_INTVAL) {
        //ident+int
        asm_global_local(node, num_reg_1, 0);
        allocate_reg();
        num_reg_2 = get_current_reg();
        create_ori_inst(num_reg_2,0, node->opr[1]->value);
        release_reg();
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
    }
    else if(node->opr[0]->nature == NODE_INTVAL) {
      num_reg_1 = get_current_reg();
      if(node->opr[1]->nature == NODE_IDENT){
        //int+ident
        create_ori_inst(num_reg_1,0, node->opr[0]->value);
        //reg_available()
        allocate_reg();
        num_reg_2 = get_current_reg();
        release_reg();
        //si var est global
        asm_global_local(node, num_reg_2, 1);
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
      else if(node->opr[1]->nature == NODE_INTVAL) {
        //int+int
        create_ori_inst(num_reg_1,0, node->opr[0]->value);
        //reg_available()
        allocate_reg();
        num_reg_2 = get_current_reg();
        create_ori_inst(num_reg_2,0, node->opr[1]->value);
        release_reg();
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
    }
  }

  if(s == 1){
    //ident+expr int+expr expr+ident expr+int expr+expr
    if(node->opr[0]->nature == NODE_IDENT){
      if(node->opr[1]->nature != NODE_IDENT && node->opr[1]->nature != NODE_INTVAL){
        //ident+expr
        num_reg_1 = get_current_reg();
        allocate_reg();
        num_reg_2 = get_current_reg();
        release_reg();
        asm_global_local(node, num_reg_2, 0);
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
    }
    else if(node->opr[0]->nature==NODE_INTVAL){
      if(node->opr[1]->nature != NODE_IDENT && node->opr[1]->nature != NODE_INTVAL){
        //int+expr
        num_reg_1=get_current_reg();
        allocate_reg();
        num_reg_2=get_current_reg();
        create_ori_inst(num_reg_2,0, node->opr[0]->value);
        release_reg();
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
    }
    else{
      num_reg_1=get_current_reg();
      if(node->opr[1]->nature==NODE_IDENT){
        //expr+ident
        allocate_reg();
        num_reg_2=get_current_reg();
        release_reg();
        asm_global_local(node, num_reg_2, 1);
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);

      }
      else if(node->opr[1]->nature==NODE_INTVAL){
        //expr+int
        allocate_reg();
        num_reg_2=get_current_reg();
        release_reg();
        create_ori_inst(num_reg_2,0, node->opr[1]->value);
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
      else{
        //expr+expr
        allocate_reg();
        num_reg_2=get_current_reg();
        release_reg();
        asm_switch_arithOpr(node, num_reg_1, num_reg_2);
      }
    }
  }
}

void asm_switch_arithOpr(node_t node, int32_t num_reg_1, int32_t num_reg_2) {
  switch(node->nature) {
    case NODE_PLUS:
      create_addu_inst(num_reg_1, num_reg_1, num_reg_2);

      break;
    case NODE_MINUS:
      create_subu_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
    case NODE_MUL:
      create_mult_inst(num_reg_1, num_reg_2);
      create_mflo_inst(num_reg_1);
      break;
    case NODE_DIV:
      create_teq_inst(0, num_reg_2);
      create_div_inst(num_reg_1, num_reg_2);
      create_mflo_inst(num_reg_1);
      break;
    case NODE_MOD:
      create_teq_inst(0, num_reg_2);
      create_div_inst(num_reg_1, num_reg_2);
      create_mfhi_inst(num_reg_1);
      break;
    case NODE_BAND:
      create_and_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
    case NODE_BOR:
      create_or_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
    case NODE_BXOR:
      create_xor_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
    case NODE_SLL:
      create_sllv_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
    case NODE_SRA:
      create_srav_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
    case NODE_SRL:
      create_srlv_inst(num_reg_1, num_reg_1, num_reg_2);
      break;
  }
}

void asm_NODE_arithOpr_u(node_t node,int s){
  int32_t num_reg_1=get_current_reg();
  if(s==0){
    if(node->opr[0]->nature==NODE_IDENT){
      asm_global_local(node, num_reg_1, 0);
      asm_switch_arithOpr_u(node, num_reg_1);
    }
    else if(node->opr[0]->nature==NODE_INTVAL||node->opr[0]->nature==NODE_BOOLVAL){
      create_ori_inst(num_reg_1,0, node->opr[0]->value);
      asm_switch_arithOpr_u(node, num_reg_1);
    }
  }

  if(s==1){
    if(node->opr[0]->nature != NODE_IDENT && node->opr[0]->nature != NODE_INTVAL
        && node->opr[0]->nature != NODE_BOOLVAL){
            asm_switch_arithOpr_u(node, num_reg_1);
    }
  }
}

void asm_switch_arithOpr_u(node_t node, int32_t num_reg_1) {
    switch(node->nature) {
      case NODE_UMINUS:
        create_subu_inst(num_reg_1, 0, num_reg_1);
        break;
      case NODE_BNOT:
        create_nor_inst(num_reg_1, 0, num_reg_1);
        break;
      case NODE_NOT:
        create_beq_inst(num_reg_1, 0, num_block);
        num_block++;
        create_j_inst(num_block);
        create_label_inst(num_block-1);
        create_ori_inst(num_reg_1, 0, 1);
        num_block++;
        create_j_inst(num_block);
        create_label_inst(num_block-1);
        create_ori_inst(num_reg_1, 0, 0);
        create_label_inst(num_block);
        num_block++;
        asm_condition_instruction();
        break;
    }
}

void asm_condition_instruction(){
  if(for_flag==1){
    create_beq_inst(get_current_reg(), 0, for_end);
    for_flag=0;
    }
  if(while_flag==1){
    create_beq_inst(get_current_reg(), 0, while_end);
    while_flag=0;
    }
  if(dowhile_flag==1){
    create_beq_inst(get_current_reg(), 0, dowhile_end);
    dowhile_flag=0;
    }
  if(if_flag==1){
    create_beq_inst(get_current_reg(), 0, if_end);
    if_flag=0;
    }
}

void asm_NODE_logicalOpr(node_t node,int s){
  int32_t num_reg_1;
  int32_t num_reg_2;
  if(s==0){
    if(while_flag==1){
      create_label_inst(while_start);
      }
    switch(node->opr[0]->nature){
      case NODE_IDENT:{
        if(node->opr[1]->nature == NODE_IDENT){
          //ident ident
          num_reg_1=get_current_reg();
          asm_global_local(node, num_reg_1, 0);
          allocate_reg();
          num_reg_2=get_current_reg();
          release_reg();
          asm_global_local(node, num_reg_2, 1);
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        else if(node->opr[1]->nature == NODE_INTVAL||node->opr[1]->nature == NODE_BOOLVAL){
          //ident intbool
          num_reg_1=get_current_reg();
          asm_global_local(node, num_reg_1, 0);
          allocate_reg();
          num_reg_2=get_current_reg();
          release_reg();
          create_ori_inst(num_reg_2,0, node->opr[1]->value);
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        break;
      }

      case NODE_INTVAL:
      case NODE_BOOLVAL:{
        if(node->opr[1]->nature == NODE_IDENT){
          //intbool ident
          num_reg_1=get_current_reg();
          create_ori_inst(num_reg_1,0,node->opr[0]->value );
          allocate_reg();
          num_reg_2=get_current_reg();
          release_reg();
          asm_global_local(node, num_reg_2, 1);
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        else if(node->opr[1]->nature == NODE_INTVAL||node->opr[1]->nature == NODE_BOOLVAL){
          //intbool intbool
          num_reg_1=get_current_reg();
          create_ori_inst(num_reg_1,0,node->opr[0]->value );
          allocate_reg();
          num_reg_2=get_current_reg();
            release_reg();
          create_ori_inst(num_reg_2,0, node->opr[1]->value);
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        break;
        }
      }
    }

  if(s==1){
    switch(node->opr[0]->nature){
      case NODE_IDENT:{
        //ident expr
        if(node->opr[1]->nature != NODE_IDENT && node->opr[1]->nature != NODE_INTVAL
            && node->opr[1]->nature != NODE_BOOLVAL){
            num_reg_2=get_current_reg();
            allocate_reg();
            num_reg_1=get_current_reg();
            release_reg();
            asm_global_local(node, num_reg_1, 0);
            asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
          }
        break;
      }

      case NODE_INTVAL:
      case NODE_BOOLVAL:{
        //intbool expr
        if(node->opr[1]->nature != NODE_IDENT && node->opr[1]->nature != NODE_INTVAL
            && node->opr[1]->nature != NODE_BOOLVAL){
            num_reg_2=get_current_reg();
            allocate_reg();
            num_reg_1=get_current_reg();
            release_reg();
            create_ori_inst(num_reg_1,0,node->opr[0]->value );
            asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
          }
        break;
      }
      default:{
        if(node->opr[1]->nature == NODE_IDENT){
          //expr ident
          num_reg_1=get_current_reg();
          allocate_reg();
          num_reg_2=get_current_reg();
          release_reg();
          asm_global_local(node, num_reg_2, 1);
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        else if(node->opr[1]->nature == NODE_INTVAL||node->opr[1]->nature == NODE_BOOLVAL){
          //expr intbool
          num_reg_1=get_current_reg();
          allocate_reg();
          num_reg_2=get_current_reg();
          release_reg();
          create_ori_inst(num_reg_2,0, node->opr[1]->value);
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        else{
          //expr expr
          num_reg_1=get_current_reg();
          allocate_reg();
          num_reg_2=get_current_reg();
          release_reg();
          asm_switch_logicalOpr(node, num_reg_1,num_reg_2);
        }
        break;
      }
    }
  }
}

void asm_switch_logicalOpr(node_t node, int32_t num_reg_1, int32_t num_reg_2){
  switch(node->nature){

    case NODE_LT:
    create_slt_inst(num_reg_1, num_reg_1, num_reg_2);//<
    asm_condition_instruction();
    break;

    case NODE_GT:
    create_slt_inst(num_reg_1,num_reg_2 , num_reg_1);//>
    asm_condition_instruction();
    break;

    case NODE_EQ:
    create_beq_inst(num_reg_1, num_reg_2, num_block);//==
    create_addiu_inst(num_reg_1, 0, 0);
    create_label_inst(num_block);
    num_block++;
    asm_condition_instruction();
    break;

    case NODE_NE:
    create_bne_inst(num_reg_1, num_reg_2, num_block);//!=
    create_addiu_inst(num_reg_1, 0, 0);
    create_label_inst(num_block);
    num_block++;
    asm_condition_instruction();
    break;

    case NODE_LE:
    create_beq_inst(num_reg_1, num_reg_2, num_block);//==
    create_slt_inst(num_reg_1, num_reg_1, num_reg_2);//<=
    create_bne_inst(num_reg_1, 0, num_block);
    create_addiu_inst(num_reg_1, 0, 0);
    create_label_inst(num_block);
    num_block++;
    asm_condition_instruction();
    break;

    case NODE_GE:
    create_beq_inst(num_reg_1, num_reg_2, num_block);//==
    create_slt_inst(num_reg_1, num_reg_2, num_reg_1);//<=
    create_bne_inst(num_reg_1, 0, num_block);
    create_addiu_inst(num_reg_1, 0, 0);
    create_label_inst(num_block);
    num_block++;
    asm_condition_instruction();
    break;

    case NODE_AND:
    create_and_inst(num_reg_1, num_reg_1, num_reg_2);
    asm_condition_instruction();
    break;

    case NODE_OR:
    create_or_inst(num_reg_1, num_reg_1, num_reg_2);
    asm_condition_instruction();
    break;

  }
}

//determiner si la variable est global
void asm_global_local(node_t node, int32_t num_reg, int s){
  if(s==0){
    if(node->opr[0]->decl_node->global_decl==true){
      create_lui_inst(num_reg, data_start);
      create_lw_inst(num_reg, node->opr[0]->decl_node->offset, num_reg);
    }
    else
      create_lw_inst(num_reg, node->opr[0]->decl_node->offset, 29);
  }

  if(s==1){
    if(node->opr[1]->decl_node->global_decl==true){
      create_lui_inst(num_reg, data_start);
      create_lw_inst(num_reg, node->opr[1]->decl_node->offset, num_reg);
    }
    else
      create_lw_inst(num_reg, node->opr[1]->decl_node->offset, 29);
  }
}
