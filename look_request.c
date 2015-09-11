#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "config.h"

#define BUFFER_LEN 20480

#define CONN_PORT 81

struct sockaddr_in server_ip;

int main(int argc, char *argv[])
{
	//get_cmd_opt(argc,argv);
	int sock,client_sock;
	int sock_err;
	int recv_len;
	int sin_size;
	char buffer[BUFFER_LEN];
	char *msg="Welcome to here!";

	sprintf(buffer,"GET %s HTTP/1.1\r\nAccept:*/*\r\nUser-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.153 Safari/537.36\r\n",argv[2]);
	sprintf(buffer,"%sHost: %s\r\n\r\n",buffer,argv[1]);

	

	sock = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sock){
		printf("创建socket失败！\n");
		exit(101);
	}
	
	server_ip.sin_family = AF_INET;
	server_ip.sin_port = htons(CONN_PORT);
	/*inet_addr 将字符转化成2进制*/
	server_ip.sin_addr.s_addr = inet_addr("127.0.0.1");

	sock_err = connect( sock, (struct sockaddr *)&server_ip, sizeof(struct sockaddr ));

	if(sock_err == -1){
		printf("链接失败\n");
		exit(102);
	}



	recv_len = send( sock, buffer, strlen(buffer), 0);
	if(recv_len == -1){
		printf("发送内容失败！\n");
		exit(106);
	}

	memset(buffer,0,BUFFER_LEN);
	recv_len = recv(sock, buffer, BUFFER_LEN, 0);
	printf("len:%d\n",recv_len);
	if(recv_len == -1){
		printf("接收内容失败！\n");
		exit(105);
	}
	if(recv_len){
		buffer[recv_len]='\0';
		printf("接收内容：\n%s\n\n",buffer );
	}	

	memset(buffer,0,BUFFER_LEN);
	recv_len = recv(sock, buffer, BUFFER_LEN, 0);
	if(recv_len){
		buffer[recv_len]='\0';
		printf("接收内容：\n%s\n\n",buffer );
	}	

	close(sock);
	return 0;
}
