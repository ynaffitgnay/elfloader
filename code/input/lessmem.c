#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "utils.h"

#define MAXLINE 1000
#define PG_SIZE 4096

int hundred_page_array[102400];

int
main( ) {
  struct rusage usage_start, usage_end;
  
  if (getrusage( RUSAGE_SELF, &usage_start ) < 0)
  {
    perror( "getrusage unsuccessful.");
    exit( -1 );
  }

  // Touch one entry
  hundred_page_array[ 57 * PG_SIZE / sizeof( int ) ] = 43;

  if (getrusage( RUSAGE_SELF, &usage_end ) < 0)
  {
    perror( "getrusage unsuccessful.");
    exit( -1 );
  }

  lu_print_rusage_diff( &usage_start, &usage_end );
  //lu_print_maps();
  //lu_print_statm();
  
  return 0;
}
