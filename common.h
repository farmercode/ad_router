
#ifndef _COMMON_H_
#define _COMMON_H_
int get_current_time();

void debug(char *format,...);

void debug_printf(char *info);

void daemon_init();

void safe_free(void *p);

#endif /*end of common.h*/
