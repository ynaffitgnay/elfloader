#ifndef _LOADER_HANDLER_H
#define _LOADER_HANDLER_H

#include <elf.h>

#include "loader.h"
#include "loader_mem.h"

enum heuristics { MAP1, MAP2, MAP3 };

typedef struct loadable_segment Loadable_segment;

struct loadable_segment {
  struct mappable_mem_region mmr;
  uint64_t first_page_addr; // Page-aligned address of the first page in this segment
  uint64_t last_page_addr;  // Page-aligned address of the last page in this segment
  size_t pages_to_map;  // Total number of pages to map
  size_t pages_mapped;  // Pages that have been mapped
  Loadable_segment* next;
  Loadable_segment* prev;
};

Loadable_segment* lh_insert_segment( Loadable_segment* load_list, Elf64_Phdr* phdr, Loadee_mgmt* loadee  );
Loadable_segment* lh_find_parent_segment( Loadable_segment* load_list, uint64_t addr );
int lh_map_pages( uint64_t start_addr, Loadable_segment* parent, int num_pages );
void lh_map_one( void* fault_addr, Loadable_segment* load_list );
void lh_map_two( void* fault_addr, Loadable_segment* load_list );
void lh_map_three( void* fault_addr, Loadable_segment* load_list );
void lh_print_load_list( Loadable_segment* load_list, int print_mmr );

#endif
