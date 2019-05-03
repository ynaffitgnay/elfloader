#ifndef _LOADER_HANDLER_H
#define _LOADER_HANDLER_H

#include <elf.h>

#include "loader.h"
#include "loader_mem.h"

enum heuristics { MAP1, MAP2, MAP3 };

typedef struct loadable_segment Loadable_segment;

struct loadable_segment {
  struct mappable_mem_region mmr;
  uint64_t first_page_addr;
  uint64_t last_page_addr;
  size_t pages_to_map;
  size_t pages_mapped;
  Loadable_segment* next;
  Loadable_segment* prev;
};

void lh_insert_segment( Elf64_Phdr* phdr, Loadable_segment* load_list );
Loadable_segment* lh_find_segment( uint64_t addr, Loadable_segment* load_list );
int lh_map_page( uint64_t start_addr, Loadable_segment* parent, int num_pages );
void lh_map_one( void* fault_addr, Loadable_segment* load_list );
void lh_map_two( void* fault_addr, Loadable_segment* load_list );
void lh_map_three( void* fault_addr, Loadable_segment* load_list );


#endif
