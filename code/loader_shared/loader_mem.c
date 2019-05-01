#define _GNU_SOURCE
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "loader_mem.h"

static void print_mapping( struct mem_region* mr);
static void print_mem_region( struct mem_region* mr);

int lm_validate_address( struct mem_bounds* loadee_mem, uint64_t addr ) {
  // todo: maybe replace this with something mmap related

  printf( "Comparing %#" PRIx64 " to start addr %#" PRIx64 " and end addr %#"
          PRIx64 "\n", addr, loadee_mem->start_addr, loadee_mem->end_addr );
  
  if ((addr <= loadee_mem->end_addr) &&
      (addr >= loadee_mem->start_addr)) 
    return 0;
  return -1;
}  

size_t lm_calc_mmap_length( uint64_t start_addr, size_t size ) {
  uint64_t end_addr;
  int num_pages;
  size_t num_bytes;

  end_addr = start_addr + (uint64_t)size;
  num_pages = (size / PG_SIZE) + ((size % PG_SIZE) && 1) + ((end_addr % PG_SIZE) && 1);

  //TODO
  printf("num_pages: %d\n", num_pages);
  num_bytes = num_pages * PG_SIZE;
  
  return num_bytes;
}
  
int lm_map_memregion(struct mem_region* mappee) {
  uint64_t map_start;
  off_t map_offset;
  size_t map_length;
  char* mapping = NULL;
  uint64_t real_end_addr;
  uint64_t map_end_addr;
  uint64_t leftover_bytes;

  map_start = (uint64_t)(mappee->virt_address) & ~((uint64_t)PG_SIZE - 1);
  map_offset = ((uint64_t)mappee->offset) & ~((uint64_t)PG_SIZE - 1);
  map_length = lm_calc_mmap_length( mappee->virt_address, mappee->length );
  
  mapping = mmap( (void*)map_start, map_length, mappee->protection,
                 (mappee->flags | DEFAULT_FLAGS), mappee->fd, map_offset );

  if ((uint64_t)mapping != map_start) {
    fprintf( stderr, "Failed to map desired region\n" );
    munmap( mapping, map_length );
 
    return -1;
  }

  map_end_addr = (uint64_t)map_start + (uint64_t)map_length;
  real_end_addr = mappee->virt_address + mappee->length;

  leftover_bytes = map_end_addr - real_end_addr; 
  
  // Zero out data that should not be file-backed
  if (leftover_bytes) {
    // Part of mapped region should not be populated by file
    memset( (void*)real_end_addr, 0, leftover_bytes );
  } else if (mappee->flags & MAP_ANONYMOUS) {
    memset( (void*)map_start, 0, map_length );
  }
  
  mappee->map_start = (char*)map_start;
  mappee->map_offset = map_offset;
  mappee->map_size = map_length;
  mappee->map_end = (char*)map_end_addr;

  print_mem_region( mappee );
  return 0;

  
  // TODO: get rid of "print_mem_region"
  print_mapping( mappee );
  
  return 0;
}

void print_mapping( struct mem_region* mr ) {
  fprintf(stderr, "mapping created at address: %#" PRIx64
          "\toffset: %ld\tsize: %lu\n",
          (uint64_t)mr->map_start, mr->map_offset, mr->map_size);
}

void print_mem_region( struct mem_region* mr ) {
  fprintf(stderr, "mapping at virtual address: %#" PRIx64
         "\n\tlength: %lu\n\tprotection: %d\n\tflags: %d\n\tfd: %d\n\t"
          "offset: %ld\n\tmap_start: %#" PRIx64
          "\n\tmap_offset: %ld\n\tmap_size: %lu\n\tmap_end: %#" PRIx64 "\n",
          (uint64_t)mr->virt_address, mr->length, mr->protection, mr->flags,
          mr->fd, mr->offset, (uint64_t)mr->map_start, mr->map_offset, mr->map_size,
          (uint64_t)mr->map_end);            
}


