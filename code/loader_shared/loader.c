#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "loader.h"

// Returns NULL if unsuccessful
Loadee_mgmt* loader_get_new_manager( char** argv ) {
  Loadee_mgmt* new_loadee = NULL;

  new_loadee = (Loadee_mgmt*)malloc( sizeof( Loadee_mgmt ) );

  if (!new_loadee) {
    fprintf( stderr, "Unable to allocate memory for Loadee_mgmt\n" );
    return NULL;
  }

  new_loadee->filename = argv[1];  // require first entry to be loadee name
  new_loadee->fd = open(new_loadee->filename, O_RDONLY);

  if (new_loadee->fd < 0) {
    perror( "Failed to open loadee file" );
    free( new_loadee );
    return NULL;
  }

  if (read( new_loadee->fd, (void*)(new_loadee->buf), HDR_BUF_SIZE ) < HDR_BUF_SIZE) {
    fprintf( stderr, "Input file may not be binary\n" );
    // TODO: decide whether or not should fail here
    //close( new_loadee->fd );
    //free( new_loadee );
    //return NULL;
  }

  // overshoot start_addr in order to set it in elf loader
  new_loadee->bounds.start_addr = (uint64_t)LOAD_START;
  new_loadee->bounds.end_addr = (uint64_t)STACK_BASE - 1;
  new_loadee->sp = (uint64_t)STACK_BASE - 1;  
  
  return new_loadee;

}

void loader_start_loadee( Loadee_mgmt* loadee ) {
  // set regs
  // free loadee_mgmt (try not freeing first in case you accidentally refer to stuff in loadee_mgmt)
  // maybe try memsetting all of loadee_mgmt with 0

  // store entry point and stack pointer locally
  uint64_t ept = loadee->entry_pt;
  uint64_t sp = loadee->sp;

  asm( "movq %0, %%rsp;"
       :
       : "g" (sp)
       :  "rsp"
    );
  
  asm( "movq %0, %%rax;"
       :
       : "g" (ept)
       : "rax"
    );

  // Move the entry point from rax to rbp, which will be 0'd out at the beginning
  // of _start
  asm( "movq %rax, %rbp ");
  
  asm ( "xor %rax, %rax" );
  asm ( "xor %rbx, %rbx" );
  asm ( "xor %rcx, %rcx" );
  asm ( "xor %rdx, %rdx" );
  asm ( "xor %rsi, %rsi" );
  asm ( "xor %rdi, %rdi" );
  asm ( "xor %r8, %r8" );
  asm ( "xor %r9, %r9" );
  asm ( "xor %r10, %r10" );
  asm ( "xor %r11, %r11" );
  asm ( "xor %r12, %r12" );
  asm ( "xor %r13, %r13" );
  asm ( "xor %r14, %r14" );
  asm ( "xor %r15, %r15" );
    
  asm ( "jmp *%rbp" );
  
}
