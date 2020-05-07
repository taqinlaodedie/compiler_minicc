#ifndef _PASSE2_H_
#define _PASSE2_H_

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
#include "utils/mips_inst.h"

program_t root_program;
extern char *outfile;

extern void asm_gen(node_t node);

#endif