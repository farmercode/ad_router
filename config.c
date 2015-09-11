/*
 * config.c
 *
 *  Created on: 2014-5-24
 *      Author: king
 */
#include <stdio.h>
 #include <stdlib.h>
#include <unistd.h>
#include <string.h>
  
#include "config.h"
#include "lib_string.h"
#include "common.h"
#include "fw_socket.h"

#define BUFFER_SIZE 512

#define CONFIG_PATH "/etc/firewall.conf"

char *config_file;
char *config_tpl="c:dg";

FC config;

int is_debug=0;
int FRsize = 0;
int WLsize = 0;
int SLsize=0;

FC get_current_config(){
	return config;
}

/**
 * 获得命令行传参
 */
 void get_cmd_opt(int argc,char **argv){
 	int key;
 	while((key = getopt(argc,argv,config_tpl)) != -1){
 		switch(key){
 			case 'c':
 				config_file = strdup(optarg);
 				break;
 			case 'd':
 				config.mode = 1;
 				break;
 			case 'g':
 				is_debug = 1;
 				break;
 			default:
 				printf("改参数为设置捕获:%d => %s\n",key,optarg );
 		}
 	}
 }

/*初始化配置文件*/
void init_config(){
	if(config_file == NULL){
		config_file = strdup(CONFIG_PATH);
	}
	config.debug_mode = is_debug;
	debug("config file:%s\n",config_file);

	//config.debug_mode = is_debug;
	read_config();

        if (is_ipv4(config.AuthServer)) {
            config.AuthServerIp = strdup(config.AuthServer);
        }else{
            config.AuthServerIp = get_ip_by_domain(config.AuthServer);
        }
        debug("AuthServerIp:%s\n", config.AuthServerIp);
 }

int get_firewallrule_size(){
	return FRsize;
}

int get_whitelist_size(){
	return WLsize;
}

int get_speedlimit_size(){
	return SLsize;
}

void read_config(){
	FILE *conf;
	char line[BUFFER_SIZE];

	conf = fopen(config_file, "r");
	if(!conf){
		debug("config file %s open failed!\n",config_file);
		exit(2);
	}
	int num = 1;

	debug("start loading config file:%s\n",config_file);

	while(!feof(conf) && fgets(line,BUFFER_SIZE,conf)){
		remove_config_note(line);
		ltrim(line);
		if(strlen(line) == 0){
			continue;
		}		

		debug("config line:%s \n",line);
		if(strncasecmp(line,"FirewallSet",strlen("FirewallSet")) == 0){
			parse_firewallrule(conf);
		}else if(!strncasecmp(line,"WhileList",9)){
			parse_whitelist(conf);
		}else if(!strncasecmp(line,"SpeedLimit",10)){
			parse_speed_limit(conf);
		}else{
			string_spilt(line);
		}
		
	}
	debug("end read config \n",NULL);
	fclose(conf);
}

void string_spilt(char *str){
	char *tmp;
	char *limit=" :\t";
	rtrim(str);
	tmp = strtok(str,limit);
	while(tmp != NULL){
		ltrim(tmp);				
		if(strncasecmp(tmp,"AuthServer",strlen("AuthServer")) == 0){
			tmp = strtok(NULL, limit);
			ltrim(tmp);
			rtrim(tmp);
			config.AuthServer = strdup(tmp);
            //config.AuthServer = "192.168.30.1";
		}else if(strncasecmp(tmp,"port",strlen("port")) == 0){
			config.port = atoi(strtok(NULL, limit));
		}else if(strncasecmp(tmp,"ASPort",6) ==0){
			config.ASPort = atoi(strtok(NULL, limit));
		}else if(!strncasecmp(tmp,"AuthPath",8)){
			tmp = strtok(NULL, limit);
			config.AuthPath = strdup(tmp);
		}else if(!strncasecmp(tmp,"UUID",4)){
			tmp = strtok(NULL, limit);
			debug("UUID:%s\n",tmp);
			config.UUID = strdup(tmp);

		}
		else{
			//printf("未捕获参数：%s\n", tmp);
		}
		tmp = strtok(NULL, limit);
	}
}

void parse_firewallrule(FILE *file){
	char line[BUFFER_SIZE];
	char *limit=" ";
	char *cell;
	FwRule fr;
	while(!feof(file) && fgets(line,BUFFER_SIZE,file) ){
		remove_config_note(line);
		ltrim(line);
		rtrim(line);
		
		if(strlen(line) == 0){
			continue;
		}	

		debug("parse_firewallrule: %s|\n",line);		
		if(strncasecmp(line,"}",1) == 0){
			break;
		}
		cell = strtok(line,limit);

		fr.host = strdup(cell);

		cell = strtok(NULL,limit);
		if(cell == NULL){
			fr.type = "all";
			fr.port = 0;
		}else{
			fr.type = strdup(cell);

			cell = strtok(NULL,limit);
			if(is_number(cell)){
				fr.port = atoi(cell);
			}else{
				fr.port = 0;
			}
		}

		config.FirewallRule[FRsize] = fr;
		FRsize++;
	}
	return ;
}

void parse_whitelist(FILE *file){
	char line[BUFFER_SIZE];
	while(!feof(file) && fgets(line,BUFFER_SIZE,file) ){
		remove_config_note(line);
		ltrim(line);	
		rtrim(line);
		if(strlen(line) == 0){
			continue;
		}		

		if(strncasecmp(line,"}",1) == 0){
			break;
		}
		strcpy(config.WhiteList[WLsize++],line);
	}
	return ;
}

void parse_speed_limit(FILE *file){
	char line[BUFFER_SIZE];
	char *limit=" \t";
	char *cell;
	SpeedRule sr;
	while(!feof(file) && fgets(line,BUFFER_SIZE,file) ){
		remove_config_note(line);
		ltrim(line);	
		if(strlen(line) == 0){
			continue;
		}		
		if(strncasecmp(line,"}",1) == 0){
			break;
		}
		cell = strtok(line,limit);

		sr.ip = strdup(cell);

		cell = strtok(NULL,limit);
		if(cell == NULL){
			debug("debug :parse_speed_limit find %s's speed is NULL",sr.ip);
		}
		sr.speed = atof(cell);

		config.SpeedLimit[SLsize] = sr;
		SLsize++;
	}
	return ;
}

