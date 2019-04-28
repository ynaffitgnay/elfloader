#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <linux/auxvec.h>

#include "loader.h"
#include "loader_utils.h"
#include "loader_elf.h"

int main( int argc, char** argv, char** envp )
{
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
  do {
    printf( "env var: %s\n", *envp );
    num_env_vars++;
  } while (*envp++ != NULL);

  num_env_vars -= 1;  // account for the extra null

  char* final_string_addr = orig_envp[num_env_vars - 1];
  unsigned long chars_in_final_string = strlen(final_string_addr);
  printf("strlen of last value @ envp: %lu\n", chars_in_final_string );  
  printf("address of final_string_addr + strlen = %lx\n",
         (unsigned long)final_string_addr + chars_in_final_string );

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
  return 0;
}
