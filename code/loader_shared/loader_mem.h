#ifndef _LOADER_MEM_H
#define _LOADER_MEM_H

#define _GNU_SOURCE
#include <inttypes.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "loader.h"

#define PG_SIZE (4096)
#define DEFAULT_FLAGS ( MAP_PRIVATE | MAP_POPULATE )
#define PG_RND_DOWN(x) (((uint64_t)(x)) & ~((uint64_t)PG_SIZE - 1))

struct mappable_mem_region {
  Mem_bounds real;  // non-page-aligned start + end address. caller provided.
  size_t length;    // non-aligned length of mapping. caller provided.
  off_t offset;     // non-aligned offset into file. caller provided.
  int protection;   // access protection of mapping. caller provided.
  int flags;        // access flags. caller provided.
  int fd;           // file descriptor of open backing file. caller provided.
  Mem_bounds map;   // page-aligned start + end addresses of mapping. filled by mapper.
  size_t map_size;  // total size of page-aligned mapping. filled by mapper.
  off_t map_offset; // page-aligned offset into file. filled by mapper
  
};


int lm_validate_address( Mem_bounds* loadee_mem, uint64_t addr );
size_t lm_calc_mmap_length( uint64_t start_addr, size_t size );
int lm_define_memregion( struct mappable_mem_region* mappee, int create_mapping );
int lm_map_memregion( struct mappable_mem_region* mappee );
void lm_print_mem_region( struct mappable_mem_region* mmr);

#endif
