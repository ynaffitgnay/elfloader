#ifndef _LOADER_H
#define _LOADER_H

#include <elf.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LOAD_START (0x700000)
#define STACK_BASE (0x600000000000)

#define HDR_BUF_SIZE (128)

typedef struct {
  uint64_t start_addr;  // start of mem region
  uint64_t end_addr;    // end of mem region
} Mem_bounds;

// TODO: SOMEONE MUST CLEAN  THIS STRUCT UP (free prog headers, close file)

typedef struct {
  char buf[HDR_BUF_SIZE];  // buffer to hold elf header
  Mem_bounds bounds;  // start of text segment + base of stack
  uint64_t sp;  // top of stack
  uint64_t entry_pt;  // entry point address 
  const char* filename;  // name of executable
  int fd; // file descriptor for open file
} Loadee_mgmt;

Loadee_mgmt* loader_get_new_manager( char** argv );
void loader_start_loadee( uint64_t sp, uint64_t entry_pt );


#endif
