#ifndef _LOADER_STACK_H_
#define _LOADER_STACK_H_

#include <inttypes.h>

#include "loader.h"

struct loader_stack_info {
  uint64_t argc;
  char** argv;
  int envc;
  char** envp;
  int auxc;
  char** auxv;
};

int ls_setup_stack( struct loader_stack_info* lsinfo, Loadee_mgmt* loadee );
char* ls_get_auxv_addr( char** envp, int* num_env_vars );


#endif
