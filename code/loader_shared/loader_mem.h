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

struct mappable_mem_region {
  Mem_bounds real;  // filled by caller
  off_t offset;            // filled by caller
  size_t length;           // filled by caller
  int protection;          // filled by caller
  int flags;               // filled by caller
  int fd;                  // filled by caller
  Mem_bounds map;   // filled by mapper
  off_t map_offset;        // filled by mapper
  size_t map_size;         // filled by mapper
};


int lm_validate_address( Mem_bounds* loadee_mem, uint64_t addr );
size_t lm_calc_mmap_length( uint64_t start_addr, size_t size );
int lm_define_memregion( struct mappable_mem_region* mappee, int create_mapping );
int lm_map_memregion( struct mappable_mem_region* mappee );

#endif
