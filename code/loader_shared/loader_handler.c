#include <stdio.h>
#include <stdlib.h>

#include "loader_handler.h"
#include "loader_mem.h"

static void* get_next_addr( uint64_t start_addr, Loadable_segment* parent );
static void* get_next_stack_addr( void );
static void print_loadable_segment( Loadable_segment* ls, int print_mmr );

// Returns a pointer to the first newly created Loadable_segment
Loadable_segment* lh_insert_segment( Loadable_segment* load_list, Elf64_Phdr* phdr, Loadee_mgmt* loadee ) {
  Loadable_segment* file_backed_seg = NULL;
  Loadable_segment* anonymous_seg = NULL;
  Loadable_segment* curr_seg = NULL;

  int prot = 0;
  int flags = MAP_PRIVATE;

  if (phdr->p_type != PT_LOAD) {
    fprintf( stderr, "Attempting to insert non-PT_LOAD segment into load_list\n" );
    return NULL;
  }

  // Create file-backed seg:
  file_backed_seg = (Loadable_segment*)malloc( sizeof( Loadable_segment ) );
  // Always map the file backed part
  file_backed_seg->mmr.real.start_addr = phdr->p_vaddr;
  file_backed_seg->mmr.length = phdr->p_filesz;
  file_backed_seg->mmr.real.end_addr =
    file_backed_seg->mmr.real.start_addr + file_backed_seg->mmr.length;
  file_backed_seg->mmr.fd = loadee->fd;
  file_backed_seg->mmr.offset = phdr->p_offset;
      
  if (phdr->p_flags & PF_X) prot = prot | PROT_EXEC;
  if (phdr->p_flags & PF_W) prot = prot | PROT_WRITE;
  if (phdr->p_flags & PF_R) prot = prot | PROT_READ;

  flags = flags | MAP_POPULATE;

  file_backed_seg->mmr.protection = prot;
  file_backed_seg->mmr.flags = flags;

  if ( lm_define_memregion( &(file_backed_seg->mmr), 0 ) != 0 ) {
    fprintf( stderr, "Failed to define file-backed segment\n" );
    return NULL;
  }

  if (PG_RND_DOWN( file_backed_seg->mmr.map.start_addr ) != file_backed_seg->mmr.map.start_addr ) {
    fprintf( stderr, "UNEXPECTED PG_RND_DOWN BHVR\n\n" );
    exit( -1 );
  }
  file_backed_seg->first_page_addr = file_backed_seg->mmr.map.start_addr;
  file_backed_seg->last_page_addr = PG_RND_DOWN(file_backed_seg->mmr.map.end_addr - 1);
    
  //TODO
  //printf("map end_addr: %lx, last_page_addr: %lx\n",
  //       file_backed_seg->mmr.map.end_addr, 
  //       file_backed_seg->last_page_addr);

  file_backed_seg->pages_to_map = (file_backed_seg->mmr.map_size / PG_SIZE);
  file_backed_seg->pages_mapped = 0;
  file_backed_seg->next = NULL;
  file_backed_seg->prev = NULL;

  
  // Insert it into list
  if (load_list != NULL) {
    for ( curr_seg = load_list; curr_seg->next != NULL; curr_seg++ ) {
      if ( curr_seg->first_page_addr > file_backed_seg->first_page_addr ) break;
    }

    
    if (curr_seg->next == NULL) {
      // Insert behind curr_seg
      curr_seg->next = file_backed_seg;
      file_backed_seg->prev = curr_seg;
    } else {
      // Insert in front of curr_seg
      file_backed_seg->prev = curr_seg->prev;
      if (file_backed_seg->prev != NULL) file_backed_seg->prev->next = file_backed_seg;
      file_backed_seg->next = curr_seg;
      curr_seg->prev = file_backed_seg;
    }
  }
    
  if (phdr->p_memsz > phdr->p_filesz) {
    size_t first_section_bytes;
    size_t total_sector_bytes;
    size_t last_section_bytes;

    anonymous_seg = (Loadable_segment*)malloc( sizeof( Loadable_segment ) );

    // Create seg for bss
    // protection stays the same
    flags = MAP_PRIVATE | MAP_ANONYMOUS;  // re-initialize flags

    first_section_bytes =
      lm_calc_mmap_length( file_backed_seg->mmr.real.start_addr,
                           file_backed_seg->mmr.length );
           
    total_sector_bytes =
      lm_calc_mmap_length( file_backed_seg->mmr.real.start_addr,
                           phdr->p_memsz );
    last_section_bytes = total_sector_bytes - first_section_bytes;

    anonymous_seg->mmr.real.start_addr = file_backed_seg->mmr.map.end_addr;
    anonymous_seg->mmr.real.end_addr =
      anonymous_seg->mmr.real.start_addr + last_section_bytes;
    anonymous_seg->mmr.length = last_section_bytes;
    anonymous_seg->mmr.protection = file_backed_seg->mmr.protection;
    anonymous_seg->mmr.flags = flags;
    anonymous_seg->mmr.fd = -1;
    anonymous_seg->mmr.offset = 0;

    if ( lm_define_memregion( &(anonymous_seg->mmr), 0 ) != 0 ) {
      fprintf( stderr, "Failed to define file-backed segment\n" );
      return NULL;
    }

    anonymous_seg->first_page_addr = anonymous_seg->mmr.map.start_addr;
    anonymous_seg->last_page_addr = PG_RND_DOWN(anonymous_seg->mmr.map.end_addr - 1);
    
    //TODO
    //printf("map end_addr: %lx, last_page_addr: %lx\n",
    //       anonymous_seg->mmr.map.end_addr, 
    //       anonymous_seg->last_page_addr);

    anonymous_seg->pages_to_map = (anonymous_seg->mmr.map_size / PG_SIZE);
    anonymous_seg->pages_mapped = 0;

    // anonymous always goes on the end of file-backed
    anonymous_seg->next = file_backed_seg->next;
    if ( anonymous_seg->next != NULL ) anonymous_seg->next->prev = anonymous_seg;
    anonymous_seg->prev = file_backed_seg;
    file_backed_seg->next = anonymous_seg;
    
  }
  
  return file_backed_seg;
  
}

Loadable_segment* lh_find_parent_segment( Loadable_segment* load_list, uint64_t addr ) {
  Loadable_segment* curr_seg = load_list;

  while( curr_seg != NULL) {
    if( lm_validate_address( &(curr_seg->mmr.map), addr ) == 0 ) return curr_seg;      
    curr_seg = load_list->next;
  }
  
  return NULL;
}

int lh_map_pages( uint64_t start_addr, Loadable_segment* parent, int num_pages ) {
  Loadable_segment child;
  if (parent == NULL) {
    child.mmr.real.start_addr = PG_RND_DOWN( start_addr );
  }
  return 0;
}

void lh_map_one( void* fault_addr, Loadable_segment* load_list ) {
}

void lh_map_two( void* fault_addr, Loadable_segment* load_list ) {
}

void lh_map_three( void* fault_addr, Loadable_segment* load_list ) {
}

void* get_next_addr( uint64_t start_addr, Loadable_segment* parent ) {
  return NULL;
}

void* get_next_stack_addr( void ) {
  return NULL;
}

void print_loadable_segment( Loadable_segment* ls, int print_mmr ) {
  if (print_mmr) lm_print_mem_region( &(ls->mmr) );
  fprintf( stderr, "first_page_addr: 0x%lx\tlast_page_addr: 0x%lx\n"
           "\tpages_to_map: %lu\tpages_mapped:%lu\n",
           ls->first_page_addr, ls->last_page_addr,
           ls->pages_to_map, ls->pages_mapped );
}

void lh_print_load_list( Loadable_segment* load_list, int print_mmr ) {
  Loadable_segment* curr_seg = load_list;
  int idx = 0;
  if (load_list == NULL) {
    fprintf( stderr, "Load_list provided to print is NULL\n" );
    return;
  }

  while( curr_seg != NULL) {
    fprintf( stderr, "LS %d: \t", idx++ );
    print_loadable_segment( curr_seg, print_mmr);
    curr_seg = curr_seg->next;
  }
}
