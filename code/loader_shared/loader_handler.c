#include <stdio.h>
#include <stdlib.h>

#include "loader_handler.h"
#include "loader_mem.h"

static Loadable_segment*
find_parent_segment( uint64_t addr, Loadable_segment* load_list );

static void*
get_next_addr( uint64_t start_addr, Loadable_segment* parent );

static int
map_n_pages( uint64_t start_addr, Loadable_segment* parent, int num_pages );


static void*
get_next_stack_addr( void );

static void
print_loadable_segment( Loadable_segment* ls, int print_mmr );

// Returns a pointer to the first newly created Loadable_segment
// anon_only indicates that only anonymous portion of loadable_segment should be
// added to the loadable segment list
Loadable_segment*
lh_insert_segment( Elf64_Phdr* phdr, Loadable_segment* load_list,
                   Loadee_mgmt* loadee, int anon_only )
{
  Loadable_segment* file_backed_seg = NULL;
  Loadable_segment* anonymous_seg = NULL;
  Loadable_segment* curr_seg = NULL;
  Loadable_segment* return_seg = NULL;

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
    
  if (phdr->p_memsz > phdr->p_filesz) {
    size_t first_section_bytes;
    size_t total_segment_bytes;
    size_t last_section_bytes;

    anonymous_seg = (Loadable_segment*)malloc( sizeof( Loadable_segment ) );

    // Create seg for bss
    // protection stays the same
    flags = MAP_PRIVATE | MAP_ANONYMOUS;  // re-initialize flags

    first_section_bytes =
      lm_calc_mmap_length( file_backed_seg->mmr.real.start_addr,
                           file_backed_seg->mmr.length );
           
    total_segment_bytes =
      lm_calc_mmap_length( file_backed_seg->mmr.real.start_addr,
                           phdr->p_memsz );
    last_section_bytes = total_segment_bytes - first_section_bytes;

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
  }

  return_seg = anon_only ? anonymous_seg : file_backed_seg;

  if ( return_seg != NULL ) {  
    // Insert segment into list
    if (load_list != NULL) {
      for ( curr_seg = load_list; curr_seg->next != NULL; curr_seg++ ) {
        if ( curr_seg->first_page_addr > return_seg->first_page_addr ) break;
      }
    
      
      if (curr_seg->next == NULL) {
        // Insert behind curr_seg
        curr_seg->next = return_seg;
        return_seg->prev = curr_seg;
      } else {
        // Insert in front of curr_seg
        return_seg->prev = curr_seg->prev;
        if (return_seg->prev != NULL) return_seg->prev->next = return_seg;
        return_seg->next = curr_seg;
        curr_seg->prev = return_seg;
      }
    }
  }

  if ( !anon_only && anonymous_seg ) {
    // anonymous goes on the end of file-backed
    anonymous_seg->next = file_backed_seg->next;
    if ( anonymous_seg->next != NULL ) anonymous_seg->next->prev = anonymous_seg;
    anonymous_seg->prev = file_backed_seg;
    file_backed_seg->next = anonymous_seg;
  } else if (anon_only) {
    free( file_backed_seg );
  }

    

  
  return return_seg;
  
}

Loadable_segment*
find_parent_segment( uint64_t addr, Loadable_segment* load_list )
{
  Loadable_segment* curr_seg = load_list;

  while( curr_seg != NULL) {
    if( lm_validate_address( &(curr_seg->mmr.map), addr ) == 0 ) return curr_seg;      
    curr_seg = curr_seg->next;
  }
  
  return NULL;
}

int
map_n_pages( uint64_t start_addr, Loadable_segment* parent, int num_pages )
{
  struct mappable_mem_region child;
  int bytes_left_in_page;
  
  if (parent == NULL) {
    child.real.start_addr = PG_RND_DOWN( start_addr );
    child.length = PG_SIZE * num_pages;
    child.real.end_addr = child.real.start_addr + child.length;
    child.offset = 0;
    child.protection = (PROT_READ | PROT_WRITE);
    child.flags = (MAP_PRIVATE | MAP_ANONYMOUS);
    child.fd = -1;
  } else {
    if ( PG_RND_DOWN( start_addr ) == parent->last_page_addr ) {
      child.real.start_addr = PG_RND_DOWN( start_addr );
      child.real.end_addr = parent->mmr.real.end_addr;
      child.length = ( parent->mmr.real.end_addr % PG_SIZE );
      if (child.length == 0) child.length = PG_SIZE;
    } else {
      child.real.start_addr = start_addr;
      bytes_left_in_page = PG_SIZE - ( start_addr % PG_SIZE );
      child.length = bytes_left_in_page + (num_pages - 1) * PG_SIZE;
      child.real.end_addr = child.real.start_addr + child.length;
    }

    child.offset =
      parent->mmr.map_offset + (child.real.start_addr - parent->mmr.map.start_addr);

    child.protection = parent->mmr.protection;
    child.flags = parent->mmr.flags;
    child.fd = parent->mmr.fd;
  }
  
  if (lm_map_memregion( &child ) != 0) {
    fprintf( stderr, "Error mapping page in\n" );
    return -1;
  }

  parent->pages_mapped += num_pages;
  
  return 0;
}

void
lh_map_one( void* fault_addr, Loadable_segment* load_list )
{
  Loadable_segment* parent = find_parent_segment( (uint64_t)fault_addr, load_list );
  if (map_n_pages( (uint64_t)fault_addr, parent, 1) != 0) {
    fprintf( stderr, "Failed to map page\n" );
    exit( -1 );
  }
}

void
lh_map_two( void* fault_addr, Loadable_segment* load_list )
{
}

void
lh_map_three( void* fault_addr, Loadable_segment* load_list )
{
}

void*
get_next_addr( uint64_t start_addr, Loadable_segment* parent )
{
  return NULL;
}

void*
get_next_stack_addr( void )
{
  return NULL;
}

void
print_loadable_segment( Loadable_segment* ls, int print_mmr )
{
  if (print_mmr) lm_print_mem_region( &(ls->mmr) );
  fprintf( stderr, "first_page_addr: 0x%lx\tlast_page_addr: 0x%lx\n"
           "\tpages_to_map: %lu\tpages_mapped: %lu\n",
           ls->first_page_addr, ls->last_page_addr,
           ls->pages_to_map, ls->pages_mapped );
}

void
lh_print_load_list( Loadable_segment* load_list, int print_mmr )
{
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
