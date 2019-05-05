#include <stdlib.h>
#include <stddef.h> 
#include <stdio.h>
#include <sys/time.h>

#include "loader_utils.h"

int
lu_print_maps( void )
{
  FILE* fp = NULL;
  char str[MAXLINE];
  char* filename = "/proc/self/maps";

  fp = fopen (filename, "r");
  if (fp == NULL)
  {
    printf ("Couldn't open file\n");
    return -1;
  }

  while (fgets(str, MAXLINE, fp) != NULL)
  {
    printf ("%s", str);
  }

  fclose (fp);
  printf( "\n\n\n" );
  return 0;
}


int
lu_print_statm( void )
{
  FILE* fp = NULL;
  char str[MAXLINE];
  char* filename = "/proc/self/statm";

  fp = fopen (filename, "r");
  if (fp == NULL)
  {
    printf ("Couldn't open file\n");
    return -1;
  }


  printf("Mem usage !! " );
  while (fgets(str, MAXLINE, fp) != NULL)
  {
    printf ("%s", str);
  }

  fclose (fp);
  printf( "\n\n" );
  return 0;
}


void
lu_print_rusage_diff( struct rusage* s_usage, struct rusage* e_usage )
{
  long utime_s, utime_us, stime_s, stime_us, ttime_s, ttime_us;
  //printf ("Diff fields of interest: \n");

  utime_s = e_usage->ru_utime.tv_sec - s_usage->ru_utime.tv_sec;
  utime_us = e_usage->ru_utime.tv_usec - s_usage->ru_utime.tv_usec;
  
  if (utime_us < 0)
  {
    --utime_s;
    utime_us += 1000000;
  }

  stime_s = e_usage->ru_stime.tv_sec - s_usage->ru_stime.tv_sec;
  stime_us = e_usage->ru_stime.tv_usec - s_usage->ru_stime.tv_usec;
  
  if (stime_us < 0)
  {
    --stime_s;
    stime_us += 1000000;
  }

  ttime_s = utime_s + stime_s;
  ttime_us = utime_us + stime_us;
  
  if (ttime_us >= 1000000)
  {
    ++ttime_s;
    ttime_us -= 1000000;
  }
      
  
  printf( "utime | %ld.%06ld | %ld.%06ld | %ld.%06ld\n", 
         s_usage->ru_utime.tv_sec, s_usage->ru_utime.tv_usec,
         e_usage->ru_utime.tv_sec, e_usage->ru_utime.tv_usec,
         utime_s, utime_us);  
  
  printf( "stime | %ld.%06ld | %ld.%06ld | %ld.%06ld\n", 
         s_usage->ru_stime.tv_sec, s_usage->ru_stime.tv_usec,
         e_usage->ru_stime.tv_sec, e_usage->ru_stime.tv_usec,
         stime_s, stime_us);

  printf( "ttime | %ld.%06ld | %ld.%06ld | %ld.%06ld\n", 
          ttime_s, ttime_us, ttime_s, ttime_us, ttime_s, ttime_us );  
  
  printf( "maxrss | %ld | %ld | %ld\n",  s_usage->ru_maxrss,
         e_usage->ru_maxrss, e_usage->ru_maxrss - s_usage->ru_maxrss);

  // These are not maintained
  /*
  printf( "ixrss | %ld | %ld | %ld\n",  s_usage->ru_ixrss,
         e_usage->ru_ixrss, e_usage->ru_ixrss - s_usage->ru_ixrss);

  printf( "idrss | %ld | %ld | %ld\n",  s_usage->ru_idrss,
         e_usage->ru_idrss, e_usage->ru_idrss - s_usage->ru_idrss);

  printf( "isrss | %ld | %ld | %ld\n",  s_usage->ru_isrss,
          e_usage->ru_isrss, e_usage->ru_isrss - s_usage->ru_isrss);
  */
          
  printf( "minflt | %ld | %ld | %ld\n",  s_usage->ru_minflt,
         e_usage->ru_minflt, e_usage->ru_minflt - s_usage->ru_minflt);

  printf( "majflt | %ld | %ld | %ld\n",  s_usage->ru_majflt,
         e_usage->ru_majflt, e_usage->ru_majflt - s_usage->ru_majflt);  

  printf( "inblock | %ld | %ld | %ld\n",  s_usage->ru_inblock,
         e_usage->ru_inblock, e_usage->ru_inblock - s_usage->ru_inblock);
 
  printf( "oublock | %ld | %ld | %ld\n",  s_usage->ru_oublock,
         e_usage->ru_oublock, e_usage->ru_oublock - s_usage->ru_oublock);  

  printf( "nvcsw | %ld | %ld | %ld\n",  s_usage->ru_nvcsw,
         e_usage->ru_nvcsw, e_usage->ru_nvcsw - s_usage->ru_nvcsw);  

  printf( "nivcsw | %ld | %ld | %ld\n",  s_usage->ru_nivcsw,
         e_usage->ru_nivcsw, e_usage->ru_nivcsw - s_usage->ru_nivcsw);

  lu_print_statm();
  
}
