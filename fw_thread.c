#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "fw_thread.h"
#include "iptables.h"
#include "config.h"
#include "gw_http.h"
#include "firewall.h"
#include "fw_socket.h"
#include "common.h"

#define BUFFER_LEN 2048

char *ckeck_status_string;
int check_auth_return;

FC fw_config;

void *gw_handler(void *argv){
	struct thread_param client=*((struct thread_param *) argv);
	char buffer[BUFFER_LEN];
	Client client_info;
	char *get_string;
	int recv_len;
	char *http_request;
	char *sendUrl;
	pthread_t help_thread;

	client_info.socket = client.sock;
	fw_config =get_current_config();
	memset(buffer,0,BUFFER_LEN);

	recv_len = recv(client.sock, buffer, BUFFER_LEN, 0);
	if(recv_len == -1 || strlen(buffer) < 1){
		close(client.sock);
		pthread_exit(0);
	}
	debug("[%s:%d]recv content:%s\n",__FILE__,__LINE__,buffer);
	client_info.ip = inet_ntoa(client.client_ip.sin_addr);
	client_info.mac = arp_get_mac(client_info.ip);
        client_info.http_content = strdup(buffer);
	debug("[fw_thread.c]ip:%s\n", client_info.ip);
	debug("[fw_thread.c]mac:%s\n", client_info.mac);
	
	sendUrl = get_auth_server_url();
	if(recv_len){
		buffer[recv_len]='\0';
		get_string = http_get((char *)&buffer);
		//debug("get:%s \n",get_string);
		debug("http_content:%s\n",client_info.http_content);
		if(get_string != NULL){
			if(!strncasecmp(get_string,"/auth",5)){	
                                /*检测用户是否登录*/
                                check_user_login(client_info);	

				client_info.token = http_get_token(get_string);
				pthread_create(&help_thread,NULL,(void *)check_client_token,(void *)&client_info);
				pthread_detach(help_thread);
				pthread_exit(0);			
			}else if(!strncasecmp(get_string,"/status/users",strlen("/status/users"))){
				printf("look over user status\n");
				pthread_create(&help_thread,NULL,(void *)print_client_user_thread,(void *)&client_info);
				pthread_detach(help_thread);
				pthread_exit(0);
			}else if(!strncasecmp(get_string,"/user/del",strlen("/user/del"))){
			    debug_printf("come into user delete...........ok");
			    pthread_create(&help_thread,NULL,(void *)delete_firewall_user,(void *)&client_info);
                            pthread_detach(help_thread);
                            pthread_exit(0);
                       }else if(!strncasecmp(get_string,"/user/speed",strlen("/user/speed"))){
			    debug_printf("come into user speed limit...........ok");
			    pthread_create(&help_thread,NULL,(void *)net_speed_limit,(void *)&client_info);
                            pthread_detach(help_thread);
                            pthread_exit(0);
                       }else if(!strncasecmp(get_string,"/weixin/login",13)){
			   char *query_string;
			   char *new_query_string;
			   int _query_len;
                           char *tmp_name,*tmp_value,*saveptr;

                           query_string = strstr(get_string,"?");
                           query_string=query_string+1;
			   _query_len = strlen(query_string)+128;
			   
			   new_query_string = malloc(_query_len*sizeof(char));
			   new_query_string=strcpy(new_query_string,query_string);
			   
			   tmp_name = strtok_r(query_string,"=",&saveptr);
			   tmp_value = strtok_r(NULL,"=",&saveptr);
			   //printf("%s=>%s\n",tmp_name,tmp_value);
			   if (!strncasecmp(tmp_name,"wifi_passwd",11)) {
			       //sprintf(new_query_string,"%s&uuid=%s",new_query_string,fw_config.UUID);
			       
                               sprintf(sendUrl,"%s/Login/weixinLink/?mac=%s&uuid=%s&ip=%s&port=%d",sendUrl,client_info.mac,fw_config.UUID,client_info.ip,fw_config.port);
                               debug("sendurl:%s\n",sendUrl);
                               strcat(sendUrl,"&");
                               strcat(sendUrl,new_query_string);
                               strcpy(new_query_string,sendUrl);
			       debug("new_query_string:%s\n",new_query_string);
                               http_request  = http_redirect(new_query_string);
                               recv_len = send(client.sock, http_request, strlen(http_request), 0);
                               close(client.sock);
                               free(new_query_string);
                               destroy_client(client_info);
                               pthread_exit(0);
			   }
                        }
		}
	}
        
	sprintf(sendUrl,"%s?mac=%s&uuid=%s&ip=%s&port=%d",sendUrl,client_info.mac,fw_config.UUID,client_info.ip,fw_config.port);

        debug("[%s:%d]sendUrl:%s\n",__FILE__,__LINE__,sendUrl);

        http_request  = http_redirect(sendUrl);

	recv_len = send(client.sock, http_request, strlen(http_request), 0);

	if(recv_len == -1){
		close(client.sock);
		pthread_exit("发送内容失败！");
	}
	close(client.sock);
        debug("[%s:%d]handle thread end!\n",__FILE__,__LINE__);
	destroy_client(client_info);
	pthread_exit(0);
}

/**
 * 获得Get地址
 */
char *http_get(char *content){
	char *split_limit=" \n";
	char *split_buffer;

	split_buffer = strtok(content,split_limit);
	while(split_buffer != NULL){
		if(!strncasecmp(split_buffer,"GET",3)){
			split_buffer = strtok(NULL,split_limit);
			break;
		}else if(!strncasecmp(split_buffer,"POST",4)){
		    split_buffer = strtok(NULL,split_limit);
		    break;
		}
		split_buffer = strtok(NULL,split_limit);
	}
	return split_buffer;
}


void *check_client_token(void *argv){
	Client client = *((Client *) argv);
	FC config;
	int status;
	int check_sock;
	int sock_result;
	char *http_request;
	char *auth_string;
	char buffer[BUFFER_LEN];
	char *content;
	char *host;
	char get_string[512];
	char *auth_server_url;
	printf("check_client_token 111\n");
	config = get_current_config();
	host = get_auth_server_host();
	check_sock = create_connect(config.AuthServerIp,config.ASPort);
	
	sprintf(get_string,"/check/?token=%s&mac=%s", client.token,client.mac);
	content = http_get_content(host,get_string);

	sock_result = send(check_sock,content,strlen(content), 0);
	if(sock_result == -1){
		close(check_sock);
		pthread_exit((void*)101);
	}

	memset(buffer,0,BUFFER_LEN);
	sock_result = recv(check_sock,buffer,BUFFER_LEN,0);
	if(sock_result == -1){
		close(check_sock);
		pthread_exit((void *)102);
	}
        debug("token check thread back:%s\n",buffer);

	auth_string = parse_auth_status(buffer);
	status = auth_status_match(auth_string);
	check_auth_return = status;
	//auth_server_url = malloc(256*sizeof(char));
	auth_server_url = get_auth_server_url();
	if(!status){
		fw_add_allow_mac(client);
		client_append_to_list(client.ip,client.mac, client.token);
		auth_server_url = strcat(auth_server_url,"portal/?msg=ok");
	}else{
		auth_server_url = strcat(auth_server_url,"portal/?msg=fail");
	}
	strcat(auth_server_url,"&uuid=");
	strcat(auth_server_url, config.UUID);
	
	http_request = http_redirect(auth_server_url);
	sock_result = send(client.socket,http_request,strlen(http_request), 0);
	if(sock_result == -1){
		printf("重定向失败！\n");		
	}else{
		printf("重定向成功！\n");
	}	
	
	close(check_sock);
	close(client.socket);
	destroy_client(client);
	safe_free(client.token);
	pthread_exit((void *)0);		
}

char *http_get_token(char *queryString){
	char *limit="=";
	char *tmp;
	char *token;

        //token = malloc(40*sizeof(char));
	tmp = strtok(queryString,limit);
	if(tmp != NULL){
		tmp = strtok(NULL,limit);
		printf("内部：%s\n", tmp);
		token = strdup(tmp);
	}
	return token;
}

void check_user_login(Client ct){
	char *portal_url;
	char *http_request;
        FC config;

	if(check_client_by_mac(ct.mac)){
            //portal_url = malloc(512*sizeof(char));
            config = get_current_config();
            portal_url = get_auth_server_url(); 
           
            portal_url = strcat(portal_url,"portal/?msg=logined");
            strcat(portal_url,"&uuid=");
            strcat(portal_url, config.UUID);
            http_request = http_redirect(portal_url);
            send(ct.socket,http_request,strlen(http_request), 0);
            close(ct.socket);
            //free(portal_url);
            pthread_exit(0);
	}
}

void *print_client_user_thread(void *client_info){
	Client client = *((Client *) client_info);
	char *cotent;
	char *cotent_head="<pre>\r\n";
	char *cotent_footer="</pre>";
	char output_buffer[10240];
	int sock_result;
	printf("inside print user thread!\n");
	/*更新当前用户列表数据*/
	//fw_update_users_data();
	printf("user status had updated!\n");
	memset(output_buffer,0,10240);
	//cotent = malloc(10240*sizeof(char));
	cotent = print_client_list();
	printf("output_buffer:%s\n", output_buffer);
	printf("cotent:%s\n", cotent);
	strcat(output_buffer,cotent_head);
	strcat(output_buffer,cotent);
	strcat(output_buffer,cotent_footer);
	//free(cotent);
	printf("\r\n\r\n\r\n");
	sock_result = send(client.socket, output_buffer, strlen(output_buffer), 0);
	close(client.socket);
	destroy_client(client);
	pthread_exit(0);
}

/**
 * 更新用户数据线程
 * @return NULL
 */
void *update_client_data_thread(){
        int sock,err_result;
        char *user_info, *server_host;
        char *content;
        char *post_path="/user/all_update";
        FC config;

        server_host = get_auth_server_host();
        debug("Thread info:server_host %s\r\n", server_host);
        config = get_current_config();
        debug_printf("Thread info:start update client data thread");
	while(1){
            fw_update_users_data();
            if (get_total_client()) {
                debug_printf("Thread info:firewall user info update....ok"); 
                sock = create_connect(config.AuthServerIp, config.ASPort);
                debug_printf("Thread info: auth server socket conntection is builded...ok");
                
		lock_client_list();
                user_info = client_list_for_post();
		unlock_client_list();
                content = http_post(server_host, post_path, user_info);
                debug("\n\n\nclient post:\n%s\n\n\n", content);
                send(sock, content, strlen(content), 0);
                close(sock);
                debug_printf("Thread info:send update client data....ok");
                safe_free(user_info);
                safe_free(content);
            }
            sleep(60);
	}
	pthread_exit(0);
}


void *delete_firewall_user(void *argv){
    Client *client = (Client *) argv;
    char *http_body, *mac;
    map_t *post_container;
    ClientLink *client_info;

    debug("[%s:%d]delete_firewall_user()\n",__FILE__,__LINE__);
    http_body = parse_http_content(client->http_content);
    debug("[%s:%d]after parse_http_content()\n",__FILE__,__LINE__);
    post_container = create_http_container();
    debug("[%s:%d]http body:%s\n",__FILE__,__LINE__,http_body);
    post_container = pare_form_field(http_body);

    mac = (char *)map_at(post_container,"mac");
    debug("mac:%s\n",mac);
    client_info = get_client_by_mac(mac);

    if (NULL != client_info) {
	fw_del_allow_mac(client_info->ip, client_info->mac); 
	debug("delete user mac:%s\n",client_info->mac);
	del_client_by_mac(client_info->mac);
    }else{
	debug("delete user mac(%s) not exists\n",mac);
    }
    send(client->socket,"ok",2,0);
    close(client->socket);

    map_destroy(post_container);
    destroy_client(*client);
    pthread_exit(0);
}

void net_speed_limit(void *argv){
    Client *client = (Client *) argv;
    char *http_body, *ip;
    float limit_speed;
    map_t *post_container;
    ClientLink *client_info;

    debug("[%s:%d]delete_firewall_user()\n",__FILE__,__LINE__);
    http_body = parse_http_content(client->http_content);
    debug("[%s:%d]after parse_http_content()\n",__FILE__,__LINE__);
    post_container = create_http_container();
    debug("[%s:%d]http body:%s\n",__FILE__,__LINE__,http_body);
    post_container = pare_form_field(http_body);

    ip = (char *)map_at(post_container,"IP");
    limit_speed = atof(map_at(post_container,"speed"));
    debug("ip:%s\n",ip);
    client_info = get_client_by_ip(ip);

    if (NULL != client_info && limit_speed) {
	debug("current user speed:%f\n",client_info->speed);
        if (client_info->speed) {
            fw_del_limit_speed(ip,client_info->speed);
        }
        if (limit_speed) {
            fw_add_limit_speed(ip, limit_speed); 
        }
        
        client_info->speed = limit_speed;
    } else {
	debug("limit speed user ip(%s) not exists\n",ip);
    }
    send(client->socket,"ok",2,0);
    close(client->socket);

    map_destroy(post_container);
    destroy_client(*client);
    pthread_exit(0);
}

void destroy_client(Client ct){
    safe_free(ct.http_content);
    safe_free(ct.mac);
    //safe_free(ct.token);
}
