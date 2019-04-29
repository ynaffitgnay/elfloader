#include "loader_stack.h"

static int populate_info_block( struct loader_stack_info* lsinfo, Loadee_mgmt* loadee,
                                char** out_argv, char** out_envp );
static int copy_args( char** in_argv, int argc, char** out_argv );
