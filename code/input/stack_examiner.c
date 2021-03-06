#define _GNU_SOURCE  // to get mmap macros
#include <elf.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <linux/auxvec.h>

#include "utils.h"

#define HDR_BUF_SIZE 128
#define NUM_AUXV 20

int main( int argc, char** argv, char** envp )
{
  char** orig_argv = argv;
  Elf64_auxv_t *auxv = NULL;
  int argvIdx = 0;

  struct rusage usage_start, usage_end;
  
  if (getrusage( RUSAGE_SELF, &usage_start ) < 0)
  {
    perror( "getrusage unsuccessful.");
    exit( -1 );
  }

  
  do {
    printf( "argv[%d]: %#" PRIx64
            "\t contents: %s\n", argvIdx++, (uint64_t)argv, *argv );
  } while ( *argv++ != NULL );
    
  FILE * fp = NULL;
  char buf[HDR_BUF_SIZE];
  if (argc > 1) {
    fp = fopen ((const char*)(*orig_argv), "r");
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
         " \n", (uint64_t)ptr, (uint64_t)(ptr + NUM_AUXV * sizeof( Elf64_auxv_t )));

  ptr += NUM_AUXV * sizeof( Elf64_auxv_t );
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
    } else if (auxv->a_type == AT_PLATFORM) {
      printf("    at_plat addr: %#" PRIx64 "\n", (uint64_t)(auxv->a_un.a_val) );
      printf("     and at_plat val: %s\n", (char*)(auxv->a_un.a_val) );
    } else if (auxv->a_type == AT_RANDOM) {
      printf("    rand addr: %#" PRIx64 "\n", (uint64_t)(auxv->a_un.a_val) );
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


  if (getrusage( RUSAGE_SELF, &usage_end ) < 0)
  {
    perror( "getrusage unsuccessful.");
    exit( -1 );
  }

  lu_print_rusage_diff( &usage_start, &usage_end );
  lu_print_maps();

  
  return 0;
}
