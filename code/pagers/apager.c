#define _GNU_SOURCE  // to get mmap macrosx
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/auxvec.h>

#include "loader.h"
#include "loader_elf.h"
#include "loader_mem.h"
#include "loader_stack.h"
#include "loader_utils.h"

int all_load_segments( Loadee_mgmt* loadee, Elf_info* ei ) {
  Elf64_Phdr* phdr_it = ei->phdrs;
  
  for (int i = 0; i < ei->hdr->e_phnum; ++i) {
    int prot = 0;
    int flags = MAP_PRIVATE;
    if (phdr_it->p_type == PT_LOAD) {
      struct mem_region file_backed_seg;
      // Always map the file backed part
      file_backed_seg.virt_address = phdr_it->p_vaddr;
      file_backed_seg.length = phdr_it->p_filesz;
      file_backed_seg.fd = loadee->fd;
      file_backed_seg.offset = phdr_it->p_offset;
      
      if (phdr_it->p_flags & PF_X) prot = prot | PROT_EXEC;
      if (phdr_it->p_flags & PF_W) prot = prot | PROT_WRITE;
      if (phdr_it->p_flags & PF_R) prot = prot | PROT_READ;

      flags = flags | MAP_POPULATE;

      file_backed_seg.protection = prot;
      file_backed_seg.flags = flags;

      printf( "Mapping a file-backed segment\n" );
      if ( lm_map_memregion( &file_backed_seg ) != 0 ) {
        fprintf( stderr, "Failed to mmap file-backed segment\n" );
        return -1;
      }
      
      if (phdr_it->p_memsz > phdr_it->p_filesz) {
        struct mem_region anonymous_seg;
        size_t first_section_bytes;
        size_t total_sector_bytes;
        size_t last_section_bytes;

        // Map the bss
        // protection stays the same
        flags = MAP_PRIVATE | MAP_ANONYMOUS;  // re-initialize flags

        first_section_bytes = lm_calc_mmap_length( file_backed_seg.virt_address,
                                                   file_backed_seg.length );
        if (first_section_bytes != file_backed_seg.map_size)
          fprintf( stderr, "Behavior you didn't expect from mapper...\n" );
        
        total_sector_bytes = lm_calc_mmap_length( file_backed_seg.virt_address,
                                                  phdr_it->p_memsz );
        last_section_bytes = total_sector_bytes - first_section_bytes;

        anonymous_seg.virt_address = (uint64_t)file_backed_seg.map_end;
        anonymous_seg.length = last_section_bytes;
        anonymous_seg.protection = file_backed_seg.protection;
        anonymous_seg.flags = flags;
        anonymous_seg.fd = -1;
        anonymous_seg.offset = 0;

        printf( "Mapping an anonymous segment\n" );
        if (lm_map_memregion( &anonymous_seg ) != 0) {
          fprintf( stderr, "Failed to map anonymous segment\n" );
          return -1;
        }
      }
      
    }
    
    phdr_it++;  
  }
  return 0;
}
 

int all_load_elf_binary( Loadee_mgmt* loadee ) { //int argc, char** argv, char** envp ) {
  if (loadee == NULL) {
    printf ("Loadee failed, i guess\n");
  }

  Elf_info ei;
  
  le_get_elfinfo( loadee, &ei );

  if ( all_load_segments( loadee, &ei ) != 0) {
    fprintf( stderr, "Failed to load segments properly\n" );
    return -1;
  }

  // Free program headers?
  return 0;
}

int main( int argc, char** argv, char** envp ) {
  if (argc < 2) {
    fprintf( stderr, "Please supply an additional argument\n" );
    return 0;
  }

  Loadee_mgmt* loadee = loader_get_new_manager( argv );
  struct loader_stack_info apager_info = { argc, argv, 0, envp, 19, NULL };

  lu_print_maps();

  // close file when you exit
  
  all_load_elf_binary( loadee );

  ls_setup_stack( &apager_info, loadee ); 
  lu_print_maps();
  
  
  loader_start_loadee( loadee );

    
  return 0;

  Elf64_auxv_t *auxv = NULL;
  printf( "argv: %#" PRIx64
          "\t contents: %s\n", (uint64_t)argv, *argv );
  ++argv;
  printf( "++argv: %#" PRIx64
          "\t contents: %s\n", (uint64_t)argv, *argv );

  FILE * fp = NULL;
  char buf[HDR_BUF_SIZE];
  if (argc > 1) {
    fp = fopen ((const char*)(*argv), "r");
    fread( buf, 1, HDR_BUF_SIZE, fp );
    fclose( fp );

    argc++;
  }

  printf( "contents: %s", buf );

  char** orig_envp = envp;
  int num_env_vars = 0;

  // Use do-while loop to see environment var output
  /*
  do {
    printf( "env var: %s\n", *envp );
    num_env_vars++;
  } while (*envp++ != NULL);
  num_env_vars -= 1;  // account for the extra null in do-while
  */
  
  while (*envp++ != NULL) ++num_env_vars;

  char* final_string_addr = orig_envp[num_env_vars - 1];
  unsigned long chars_in_final_string = strlen(final_string_addr);
  printf("strlen of last value @ envp: %lu\n", chars_in_final_string );

  printf("address of final_string_addr + strlen = %lx\n",
         (unsigned long)final_string_addr + chars_in_final_string );
  
  // this should give address to execfn (includes null terminator)
  printf("address of final_string_addr + strlen + 1 = %lx\n",
         (unsigned long)final_string_addr + chars_in_final_string + 1 );


  printf( "total number of environment variables: %d\n",
          num_env_vars );
  
  char* ptr = (char*) envp;
  printf("sp: %#" PRIx64
         " end o auxv: %#" PRIx64
         " \n", (uint64_t)ptr, (uint64_t)(ptr + 19 * sizeof( Elf64_auxv_t )));

  ptr += 19 * sizeof( Elf64_auxv_t );
  printf("length of random bytes + cap string: %lu\n", strlen(ptr));

  /* see which entries we have */

  for (auxv = (Elf64_auxv_t*)envp; auxv->a_type != AT_NULL; auxv++) {
    printf("aux type: %" PRIu64
           "\n", auxv->a_type );
    if (auxv->a_type == AT_EXECFN) {
      printf("    exec addr: %#" PRIx64 " \n contents: %s\n",
             (uint64_t)(auxv->a_un.a_val), (char*)(auxv->a_un.a_val));
      int execfn_len = strlen( (char*)(auxv->a_un.a_val) );
      printf("        strlen: %d\n", execfn_len );
    } else if (auxv->a_type == AT_PHDR) {
      printf("    phdr addr: %#" PRIx64 "\n", (uint64_t)(auxv->a_un.a_val) );
    }
  }

  char buffer[17];
  char* sp = (char*)(++auxv);  // end of auxiliary vector (after null)
  strncpy(buffer, sp, 16);
  buffer[16] = '\0';  
  printf("sp: %#" PRIx64 " random bytes:  %s \n", (uint64_t)sp, buffer);
  sp += 16;
  printf("sp: %#" PRIx64 " capability string? %s \n", (uint64_t)sp, sp);
  //printf("capability string now? %s \n", *(char*)(auxv++));

  lu_print_maps();

  char* newstack = mmap( (void*)0x600000000000, INITIAL_STACK_SIZE, 
                         PROT_READ |  PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);

  if (newstack) {
    printf("Mapped a stack! \n");
  } else {
    perror( "Unable to mmap for stack" );
  }

  lu_print_maps();
  
  return 0;
}
