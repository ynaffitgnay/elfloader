#define _GNU_SOURCE
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "loader_mem.h"
#include "loader_utils.h"

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
  
  //printf( " (size / PG_SIZE): %lu (%lx)\n ((start_addr % PG_SIZE) && 1): %lu (%lx)\n ((end_addr % PG_SIZE) && 1): %lu (%lx)\n",
  //        (size / PG_SIZE), (size / PG_SIZE),
  //        ((start_addr % PG_SIZE) && 1), ((start_addr % PG_SIZE) && 1),
  //        ((end_addr % PG_SIZE) && 1), ((end_addr % PG_SIZE) && 1));
  
  num_pages = (size / PG_SIZE) + ((start_addr % PG_SIZE) && 1) + ((end_addr % PG_SIZE) && 1);

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

  //lu_print_maps();
  mapping = mmap( (void*)map_start, map_length, mappee->protection,
                 (mappee->flags | DEFAULT_FLAGS), mappee->fd, map_offset );
  //lu_print_maps();
  
  
  if ((uint64_t)mapping != map_start) {
    fprintf( stderr, "Failed to map desired region\n" );
    munmap( mapping, map_length );
 
    return -1;
  }

  map_end_addr = (uint64_t)map_start + (uint64_t)map_length;
  real_end_addr = mappee->virt_address + mappee->length;

  printf( "real_end_addr: %#" PRIx64 "\n", real_end_addr );
  
  mappee->map_start = (char*)map_start;
  mappee->map_offset = map_offset;
  mappee->map_size = map_length;
  mappee->map_end = (char*)map_end_addr;


  // Can't memset unwritable region
  if (mappee->protection & PROT_WRITE) {
    leftover_bytes = map_end_addr - real_end_addr; 
    printf( "leftover bytes: %ld (0x%lx)\n", leftover_bytes, leftover_bytes );

    // Zero out data that should not be file-backed
    if (leftover_bytes) {
      printf( "Memsetting %" PRIu64 " bytes (0x%lx) to 0 starting at 0x%lx\n",
              leftover_bytes, leftover_bytes,  real_end_addr);
      // Part of mapped region should not be populated by file
      memset( (void*)real_end_addr, 0, leftover_bytes );
    } else if (mappee->flags & MAP_ANONYMOUS) {
      memset( (void*)map_start, 0, map_length );
    }
  }

  //print_mem_region( mappee );
  
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
         "\n\tlength: %lu (0x%lx)\n\tprotection: %s %s %s\n\tflags: %d\n\tfd: %d\n\t"
          "offset: %ld\n\tmap_start: %#" PRIx64
          "\n\tmap_offset: %ld\n\tmap_size: %lu (0x%lx)\n\tmap_end: %#" PRIx64 "\n",
          (uint64_t)mr->virt_address, mr->length, mr->length,
          (mr->protection & PROT_EXEC) ? "PROT_EXEC |" : "",
          (mr->protection & PROT_READ) ? "PROT_READ |" : "",
          (mr->protection & PROT_WRITE) ? "PROT_WRITE" : "",mr->flags,
          mr->fd, mr->offset, (uint64_t)mr->map_start, mr->map_offset,
          mr->map_size, mr->map_size, (uint64_t)mr->map_end);            
}


