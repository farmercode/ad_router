#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gw_http.h"
#include "config.h"
#include "lib_string.h"

char auth_server_url[128];

map_t *create_http_container(){
    map_t *http_container;

    http_container=create_map(char *,char *);
    map_init(http_container);

    return http_container;
}


void insert_container(map_t *contaniner,char *fieldname,char *fieldvalue){
    pair_t *ppair= create_pair(char *,char *);
    pair_init(ppair);
    pair_make(ppair,fieldname,fieldvalue);
    map_insert(contaniner,ppair);
    pair_destroy(ppair);
}


char *http_server(){
	static char *server="Server:King Http Server 1.0\n";
	return server;
}

char *http_redirect(char *url){
	static char http_request[1024];
	char location[512];

        memset(http_request,0,1024);
        memset(location,0,512);

	strcpy(http_request,"HTTP/1.1 302 Found\r\n");
	strcat(http_request,http_server());
	strcpy(location, "location:");
	strcat(location, url);
	strcat(location,"\r\n\r\n");

	strcat(http_request,location);

	return http_request;
}

char *get_auth_server_url(){
	FC fw_config;
        
        fw_config = get_current_config(); 
        memset(auth_server_url,0,128);
	if(fw_config.ASPort != 80){
		sprintf(auth_server_url,"http://%s:%d%s",fw_config.AuthServer,fw_config.ASPort,fw_config.AuthPath);
	}else{
		sprintf(auth_server_url,"http://%s%s",fw_config.AuthServer,fw_config.AuthPath);
	}	

	return auth_server_url;
}

char *http_get_content(char *host,char *get){
	static char content[512];
	
	sprintf(content,"GET %s HTTP/1.1\r\n",get);
	
	strcat(content,"Host: ");
	strcat(content,host);
	strcat(content,"\r\n");
	strcat(content,"Connection: keep-alive\r\nAccept: */*\r\nAccept-Language: zh-CN,zh\r\n\r\n");
	return content;
}

char *http_post(char *host,char *path,char *content){
    char *post_content;
    char *encoded_content;
    int len;

    len=strlen(content);
    post_content = (char *)malloc((len+256)*sizeof(char));
    //encoded_content = urlencode(content);
    sprintf(post_content,"POST %s HTTP/1.1\r\n"
            "Accept: */*\r\n"
            "Host: %s\r\n"
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s\r\n", path, host, len, content);
    //free(encoded_content);
    return post_content;
}

char *get_auth_server_host(){
	static char host[128];
	FC fw_config;
	fw_config = get_current_config();
        memset(host,0,128);
	if(fw_config.ASPort != 80){
		sprintf(host,"%s:%d",fw_config.AuthServer,fw_config.ASPort);
	}else{
		strcpy(host, fw_config.AuthServer);
	}	
	return host;
}

char *parse_auth_status(char *content){
	char *line_limit="\n";
	char *line;	
	int pos;
	int status;

	line = strtok(content,line_limit);	
	line = strtok(NULL,line_limit);

	while(line != NULL){
		ltrim(line);
		rtrim(line);
		if(!strncasecmp(line,"AuthStatus",10)){
			break;			
		}
		line = strtok(NULL,line_limit);
	}

	return line;
}

int auth_status_match(char *line){
	char *cell_limit=":";
	char *cell;
	int status;
	cell = strtok(line,cell_limit);
	cell = strtok(NULL,cell_limit);
	ltrim(cell);
	rtrim(cell);
	status = atoi(cell);

	if(status == 1){
		return 0;
	}else{
		return 1;
	}
}


char *parse_http_content(char *http){
	char *content;
	char *buffer;
	//char *saveptr;
        int len;
        debug_printf("come into parse_http_content function....[ok]");
        debug("http content =================\n:%s\n",http);
        len=strlen(http);
	content = strstr(http,"\r\n\r\n");
	ltrim(content);
        debug("content:\n%s\n",content);
        return content;
}

map_t *pare_form_field(char *form){
    map_t *post_container;
    char *saveptr1,*saveptr2, *buffer, *fieldname, *fieldvalue;

    post_container = create_http_container();

    buffer = strtok_r(form,"&",&saveptr1);
    while(buffer != NULL){
        fieldname = strtok_r(buffer,"=",&saveptr2);
        fieldvalue = strtok_r(NULL,"=",&saveptr2);
        if (fieldvalue==NULL) {
            buffer = strtok_r(NULL,"&",&saveptr1);
            continue;
        }
        urldecode(fieldname);
        urldecode(fieldvalue);
        debug("fieldname:%s=>fieldvalue:%s\n", fieldname, fieldvalue); 
        insert_container(post_container, fieldname, fieldvalue);
        buffer = strtok_r(NULL,"&",&saveptr1);
    }
    debug_printf("parse form end......");
    return post_container;
}
