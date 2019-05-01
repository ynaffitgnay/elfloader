#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#include "loader_elf.h"
#include "loader_mem.h"

int
le_get_elfinfo( Loadee_mgmt* loadee, Elf_info* info ) {
  info->hdr = (Elf64_Ehdr*)loadee->buf;

  if (memcmp( info->hdr->e_ident, ELFMAG, SELFMAG ) != 0) {
    fprintf( stderr, "Input file doesn't appear to be an elf, so... jokes on you\n" );
    return -1;
  }

  if (info->hdr->e_type != ET_EXEC) {
    fprintf( stderr, "Input file is not an executable.\n" );
    return -1;
  }

  if (info->hdr->e_machine != EM_X86_64) {
    fprintf( stderr, "Input executable is built for incorrect architecture.\n" );
    return -1;
  }

  printf( "Oooh loading in an elf\n" );
  
  if ((info->phdrs = le_load_elf_phdrs( loadee, info )) != NULL) {
    printf( "Successfully read in program headers\n" );
  }

  if (le_check_section_addrs( loadee, info ) != 0) {
    fprintf( stderr, "Input program overlaps with loader\n" );
    exit(-1);
  }

  loadee->entry_pt = info->hdr->e_entry;    
  
  return 0;
}

Elf64_Phdr*
le_load_elf_phdrs( Loadee_mgmt* loadee, Elf_info* info ) {
  // Make sure to do checks here
  Elf64_Phdr* phdrs = NULL;
  int phdrs_size = -1;
  
  if (info->hdr->e_phentsize != sizeof( Elf64_Phdr )) {
    fprintf( stderr, "Program header entry size is incorrect.\n" );
    return NULL;
  }
  
  if (info->hdr->e_phnum < 1 ||
      info->hdr->e_phnum > (USHRT_MAX / sizeof( Elf64_Phdr ))) {
    fprintf( stderr, "Number of program headers is... suspect\n" );
    return NULL;
  }

  phdrs_size = sizeof( Elf64_Phdr ) * info->hdr->e_phnum;

  phdrs = (Elf64_Phdr*)malloc( phdrs_size );

  if ( phdrs == NULL ) {
    fprintf( stderr, "Unable to allocate memory for pheaders\n" );
    return NULL;
  }

  if ((lseek( loadee->fd, info->hdr->e_phoff, SEEK_SET )) != info->hdr->e_phoff) {
    fprintf( stderr, "Unsuccessful seek to program headers offset\n" );
    free( phdrs );
    return NULL;
  }
  
  if ((read( loadee->fd, phdrs, phdrs_size )) != phdrs_size ) {
    fprintf( stderr, "Unsuccessful read-in of program headers\n" );
    free( phdrs );
    return NULL;
  }
  
  return phdrs;
}

int
le_check_section_addrs( Loadee_mgmt* loadee, Elf_info* info ) {
  Elf64_Phdr* phdr_it = info->phdrs;
  int first_load = 0;
   
  for (int i = 0; i < info->hdr->e_phnum; ++i) {
    if (lm_validate_address( &(loadee->bounds), phdr_it->p_vaddr ) == -1 ||
        lm_validate_address( &(loadee->bounds), (phdr_it->p_vaddr + phdr_it->p_memsz)) == -1 ) {
      fprintf( stderr, "Failed address verification on %dth section\n", i);
      return -1;
    }

    // If this is the first load segment, after verifying that vaddr falls in
    // default bounds, set start address of bounds to start of the text segment
    if (phdr_it->p_type == PT_LOAD && !first_load) {
      first_load = 1;
      printf( "Setting start bound to %#" PRIx64 "\n", phdr_it->p_vaddr ); 
    
      loadee->bounds.start_addr = phdr_it->p_vaddr;
    }

    ++phdr_it;
  }

  if (!first_load) {
    fprintf( stderr, "First load section never got set\n" );
  }
  
  return 0;
}


int
le_create_elf_tables( Loadee_mgmt* loadee, Elf64_auxv_t* loader_auxv ) {
  uint64_t rand_n_plat_ptr;
  Elf64_auxv_t* loadee_stack_auxv = NULL;
  uint64_t auxv_size;

  Elf64_Ehdr* hdr = (Elf64_Ehdr*)loadee->buf;

  auxv_size = 19 * sizeof( Elf64_auxv_t );

  // Get a pointer to the address of the random number and platform capability string
  rand_n_plat_ptr = (uint64_t)loader_auxv + auxv_size;  

  loadee->sp -= 24;  // Subtract 24 bytes from stack pointer

  // Copy the random number and platform capability string to the loadee stack
  if ((uint64_t)memcpy( (void*)loadee->sp, (void*)rand_n_plat_ptr, 24 ) != loadee->sp) {
    fprintf( stderr, "Error while memcpying platform cap string etc. into stack\n" );
  }

  // Move the top of the loadee's stack to the front of a 19-entry auxv array
  loadee->sp -= auxv_size;

  loadee_stack_auxv = (Elf64_auxv_t*)(loadee->sp);

  // Copy the loader's auxv into the loadee's auxv
  if (memcpy( loadee_stack_auxv, loader_auxv, auxv_size ) != loadee_stack_auxv ) {
    fprintf( stderr, "Error while memcpying auxv into stack\n" );
  }
  
  // Correct entries in the auxv
  if (loadee_stack_auxv[4].a_type != AT_PHDR) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 4 );
  }
  
  loadee_stack_auxv[4].a_un.a_val = loadee->bounds.start_addr + hdr->e_phoff;

  if (loadee_stack_auxv[6].a_type != AT_PHNUM) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 6 );
  }
  
  loadee_stack_auxv[6].a_un.a_val = hdr->e_phnum;

  if (loadee_stack_auxv[8].a_type != AT_FLAGS) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 8 );
  }
  
  loadee_stack_auxv[8].a_un.a_val = hdr->e_flags;
  
  if (loadee_stack_auxv[9].a_type != AT_ENTRY) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 9 );
  }
  
  loadee_stack_auxv[9].a_un.a_val = hdr->e_entry;

  if (loadee_stack_auxv[15].a_type != AT_RANDOM) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 15 );
  }
  
  loadee_stack_auxv[15].a_un.a_val = loadee->sp + auxv_size + 1;

  if (loadee_stack_auxv[16].a_type != AT_EXECFN) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 16 );
  }
  
  // Execfn is last value in the info block (so closest to the base of the stack)
  loadee_stack_auxv[16].a_un.a_val = loadee->bounds.end_addr + strlen( loadee->filename ) + 1;

  if (loadee_stack_auxv[17].a_type != AT_PLATFORM) {
    fprintf( stderr, "Unexpected a_type at aux vector entry %d\n", 17 );
  }
  
  loadee_stack_auxv[17].a_un.a_val = loadee_stack_auxv[15].a_un.a_val + 16;
  
  
  return 0;
}
