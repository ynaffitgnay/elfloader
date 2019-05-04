#define _GNU_SOURCE  // to get mmap macrosx
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/auxvec.h>

#include "loader.h"
#include "loader_elf.h"
#include "loader_handler.h"
#include "loader_mem.h"
#include "loader_stack.h"
#include "loader_utils.h"

static Loadable_segment* load_list_head = NULL;

int
demand_get_segments( Loadee_mgmt* loadee, Elf_info* ei );

int
demand_process_elf_binary( Loadee_mgmt* loadee );

void
demand_map_first_page( Loadee_mgmt* loadee );

int
main( int argc, char** argv, char** envp )
{
  
  if (argc < 2) {
    fprintf( stderr, "Please supply an additional argument\n" );
    return 0;
  }

  Loadee_mgmt* loadee = loader_get_new_manager( argv );
  uint64_t sp;
  uint64_t ept;
  
  // if number of aux_vectors is incorrect, it'll get reset in le_setup_stack
  struct loader_stack_info apager_info = { argc, argv, 0, envp, 19, NULL };

  //lu_print_maps();

  if (demand_process_elf_binary( loadee ) != 0) {
    fprintf( stderr, "Failed to load elf binary\n" );
    return -1;    
  }


  if (ls_setup_stack( &apager_info, loadee ) != 0 ) {
    fprintf( stderr, "Failed to set up stack\n" );
    return -1;
  }
  //lu_print_maps();

  demand_map_first_page( loadee );

  // Clean up data structures
  //close( loadee->fd );  // don't close file b/c still need to perform mappings
  sp = loadee->sp;
  ept = loadee->entry_pt;
  free( loadee );
  
  //loader_start_loadee( sp, ept );
    
  return 0;
  
}

int
demand_get_segments( Loadee_mgmt* loadee, Elf_info* ei )
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

  //lh_print_load_list( load_list_head, 1 );
  return 0;
} 

int
demand_process_elf_binary( Loadee_mgmt* loadee )
{ 
  if (loadee == NULL) {
    printf ("Invalid loadee\n");
    return -1;
  }

  Elf_info ei;
  
  le_get_elfinfo( loadee, &ei );

  if ( demand_get_segments( loadee, &ei ) != 0) {
    fprintf( stderr, "Failed to process segments properly\n" );
    exit( -1 );
  }

  free (ei.phdrs);
  return 0;
}

void
demand_map_first_page( Loadee_mgmt* loadee ) {
  lh_map_one( (void*)loadee->entry_pt, load_list_head );
}

