#define _GNU_SOURCE
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "loader.h"
#include "loader_elf.h"
#include "loader_handler.h"
#include "loader_mem.h"
#include "loader_stack.h"

static enum heuristics heuristic = 0;
static Loadable_segment* load_list_head = NULL;
static Loadee_mgmt* loadee = NULL;

int
hybrid_load_segments( Elf_info* ei ); 

int
hybrid_load_elf_binary( void );

static void
hybrid_segv_handler( int sig, siginfo_t* si, void* unused );

int
main( int argc, char** argv, char** envp ) {
  struct sigaction sa;
  uint64_t sp;
  uint64_t ept;
  
  if (argc < 3) {
    fprintf( stderr, "Please supply an additional argument. \nExample: \n" );
    fprintf( stderr, "  $ ./hpager 1 input_prog\n" );
    return 0;
  }

  // Determine which heuristic is desired
  if (strncmp( argv[1], "1", 1 ) == 0) {
    printf( "Map 1 page\n" );
    heuristic = MAP1; 
  } else if (strncmp( argv[1], "2", 1 ) == 0) {
    printf( "Map 2 pages\n" );
    heuristic = MAP2; 
  } else if (strncmp( argv[1], "3", 1 ) == 0) {
    printf( "Map 3 pages\n" );
    heuristic = MAP3; 
  } else {
    fprintf( stderr, "Unexpected heuristic choice. Please use either 1, 2, or 3" );
    return -1;
  }
  
  
  loadee = loader_get_new_manager( argv + 1 );
  
  
  // if number of aux_vectors is incorrect, it'll get reset in le_setup_stack
  struct loader_stack_info hpager_info = { argc - 1, argv + 1, 0, envp, 19, NULL };

  if (hybrid_load_elf_binary( ) != 0) {
    fprintf( stderr, "Failed to load elf binary\n" );
    return -1;    
  }

  if (ls_setup_stack( &hpager_info, loadee ) != 0) {
    fprintf( stderr, "Failed to set up stack\n" );
    return -1;
  }

  // Set up signal handler
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = hybrid_segv_handler;

  if ( sigaction( SIGSEGV, &sa, NULL ) == -1 ) {
    perror( "Failed to install signal handler" );
    return -1;
  }

  // Clean up data structures
  // don't close file b/c still need to perform mappings
  sp = loadee->sp;
  ept = loadee->entry_pt;
  
  loader_start_loadee( sp, ept );
    
  return 0;  
}

int
hybrid_load_segments( Elf_info* ei )
{
  Elf64_Phdr* phdr_it = ei->phdrs;
  Loadable_segment* inserted_segment = NULL;
  for (int i = 0; i < ei->hdr->e_phnum; ++i) {
    int prot = 0;
    int flags = MAP_PRIVATE;
    if (phdr_it->p_type == PT_LOAD) {
      struct mappable_mem_region file_backed_seg;
      // Always map the file backed part
      file_backed_seg.real.start_addr = phdr_it->p_vaddr;
      file_backed_seg.length = phdr_it->p_filesz;
      file_backed_seg.real.end_addr =
        file_backed_seg.real.start_addr + file_backed_seg.length;
      file_backed_seg.fd = loadee->fd;
      file_backed_seg.offset = phdr_it->p_offset;
      
      if (phdr_it->p_flags & PF_X) prot = prot | PROT_EXEC;
      if (phdr_it->p_flags & PF_W) prot = prot | PROT_WRITE;
      if (phdr_it->p_flags & PF_R) prot = prot | PROT_READ;

      flags = flags | MAP_POPULATE;

      file_backed_seg.protection = prot;
      file_backed_seg.flags = flags;

      if ( lm_map_memregion( &file_backed_seg ) != 0 ) {
        fprintf( stderr, "Failed to mmap file-backed segment\n" );
        return -1;
      }

      if (phdr_it->p_memsz > phdr_it->p_filesz) {
        inserted_segment = lh_insert_segment( phdr_it, load_list_head, loadee, 1 );
        if (!load_list_head) load_list_head = inserted_segment;

        if (inserted_segment == NULL) {
          fprintf( stderr, "Error inserting load segment into load_list\n" );
          return -1;
        }
      }      
    }
    
    phdr_it++;  
  }

  return 0;
}


int
hybrid_load_elf_binary( void )
{
  if (loadee == NULL) {
    printf ("Invalid loadee\n");
    return -1;
  }

  Elf_info ei;
  
  le_get_elfinfo( loadee, &ei );

  if ( hybrid_load_segments(  &ei ) != 0) {
    fprintf( stderr, "Failed to load segments properly\n" );
    exit( -1 );
  }

  free (ei.phdrs);
  return 0;
}

static void
hybrid_segv_handler( int sig, siginfo_t* si, void* unused )
{
  // Make sure that the faulting address is within bounds
  if ( lm_validate_address( &(loadee->bounds), (uint64_t)si->si_addr ) != 0 ) {
    fprintf( stderr, "Segmentation fault caused by invalid address\n" );
    exit( -1 );
  }

  switch( heuristic ) {
    case MAP1: 
      lh_map_one( si->si_addr, load_list_head );
      break;
    case MAP2:
      lh_map_two( si->si_addr, load_list_head );
      break;
    case MAP3:
      lh_map_three( si->si_addr, load_list_head );
      break;
    default:
      fprintf( stderr, "Unexpected heuristic identifier. Exiting.\n ");
      exit( -1 );
  }

  // See the pages that have been mapped
  //lh_print_load_list( load_list_head, 0 );
}
