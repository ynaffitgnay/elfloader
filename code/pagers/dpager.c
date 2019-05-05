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

static Loadable_segment* load_list_head = NULL;
static Loadee_mgmt* loadee = NULL;

int
demand_get_segments( Elf_info* ei );

int
demand_process_elf_binary( void );

void
demand_map_first_page( void );

static void
demand_segv_handler( int sig, siginfo_t* si, void* unused );

int
main( int argc, char** argv, char** envp )
{  
  if (argc < 2) {
    fprintf( stderr, "Please supply an additional argument\n" );
    return 0;
  }

  struct sigaction sa;
  uint64_t sp;
  uint64_t ept;

  loadee = loader_get_new_manager( argv );
  
  // if number of aux_vectors is incorrect, it'll get reset in le_setup_stack
  struct loader_stack_info dpager_info = { argc, argv, 0, envp, 19, NULL };

  if (demand_process_elf_binary( ) != 0) {
    fprintf( stderr, "Failed to load elf binary\n" );
    return -1;    
  }


  if (ls_setup_stack( &dpager_info, loadee ) != 0 ) {
    fprintf( stderr, "Failed to set up stack\n" );
    return -1;
  }

  demand_map_first_page( );

  // Set up signal handler
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = demand_segv_handler;

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
demand_get_segments( Elf_info* ei )
{
  Loadable_segment* inserted_segment = NULL;
  Elf64_Phdr* phdr_it = ei->phdrs;
  
  for (int i = 0; i < ei->hdr->e_phnum; ++i) {
    if (phdr_it->p_type == PT_LOAD) {
      inserted_segment = lh_insert_segment( phdr_it, load_list_head, loadee, 0 );
      
      if (!load_list_head) load_list_head = inserted_segment;

      if (inserted_segment == NULL) {
        fprintf( stderr, "Error inserting load segment into load_list\n" );
        return -1;
      }
    }
    
    phdr_it++;  
  }

  return 0;
} 

int
demand_process_elf_binary( void )
{ 
  if (loadee == NULL) {
    printf ("Invalid loadee\n");
    return -1;
  }

  Elf_info ei;
  
  le_get_elfinfo( loadee, &ei );

  if ( demand_get_segments( &ei ) != 0) {
    fprintf( stderr, "Failed to process segments properly\n" );
    exit( -1 );
  }

  free (ei.phdrs);
  return 0;
}

void
demand_map_first_page( void ) {
  lh_map_one( (void*)loadee->entry_pt, load_list_head );
}


static void
demand_segv_handler( int sig, siginfo_t* si, void* unused )
{
  // Make sure that the faulting address is within bounds
  if ( lm_validate_address( &(loadee->bounds), (uint64_t)si->si_addr ) != 0 ) {
    fprintf( stderr, "Segmentation fault caused by invalid address\n" );
    exit( -1 );
  }
  
  lh_map_one( si->si_addr, load_list_head );
}
