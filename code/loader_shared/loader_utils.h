#ifndef _LDR_UTILS_H_
#define _LDR_UTILS_H_

#include <sys/resource.h>

#define MAXLINE 1000

/* Read /proc/self/maps to make mapping of existing memory layout */
int
lu_read_maps( void );  /* UNIMPLEMENTED */

int
lu_validate_addrs( );

int
lu_print_maps( void );

void
lu_print_rusage_diff( int mode, struct rusage* s_usage, struct rusage* e_usage );

#endif
