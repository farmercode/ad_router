#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "client_list.h"
#include "common.h"
#include "fw_socket.h"
#include "config.h"
#include "gw_http.h"

ClientLink *client_first = NULL;

char list_output[10240];

int totoal_user=0;

pthread_mutex_t user_mutex;

void init_client_mutex(){
    int result;
    result = pthread_mutex_init(&user_mutex,NULL);
    if (result!=0) {
        debug_printf("[init_user_mutex]:user mutex init failed\n");
        exit(211);
    }
}

void lock_client_list(){
    pthread_mutex_lock(&user_mutex);
}

void unlock_client_list(){
    pthread_mutex_unlock(&user_mutex);
}

int get_total_client(){
	return totoal_user;
}

void client_append_to_list(char *ip,char *mac,char *token){
	ClientLink *current,*previous;
	NetData counter;
	current = previous = NULL;
	current = client_first;
	int now;

        lock_client_list();
	while(current != NULL){
		previous = current;
		current = current->next;
	}

	int size = sizeof(ClientLink);

	current = (ClientLink *)malloc(size);
	memset(current, 0, size);
	now = get_current_time();
	counter.download = 0;
	counter.upload = 0;
	current->ip = strdup(ip);
	current->mac = strdup(mac);
	current->token = strdup(token);
        current->speed = 0;
	current->login_time = now;
	current->last_update = now;
	current->net_data = counter;
	
	if(previous == NULL){
		client_first= current;
	}else{
		previous->next = current;
	}

	totoal_user++;
        unlock_client_list();
}

ClientLink *get_client_by_mac(char *mac){
	ClientLink *current;
	int len=strlen(mac);

	//current = (ClientLink *)malloc(sizeof(ClientLink));
	current = client_first;
        debug("\n[get_client_by_mac]:%s\n",mac);
	while(current != NULL && len>0){
		printf("mac client loop mac:%s\n", current->mac);
		if(!strncasecmp(mac,current->mac,len)){
			return current;
		}
		current = current->next;
	}
	return NULL;
}

ClientLink *get_client_by_ip(char *ip){
	ClientLink *current;
	int len=strlen(ip);
	//current = (ClientLink *)malloc(sizeof(ClientLink));
	current = client_first;

	while(current != NULL){
		printf("client loop ip:%s\n", current->ip);
		if(!strncasecmp(ip,current->ip,len)){
			printf("ip:%s\n",ip );
			return current;
		}
		current = current->next;
	}
	return NULL;
}

int check_client_by_mac(char *mac){
	ClientLink *current;
	int len=strlen(mac);
	current = client_first;
        debug("[check_client_by_mac:%s\n]",mac);
	while(current != NULL){
		if(!strncasecmp(mac,current->mac,len)){
			debug("mac:%s\n",mac );
			return 1;
		}
		current = current->next;
	}
	return 0;
}

void del_client_by_mac(char *mac){
	ClientLink *current,*previous;
	int len=strlen(mac);
        previous = NULL;
	current = client_first;

	while(strncasecmp(mac,current->mac,len) && current->next != NULL){
		previous = current;
		current = current->next;
	}

	if(strncasecmp(mac,current->mac,len)){
		printf("didn't find mac:%s\n",mac);
		return ;
	}	

	if(previous == NULL){
		client_first =  current->next;
	}else{
		previous->next = current->next;
	}

	if(current->ip != NULL){
		safe_free(current->ip);
	}
	
	if(current->mac != NULL){
		safe_free(current->mac);
	}

	if(current->token != NULL){
		safe_free(current->token);
	}
	
	safe_free(current);
	totoal_user--;
}

char *print_client_list(){
	ClientLink *current;
	char buffer[1024];
	memset(list_output,0,10240);
	current = (ClientLink *)malloc(sizeof(ClientLink));
	current = client_first;
	while(current != NULL){
		memset(buffer,0,1024);
		printf("ip:%s\r\n",current->ip);
		sprintf(buffer,"Ip:%s \tMac:%s\tLogin_time:%d <br>\r\nToken:%s<br>\r\nDownload:%llu\tUpload:%llu\tLastUpdate:%d<br>\r\n",current->ip,current->mac,current->login_time, current->token,current->net_data.download,current->net_data.upload,(int)current->last_update);
		current = current->next;
		strcat(list_output,buffer);
	}
	printf("client_list:%s",list_output);
	free(current);
	return list_output;
}

char *client_list_for_post(){
    ClientLink *current;
    char buffer[256];
    char *post_content=NULL;
    int len;
    int index=0;
    debug("[%s:%d]client_list_for_post()\n",__FILE__,__LINE__);

    if (!totoal_user) {
        return NULL;
    }

    len = totoal_user * 256; 
    post_content = (char *)malloc(len*sizeof(char));
    memset(post_content,0,len);
    debug("[%s:%d]post_content:\n%s\n",__FILE__,__LINE__,post_content);
    current = client_first;
    while(current != NULL){
            memset(buffer,0,256);
            sprintf(buffer,"user[%d][IP]=%s&user[%d][mac]=%s&user[%d][token]=%s&user[%d][download]=%llu&"
                    "user[%d][upload]=%llu", index, current->ip, index,current->mac, index, current->token,
                    index,current->net_data.download, index, current->net_data.upload);
            current = current->next;
            //debug("user %d buffer:%s\n",index,buffer);
            if (index) {
                strcat(post_content, "&");
            }
             
            strcat(post_content,buffer);
            index++;
    }
    debug("[%s:%d]post_content:\n%s\n",__FILE__,__LINE__,post_content);
    return post_content;
}

void net_delete_user(ClientLink *client){
    char *send_content;
    char field[128];
    char *server_host;
    char *path = "/user/del";
    int sock,err_result;
    FC config;

    config = get_current_config();
    sock = create_connect(config.AuthServerIp, config.ASPort);
    server_host = get_auth_server_host();
    send_content = (char *)malloc(512*sizeof(char));
    sprintf(field,"mac=%s",client->mac);
    send_content = http_post(server_host, path, field);
    err_result = send(sock, send_content,strlen(send_content),0);

    if (err_result == -1) {
        debug("debug info:send to server deleted user info failed,user mac '%s'\r\n",client->mac);
    }
    close(sock);
    free(send_content);
}
