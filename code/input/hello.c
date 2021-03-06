#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "utils.h"

int main() {
  struct rusage usage_start, usage_end;
  
  if (getrusage( RUSAGE_SELF, &usage_start ) < 0)
  {
    perror( "getrusage unsuccessful.");
    exit( -1 );
  }

  printf( "Hello, world!\n" );
  
  if (getrusage( RUSAGE_SELF, &usage_end ) < 0)
  {
    perror( "getrusage unsuccessful.");
    exit( -1 );
  }

  lu_print_rusage_diff( &usage_start, &usage_end );
  return 0;
}
