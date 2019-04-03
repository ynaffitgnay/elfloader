#include <inttypes.h>
#include <stdio.h>

#include "loader_utils.h"

int main( int argc, char** argv, char** envp )
{
  printf( "argv: %#" PRIx64
          "\t contents: %s\n", (uint64_t)argv, *argv );
  ++argv;
  printf( "++argv: %#" PRIx64
          "\t contents: %s\n", (uint64_t)argv, *argv );

          
  return 0;
}
