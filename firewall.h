#ifndef _FIREWALL_H
#define _FIREWALL_H
#define GW_PORT 9020

struct thread_param{
	int sock;
	struct sockaddr_in client_ip;
};

struct sockaddr_in client_ip,server_ip;

void main_loop();
void fw_exit(int signo);
void sig_init();

#endif /*end _FIREWALL_H*/
