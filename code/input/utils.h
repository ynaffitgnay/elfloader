#ifndef _LDR_UTILS_H_
#define _LDR_UTILS_H_

#include <sys/resource.h>

#define MAXLINE 1000

int
lu_print_maps( void );

int
lu_print_statm( void);

void
lu_print_rusage_diff( struct rusage* s_usage, struct rusage* e_usage );

#endif
