#ifndef _LOADER_H
#define _LOADER_H

#include <elf.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define INITIAL_STACK_SIZE (131072)
//#define MAX_SIZE 1000

#define LOAD_START (0x700000)
#define STACK_BASE (0x600000000000)

#define HDR_BUF_SIZE (128)

struct mem_bounds {
  uint64_t start_addr;  // start of mem region
  uint64_t end_addr;    // end of mem region
};

// TODO: SOMEONE MUST CLEAN  THIS STRUCT UP (free prog headers, close file)

typedef struct {
  char buf[HDR_BUF_SIZE];  // buffer to hold elf header
  struct mem_bounds bounds;  // start of text segment + base of stack
  uint64_t sp;  // top of stack
  uint64_t entry_pt;  // entry point address 
  //int argc;  // number of arguments
  //int envc;  // number of environment variables
  const char* filename;  // name of executable
  int fd; // file descriptor for open file
} Loadee_mgmt;

Loadee_mgmt* loader_get_new_manager( char** argv );
void loader_start_loadee( Loadee_mgmt* loadee );


#endif
