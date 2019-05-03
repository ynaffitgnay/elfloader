#include "loader_handler.h"

static void* get_next_addr( uint64_t start_addr, Loadable_segment* parent );
static void* get_next_stack_addr( void );
