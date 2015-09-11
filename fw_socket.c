#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "fw_socket.h"

#define QUEUE_MAX 50

/**
 * 由ip获得Mac地址
 * @param  ip 客户端机器ip地址
 * @return    客户端Mac地址
 */
char *arp_get_mac(char * ip){
	char *mac;
	char tmp_ip[15];
	FILE *proc;
        int num;

        mac =(char *)malloc(18*sizeof(char));
	if((proc = fopen("/proc/net/arp","r")) == NULL){
		printf("打开arp缓存文件失败！\n");
		exit(11);
	}

	/*跳过空白行*/
	while(!feof(proc) && fgetc(proc) != '\n');

	while((num = fscanf( proc, "%s %*s %*s %s %*s %*s", tmp_ip, mac)) == 2){
		if(strcmp(tmp_ip, ip)==0){
			break;
		}
	}
	fclose(proc);
	
	return mac;

}

int create_socket(){
	int socket_handle;
	socket_handle = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == socket_handle){
		printf("创建socket失败！\n");
		exit(101);
	}
	return socket_handle;
}

int create_server(int port){
	int sock;
	struct sockaddr_in server_ip;
	sock = create_socket();
	
	server_ip.sin_family = AF_INET;
	server_ip.sin_port = htons(port);
	/*inet_addr 将字符转化成2进制*/
	//server_ip.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_ip.sin_addr.s_addr = INADDR_ANY;

	if( bind(sock,(struct sockaddr *)& server_ip, sizeof(struct sockaddr)) == -1){
		printf("绑定端口失败！\n" );
		exit(102);
	}

	if( listen(sock,QUEUE_MAX) == -1){
		printf("监听失败！\n");
		exit(103);
	}
	return sock;
}

int create_connect(char *host,int port){
	int sock;
	int error;
	struct sockaddr_in server_ip;
	sock = create_socket();
	server_ip.sin_family = AF_INET;
	server_ip.sin_port = htons(port);
	/*inet_addr 将字符转化成2进制*/
     
	server_ip.sin_addr.s_addr = inet_addr(host);

	error = connect(sock, (struct sockaddr *)&server_ip, sizeof(struct sockaddr));

	if(error == -1){
		return -1;
	}

	return sock;
}

char *get_ip_by_domain(char *domain){
    struct hostent *host_info;
    char *ip;
    char **ip_list;

    ip =(char *)malloc(15*sizeof(char));
    host_info = gethostbyname(domain);

    if (host_info == NULL) {
        return NULL;
    }

    switch (host_info->h_addrtype) {
    case AF_INET:
        ip_list = host_info->h_addr_list;
        inet_ntop( host_info->h_addrtype, *ip_list, ip, 15);
        break;
    default:
        printf("unknow address type!\n");
        break;
    }
    return ip;
}

int is_ipv4(char *ip){
    struct sockaddr_in addr;
    int result;

    result = inet_pton(AF_INET, ip, &addr);

    return result>0? 1:0;
}
