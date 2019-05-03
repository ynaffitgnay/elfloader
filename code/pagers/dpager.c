#define _GNU_SOURCE  // to get mmap macrosx
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/auxvec.h>

#include "loader.h"
#include "loader_elf.h"
#include "loader_mem.h"
#include "loader_stack.h"
#include "loader_utils.h"

int demand_get_segments( Loadee_mgmt* loadee, Elf_info* ei ) {
  Elf64_Phdr* phdr_it = ei->phdrs;
  for (int i = 0; i < ei->hdr->e_phnum; ++i) {
    int prot = 0;
    int flags = MAP_PRIVATE;
    if (phdr_it->p_type == PT_LOAD) {
      struct mappable_mem_region file_backed_seg;
      // Always map the file backed part
      file_backed_seg.real_start = phdr_it->p_vaddr;
      file_backed_seg.length = phdr_it->p_filesz;
      file_backed_seg.real_end = phdr_it->p_vaddr + phdr_it->p_filesz; 
      file_backed_seg.fd = loadee->fd;
      file_backed_seg.offset = phdr_it->p_offset;
      
      if (phdr_it->p_flags & PF_X) prot = prot | PROT_EXEC;
      if (phdr_it->p_flags & PF_W) prot = prot | PROT_WRITE;
      if (phdr_it->p_flags & PF_R) prot = prot | PROT_READ;

      flags = flags | MAP_POPULATE;

      file_backed_seg.protection = prot;
      file_backed_seg.flags = flags;

      //printf( "Mapping a file-backed segment\n" );
      if ( lm_define_memregion( &file_backed_seg, 0 ) != 0 ) {
        fprintf( stderr, "Failed to mmap file-backed segment\n" );
        return -1;
      }
      
      if (phdr_it->p_memsz > phdr_it->p_filesz) {
        struct mappable_mem_region anonymous_seg;
        size_t first_section_bytes;
        size_t total_sector_bytes;
        size_t last_section_bytes;

        // Map the bss
        // protection stays the same
        flags = MAP_PRIVATE | MAP_ANONYMOUS;  // re-initialize flags

        first_section_bytes = lm_calc_mmap_length( file_backed_seg.real_start,
                                                   file_backed_seg.length );
        if (first_section_bytes != file_backed_seg.map_size)
          fprintf( stderr, "Behavior you didn't expect from mapper...\n" );
        
        total_sector_bytes = lm_calc_mmap_length( file_backed_seg.real_start,
                                                  phdr_it->p_memsz );
        last_section_bytes = total_sector_bytes - first_section_bytes;

        anonymous_seg.real_start = (uint64_t)file_backed_seg.map_end;
        anonymous_seg.length = last_section_bytes;
        anonymous_seg.protection = file_backed_seg.protection;
        anonymous_seg.flags = flags;
        anonymous_seg.fd = -1;
        anonymous_seg.offset = 0;

        //printf( "Mapping an anonymous segment\n" );
        if (lm_define_memregion( &anonymous_seg, 0 ) != 0) {
          fprintf( stderr, "Failed to map anonymous segment\n" );
          return -1;
        }
      }
      
    }
    
    phdr_it++;  
  }

  return 0;
}
 

int demand_get_elf_binary( Loadee_mgmt* loadee ) { //int argc, char** argv, char** envp ) {
  if (loadee == NULL) {
    printf ("Invalid loadee\n");
    return -1;
  }

  Elf_info ei;
  
  le_get_elfinfo( loadee, &ei );

  if ( demand_get_segments( loadee, &ei ) != 0) {
    fprintf( stderr, "Failed to load segments properly\n" );
    exit( -1 );
  }

  free (ei.phdrs);
  return 0;
}

int main( int argc, char** argv, char** envp ) {
  
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

  if (demand_get_elf_binary( loadee ) != 0) {
    fprintf( stderr, "Failed to load elf binary\n" );
    return -1;    
  }


  if (ls_setup_stack( &apager_info, loadee ) != 0 ) {
    fprintf( stderr, "Failed to set up stack\n" );
    return -1;
  }
  //lu_print_maps();

  // Clean up data structures
  close( loadee->fd );
  sp = loadee->sp;
  ept = loadee->entry_pt;
  free( loadee );
  
  loader_start_loadee( sp, ept );
    
  return 0;
  
}
