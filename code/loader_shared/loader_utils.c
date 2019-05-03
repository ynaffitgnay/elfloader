#include <stdlib.h>
#include <stddef.h> 
#include <stdio.h>
#include <sys/time.h>

#include "loader_utils.h"

int
lu_print_maps( void ) {
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

void
lu_print_rusage_diff( struct rusage* s_usage, struct rusage* e_usage )
{
  long utime_s, utime_us, stime_s, stime_us;
  printf ("Diff fields of interest: \n");

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

  printf ("\tutime:\t%ld.%06ld\t%ld.%06ld",
          s_usage->ru_utime.tv_sec, s_usage->ru_utime.tv_usec,
          e_usage->ru_utime.tv_sec, e_usage->ru_utime.tv_usec);
  printf ("\tdiff:\t%ld.%06ld\n", utime_s, utime_us);
  
  
  printf( "utime | %ld.%06ld | %ld.%06ld | %ld.%06ld", 
         s_usage->ru_utime.tv_sec, s_usage->ru_utime.tv_usec,
         e_usage->ru_utime.tv_sec, e_usage->ru_utime.tv_usec,
         utime_s, utime_us);
  

  printf ("\tstime:\t%ld.%06ld\t%ld.%06ld",
          s_usage->ru_stime.tv_sec, s_usage->ru_stime.tv_usec,
          e_usage->ru_stime.tv_sec, e_usage->ru_stime.tv_usec);
  printf ("\tdiff:\t%ld.%06ld\n", stime_s, stime_us);
  
  
  printf( "stime | %ld.%06ld | %ld.%06ld | %ld.%06ld", 
         s_usage->ru_stime.tv_sec, s_usage->ru_stime.tv_usec,
         e_usage->ru_stime.tv_sec, e_usage->ru_stime.tv_usec,
         stime_s, stime_us);
  

  printf ("\tmaxrss:\t%8ld\t%8ld", s_usage->ru_maxrss, e_usage->ru_maxrss);
  printf ("\tdiff:\t%8ld\n", e_usage->ru_maxrss - s_usage->ru_maxrss);
  

  printf( "maxrss | %ld | %ld | %ld",  s_usage->ru_maxrss,
         e_usage->ru_maxrss, e_usage->ru_maxrss - s_usage->ru_maxrss);
  

  printf ("\tminflt:\t%8ld\t%8ld", s_usage->ru_minflt, e_usage->ru_minflt);
  printf ("\tdiff:\t%8ld\n", e_usage->ru_minflt - s_usage->ru_minflt);
  

  printf( "minflt | %ld | %ld | %ld",  s_usage->ru_minflt,
         e_usage->ru_minflt, e_usage->ru_minflt - s_usage->ru_minflt);
  


  printf ("\tmajflt:\t%8ld\t%8ld", s_usage->ru_majflt, e_usage->ru_majflt);
  printf ("\tdiff:\t%8ld\n", e_usage->ru_majflt - s_usage->ru_majflt);
  

  printf( "majflt | %ld | %ld | %ld",  s_usage->ru_majflt,
         e_usage->ru_majflt, e_usage->ru_majflt - s_usage->ru_majflt);
  

  printf ("\tinblock:\t%4ld\t%8ld", s_usage->ru_inblock, e_usage->ru_inblock);
  printf ("\tdiff:\t%8ld\n", e_usage->ru_inblock - s_usage->ru_inblock);
  

  printf( "inblock | %ld | %ld | %ld",  s_usage->ru_inblock,
         e_usage->ru_inblock, e_usage->ru_inblock - s_usage->ru_inblock);
  


  printf ("\toublock:\t%4ld\t%8ld", s_usage->ru_oublock, e_usage->ru_oublock);
  printf ("\tdiff:\t%8ld\n", e_usage->ru_oublock - s_usage->ru_oublock);
  

  printf( "oublock | %ld | %ld | %ld",  s_usage->ru_oublock,
         e_usage->ru_oublock, e_usage->ru_oublock - s_usage->ru_oublock);
  


  printf ("\tvol switches:\t%4ld\t%8ld", s_usage->ru_nvcsw,
          e_usage->ru_nvcsw);
  printf ("\tdiff:\t%8ld\n", e_usage->ru_nvcsw - s_usage->ru_nvcsw);
  

  printf( "nvcsw | %ld | %ld | %ld",  s_usage->ru_nvcsw,
         e_usage->ru_nvcsw, e_usage->ru_nvcsw - s_usage->ru_nvcsw);
  


  printf ("\tinvol switches:\t%4ld\t%8ld", s_usage->ru_nivcsw,
          e_usage->ru_nivcsw);
  printf ("\tdiff:\t%8ld\n",
          e_usage->ru_nivcsw - s_usage->ru_nivcsw);
  

  printf( "nivcsw | %ld | %ld | %ld",  s_usage->ru_nivcsw,
         e_usage->ru_nivcsw, e_usage->ru_nivcsw - s_usage->ru_nivcsw);
  

  //printf ("Fields of less interest: \n");
  //printf ("\tixrss diff: %ld\n", s_usage->ru_ixrss);
  //printf ("\tidrss diff: %ld\n", s_usage->ru_idrss);
  //printf ("\tisrss diff: %ld\n", s_usage->ru_isrss);
  //printf ("\tnswap diff: %ld\n", s_usage->ru_nswap);
  //printf ("\tmsgsnd diff: %ld\n", s_usage->ru_msgsnd);
  //printf ("\tmsgrcv diff: %ld\n", s_usage->ru_msgrcv);
  //printf ("\tnsignals diff: %ld\n", s_usage->ru_nsignals);
}
