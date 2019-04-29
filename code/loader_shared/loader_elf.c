#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "loader_elf.h"

int
le_get_elfinfo( Loadee_mgmt* loadee, Elf_info* info ) {
  info->hdr = (Elf64_Ehdr*)loadee->buf;

  if (memcmp( info->hdr->e_ident, ELFMAG, SELFMAG ) != 0) {
    fprintf( stderr, "Input file doesn't appear to be an elf, so... jokes on you\n" );
    return -1;
  }

  printf( "Oooh loading in an elf\n" );
  
  
  return 0;
}

Elf64_Phdr*
le_load_elf_phdrs( Elf64_Ehdr* elf_hdr ) {
  return NULL;
}

int
le_check_sector_addrs( struct mem_bounds* bounds, Elf_info* info ) {
  return 0;
}


int
le_create_elf_tables( Loadee_mgmt* loadee, Elf64_auxv_t* loader_auxv ) {
  return 0;
}
