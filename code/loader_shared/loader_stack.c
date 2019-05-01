#include <elf.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "loader_stack.h"
#include "loader_elf.h"

static int populate_info_block( struct loader_stack_info* lsinfo, Loadee_mgmt* loadee,
                                char** out_argv, char** out_envp, char** execfn_addr );
static int copy_args( char** in_argv, int argc, Loadee_mgmt* loadee, char** out_argv );

int ls_setup_stack( struct loader_stack_info* lsinfo, Loadee_mgmt* loadee ) {
  void* newstack = NULL;
  int argv_size;
  char** temp_argv = NULL;
  int num_env_vars;
  int envp_size;
  char* auxv_addr = NULL;
  char** temp_envp = NULL;
  int* argc_ptr = NULL;
  char* execfn = NULL;
  

  // Allocate room for the stack
  newstack = mmap( (void*)loadee->sp, INITIAL_STACK_SIZE,
                   (PROT_READ | PROT_WRITE),
                   (MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN), -1, 0 );

  // TODO: maybe allow stack to move? just have to update bounds and sp in loadee
  if ((uint64_t)newstack != loadee->sp) {
    fprintf( stderr, "Unable to allocate stack in correct location\n" );
    munmap( newstack, INITIAL_STACK_SIZE );
    return -1;
  }
  
  // Create vector to hold pointers to each argument in the loadee stack
  argv_size = (lsinfo->argc - 1) * sizeof( char* );
  temp_argv = (char**)malloc( argv_size );
  if (temp_argv == NULL) {
    fprintf( stderr, "Unable to allocate argument vector\n" );
    return -1;
  }

  //TODO: erase this
  // Make sure envp isn't getting changed by this function inadverdently 
  printf( "Original envp: %#" PRIx64 " ", (uint64_t)lsinfo->envp );
  auxv_addr = ls_get_auxv_addr( lsinfo->envp, &num_env_vars );
  printf( "Final envp: %#" PRIx64 " \n", (uint64_t)lsinfo->envp);

  lsinfo->envc = num_env_vars;
  
  // Create vector to hold pointers to each environment variable in the loadee stack
  envp_size = num_env_vars * sizeof( char* );
  temp_envp = (char**)malloc( num_env_vars * sizeof( char* ) );
  if (temp_envp == NULL) {
    fprintf( stderr, "Unable to allocate environment vector\n" );
    free( temp_argv );
    return -1;
  }

  // populate_info_block does alignment
  populate_info_block( lsinfo, loadee, temp_argv, temp_envp, &execfn );


  // TODO: need to propogate execfn addr to program
  le_create_elf_tables( loadee, (Elf64_auxv_t*)auxv_addr );

  // Insert null word between aux vector and envp
  loadee->sp -= 8;
  if ((uint64_t)memset( (void*)loadee->sp, 0, 8) != loadee->sp ) {
    fprintf( stderr, "Error inserting null word in between auxv and envp\n" );
    free( temp_argv );
    free( temp_envp );
    return -1;
  }

  loadee->sp -= envp_size;
  if ((uint64_t)memcpy( (void*)loadee->sp, temp_envp, envp_size ) != loadee->sp ) {
    fprintf( stderr, "Error copying envp into stack\n" );
    free( temp_argv );
    free( temp_envp );
    return -1;
  }

  free( temp_envp );

  // Insert null word between envp and argv
  loadee->sp -= 8;
  if ((uint64_t)memset( (void*)loadee->sp, 0, 8) != loadee->sp ) {
    fprintf( stderr, "Error inserting null word in between envp and argv\n" );
    free( temp_argv );
    return -1;
  }

  loadee->sp -= argv_size;
  if ((uint64_t)memcpy( (void*)loadee->sp, temp_argv, argv_size ) != loadee->sp ) {
    fprintf( stderr, "Error copying envp into stack\n" );
    free( temp_argv );
    return -1;
  }

  free( temp_argv );

  // Put argc on the stack
  loadee->sp -= 8;
  argc_ptr = (int*)(loadee->sp);

  *argc_ptr = lsinfo->argc - 1;  // subtract 1 for the original loader name
  
  
  return 0;
}

char* ls_get_auxv_addr( char** envp, int* num_env_vars ) {
  int envc = 0;
  
  while (*envp++ != NULL) ++envc;

  printf( "Number of environment vars: %d\n", envc );
  
  if (num_env_vars != NULL) *num_env_vars = envc;
  
  return (char*)envp;
}


int populate_info_block( struct loader_stack_info* lsinfo, Loadee_mgmt* loadee,
                         char** out_argv, char** out_envp, char** execfn_addr ) {

  // MAKE SURE WHEN CALLING COPY_ARGS WITH ARGV, INCREMENT ARGV BY 1 AND
  // DECREMENT ARGC BY 1
  uint64_t loadee_argc;
  char** loadee_argv = NULL;
  int alignment;

  loadee_argc = lsinfo->argc - 1;
  loadee_argv = lsinfo->argv + 1;

  // Copy the name of the program into the stack
  // JK DON'T DO IT THIS WAY
  copy_args( loadee_argv, 1, loadee, execfn_addr );

  // Copy the environment variables onto the stack
  copy_args( lsinfo->envp, lsinfo->envc, loadee, out_envp );

  // Copy the arguments onto the stack
  copy_args( loadee_argv, loadee_argc, loadee, out_argv );

  // MAKE SURE TO DO ALIGNMENT
  alignment = (8 - (loadee->sp % 8)) * ((loadee->sp % 8) && 1);
  
  loadee->sp -= alignment;

  if (loadee->sp % 8 != 0) {
    fprintf( stderr, "You seem to be aligning incorrectly.\n" );
  }

  if ((uint64_t)memset( (void*)loadee->sp, 0, alignment ) != loadee->sp) {
    fprintf( stderr, "Error memsetting stack alignment\n" );
  }
  
  return 0;
}
  
int copy_args( char** in_argv, int argc, Loadee_mgmt* loadee, char** out_argv ) {
  const char* str = NULL;
  size_t len;
  char* sp = (char*) loadee->sp;
  
  while( argc-- > 0 ) {
    str = in_argv[ argc ];
    len = strlen( str ) + 1;  // include 1 for the null terminator

    sp -= len;

    if (memcpy( sp, str, len ) != sp) {
      fprintf(stderr, "Error copying argument %d to the argument array\n", argc );
    }

    out_argv[ argc ] = sp;
  }
  
  loadee->sp = (uint64_t)sp;
  return 0;
}
