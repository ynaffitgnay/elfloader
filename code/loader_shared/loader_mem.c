#define _GNU_SOURCE
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "loader_mem.h"

static void
print_mapping( struct mappable_mem_region* mmr);

int
lm_validate_address( Mem_bounds* loadee_mem, uint64_t addr )
{
  //printf( "Comparing %#" PRIx64 " to start addr %#" PRIx64 " and end addr %#"
  //        PRIx64 "\n", addr, loadee_mem->start_addr, loadee_mem->end_addr );
  
  if ((addr <= loadee_mem->end_addr) &&
      (addr >= loadee_mem->start_addr)) 
    return 0;
  return -1;
}  

size_t
lm_calc_mmap_length( uint64_t start_addr, size_t size )
{
  uint64_t end_addr;
  int num_pages;
  size_t num_bytes;

  end_addr = start_addr + (uint64_t)size;
  
  num_pages =
    (size / PG_SIZE) + ((start_addr % PG_SIZE) && 1) + ((end_addr % PG_SIZE) && 1);

  num_bytes = num_pages * PG_SIZE;
  
  return num_bytes;
}

int
lm_define_memregion( struct mappable_mem_region* mappee, int create_mapping )
{
  uint64_t map_start;
  off_t map_offset;
  size_t map_length;
  char* mapping = NULL;
  uint64_t map_end_addr;
  uint64_t leftover_bytes;

  map_start = PG_RND_DOWN(mappee->real.start_addr);
  
  map_offset = PG_RND_DOWN(mappee->offset);
  
  map_length = lm_calc_mmap_length( mappee->real.start_addr, mappee->length );

  map_end_addr = (uint64_t)map_start + (uint64_t)map_length;


  mappee->map.start_addr = map_start;
  mappee->map.end_addr = map_end_addr;
  mappee->map_offset = map_offset;
  mappee->map_size = map_length;  

  
  if (create_mapping) {
    mapping = mmap( (void*)map_start, map_length, mappee->protection,
                    (mappee->flags | DEFAULT_FLAGS), mappee->fd, map_offset );

    if ((uint64_t)mapping != map_start) {
      //fprintf( stderr, "mapping: %lx, map_start: %lx\t", (uint64_t)mapping, map_start );
      fprintf( stderr, "Failed to create mapping at desired address. " );
      if (mapping  == (void*)-1) {
        perror( "" );
      } else {
        fprintf( stderr, "Please try loading the same program again. \n" );
        munmap( mapping, map_length );
      } 
 
      return -1;
    }

    // Can't memset unwritable region
    if (mappee->protection & PROT_WRITE) {
      leftover_bytes = map_end_addr - mappee->real.end_addr; 

      if (leftover_bytes >= PG_SIZE) {
        fprintf(stderr, "\n\n     CALCULATING TOTAL NUMBER OF NECESSARY BYTES INCORRECTLY  \n\n");
        return -1;
      }

      // Zero out data that should not be file-backed
      if (leftover_bytes) {
        //printf( "Memsetting %" PRIu64 " bytes (0x%lx) to 0 starting at 0x%lx\n",
        //        leftover_bytes, leftover_bytes,  mappee->real.end_addr);
        
        // Part of mapped region should not be populated by file
        memset( (void*)mappee->real.end_addr, 0, leftover_bytes );
      } else if (mappee->flags & MAP_ANONYMOUS) {
        memset( (void*)map_start, 0, map_length );
      }
    }

    print_mapping( mappee );
  }
  
  return 0;
}

int
lm_map_memregion( struct mappable_mem_region* mappee )
{
  return lm_define_memregion( mappee, 1 );
}


void
print_mapping( struct mappable_mem_region* mmr )
{
  if ( mmr->fd != -1 ) {
    fprintf(stderr, "mapping created at address: %#" PRIx64
            "\toffset: %ld\tsize: %lu (0x%lx)\n",
            mmr->map.start_addr, mmr->map_offset, mmr->map_size, mmr->map_size);
  } else {  
    fprintf(stderr, "mapping created at address: %#" PRIx64
            "\toffset: N/A \tsize: %lu (0x%lx)\n",
            mmr->map.start_addr, mmr->map_size, mmr->map_size);
  }
}

void
lm_print_mem_region( struct mappable_mem_region* mmr )
{
  fprintf(stderr, "mapping at virtual address: %#" PRIx64
         "\n\tlength: %lu (0x%lx)\n\tprotection: %s %s %s\n\tflags: %d\n\tfd: %d\n\t"
          "offset: %ld\n\tmap_start: %#" PRIx64
          "\n\tmap_offset: %ld\n\tmap_size: %lu (0x%lx)\n\tmap_end: %#" PRIx64 "\n",
          mmr->real.start_addr, mmr->length, mmr->length,
          (mmr->protection & PROT_EXEC) ? "PROT_EXEC |" : "",
          (mmr->protection & PROT_READ) ? "PROT_READ |" : "",
          (mmr->protection & PROT_WRITE) ? "PROT_WRITE" : "",mmr->flags,
          mmr->fd, mmr->offset, mmr->map.start_addr, mmr->map_offset,
          mmr->map_size, mmr->map_size, mmr->map.end_addr);            
}


