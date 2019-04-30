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

struct mem_region {
  char* virt_address;
  size_t length;
  int protection;
  int flags;
  int fd;
  off_t offset;
  char* map_start;
  off_t map_offset;
  size_t map_size;
  char* map_end;  
};

int lm_validate_address( struct mem_bounds* loadee_mem, uint64_t addr );
size_t lm_calc_mmap_length( void* start_addr, size_t size );
int lm_map_memregion(struct mem_region* mappee);

#endif
