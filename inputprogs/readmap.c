#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAXLINE 1000

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
  return 0;
}

int
main( )
{
  read_maps();
  printf( "Printing this other thing\n" );
  return 0;
}
