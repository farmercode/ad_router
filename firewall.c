#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "firewall.h"
#include "iptables.h"
#include "fw_socket.h"
#include "fw_thread.h"
#include "config.h"
#include "client_list.h"

#define BUFFER_LEN 2048

#define GET_ARRAY_LEN(array,len){len = (sizeof(array) / sizeof(array[0]));}

int gw_sock;

FC fw_config;

int client_sock,sin_size;

int main(int argc, char **argv)
{
	int sock_err;
	int recv_len;
	int sin_size;
	char buffer[BUFFER_LEN];
	char *msg="欢迎到来！";
	pid_t pid;
	int al;

	get_cmd_opt(argc,argv);
	debug_printf("after get cmd opt\n");
        fw_config = get_current_config();

	if(fw_config.mode){
            pid = fork();
            if(pid == 0){
                daemon_init();
                main_loop();
            }else if(pid > 0){
                exit(0);
            }else{
                printf("fork失败！");
                exit(1);
            }
	}else{
            main_loop();
	}
	exit(0);
}

void main_loop(){	
	sin_size = sizeof(struct sockaddr_in);	
	pthread_t client_handler_t,update_client_thread;
	void *thread_result;
	struct thread_param t_param;

        init_config();

	sig_init();
	gw_sock = create_server(GW_PORT);

	/*删除程序上次遗留防火墙信息*/
	fw_iptables_destroy();
	/*初始化防火墙信息*/
	fw_iptables_init();

        init_client_mutex();

	/*更新用户数据线程*/
	pthread_create(&update_client_thread,NULL, update_client_data_thread,NULL);
	pthread_detach(update_client_thread);

	while(1){
		client_sock = accept(gw_sock, (struct sockaddr *)&client_ip, &sin_size);
		t_param.sock=client_sock;
		t_param.client_ip = client_ip;
		if(client_sock != -1){
			pthread_create(&client_handler_t,NULL,(void *)gw_handler,(void *)&t_param);
			pthread_detach(client_handler_t);
		}
	}
}

void fw_exit(int signo){
	fw_iptables_destroy();
	close(gw_sock);
	exit(signo);
}

/**
 * 信号初始化
 */
void sig_init(){
	struct sigaction sa;

	/*处理SIGTERM信号*/
	sa.sa_handler = fw_exit;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGTERM,&sa,NULL) == -1)
	{
		printf("SIGTERM捕捉失败!\n");
		exit(1);
	}

	if(sigaction(SIGQUIT,&sa,NULL) == -1){
		printf("SIGQUIT!\n");
		exit(1);
	}

	if(sigaction(SIGINT,&sa,NULL)==-1){
		printf("SIGINT捕捉失败！\n");
		exit(1);
	}

}
