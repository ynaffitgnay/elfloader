#ifndef _LOADER_ELF_H_
#define _LOADER_ELF_H_

#include <elf.h>

typedef struct
{
  Elf64_Ehdr hdr;  /* Actual ELF header */
  Elf64_Phdr* phdr; /* Pointer to first entry in program header table */
  Elf64_Shdr* shdr; /* Pointer to first entry in section header table */
  
} Elf_info;

int
le_get_elfinfo( );

//int
//le_load_elf_binary( );

int
le_load_elf_phdrs( );

int
le_load_elf_shdrs( );

int
le_map_section( int section );
#endif
