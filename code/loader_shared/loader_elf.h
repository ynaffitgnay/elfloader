#ifndef _LOADER_ELF_H_
#define _LOADER_ELF_H_

#include <elf.h>

#include "loader.h"

typedef struct
{
  Elf64_Ehdr* hdr;  /* ELF header pointer */
  Elf64_Phdr* phdrs; /* Pointer to first entry in program header table */
  
} Elf_info;


int
le_get_elfinfo( Loadee_mgmt* loadee, Elf_info* info );

Elf64_Phdr*
le_load_elf_phdrs( Loadee_mgmt* loadee, Elf_info* info );

int
le_check_segment_addrs( Loadee_mgmt* loadee, Elf_info* info );

int
le_create_elf_tables( Loadee_mgmt* loadee, Elf64_auxv_t* loader_auxv, int auxv_entries );

#endif
