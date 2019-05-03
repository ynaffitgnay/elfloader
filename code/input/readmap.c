#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAXLINE 1000
#define PG_SIZE 4096

int hundred_page_array[102400];

int
read_maps( void ) {
  FILE* fp = NULL;
  char str[MAXLINE];
  char* filename = "/proc/self/maps";

  fp = fopen (filename, "r");
  if (fp == NULL)
  {
    printf ("Couldn't open file\n");
    return 1;
  }

  while (fgets(str, MAXLINE, fp) != NULL)
  {
    printf ("%s", str);
  }

  fclose (fp);

  printf( "\n\n" );
  return 0;
}

int
main( ) {
  size_t size = (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)4; 
  read_maps();
  
  printf( "mallocing small (less than 1GB) region.\n" );

  if (malloc( 4097 ) == NULL) {
  fprintf( stderr,"Unable to malloc small region\n" );
    return -1;
  }
  read_maps();
  
  printf( "mallocing large (greater than 1GB) region.\n" );
  if (malloc( size ) == NULL) {
  fprintf( stderr,
    "Unable to malloc large region\n" );
    return -1;
}
  read_maps();
  
  
  return 0;
}
