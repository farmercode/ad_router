#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include "config.h"

#define MAXFD 64

int get_current_time(){
	static int now;
	now = time((time_t*)NULL);
	return now;
}

void debug(char *format,...){
	FC config;
	config = get_current_config();
	if(config.debug_mode){
		va_list vl;
		va_start(vl,format);
		vprintf(format,vl);
		va_end(vl);
	}
}

void debug_printf(char *info){
    FC config;
    config = get_current_config();
    if(config.debug_mode){
           printf("%s\r\n", info);
    }
}

void daemon_init(){

    int i;
    /*改变工作目录*/
    //chdir("/");
    /*关闭常用描述副*/
    for (i=0;i<MAXFD;i++) {
        close(i);
    }
    /*将stdin,stdout,stderr重定向到/dev/null*/
    open("/dev/null",O_RDONLY);
    open("/dev/null",O_RDWR);
    open("/dev/null",O_RDWR);
}

void safe_free(void *p){
    free(p);
    if (p !=NULL) {
        p = NULL;
    }
}
