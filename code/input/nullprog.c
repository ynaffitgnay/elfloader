#include <stdlib.h>
#include <stdio.h>

int
main() {
  int *zero = malloc( (long)1024 * (long)1024 * (long)1024 * 2 );
  if ( zero == NULL ) {
    printf( "Unable to malloc as much space as I wanted\n" );
    return 0;
  }
  return *zero;
  //return *zero;
}
