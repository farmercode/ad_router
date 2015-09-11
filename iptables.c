#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "iptables.h"
#include "config.h"
#include "client_list.h"


int do_iptables_cmd(char *cmd){
	int result = 0;
	char ip_cmd[1024];
	sprintf(ip_cmd,"iptables %s",cmd);
        debug("iptables command:%s\n",ip_cmd);
	result = system(ip_cmd);
	return result;
}

/**
 * 防火墙初始化
 */
void fw_iptables_init(){
	atexit(fw_iptables_destroy);

	do_iptables_cmd("-t mangle -N " FW_USER_OUT);
	do_iptables_cmd("-t mangle -N " FW_USER_IN);
	do_iptables_cmd("-t mangle -N FW_LIMIT_SPEED");

	do_iptables_cmd("-t nat -N " FW_OUTPUT );
	do_iptables_cmd("-t nat -N " FW_WHITELIST);
	do_iptables_cmd("-t nat -N " FW_GLOBAL);
	do_iptables_cmd("-t nat -N " FW_ALLOW_MAC);

	do_iptables_cmd("-t mangle -I PREROUTING -j " FW_USER_OUT);
	do_iptables_cmd("-t mangle -I POSTROUTING -j " FW_USER_IN);
	do_iptables_cmd("-t mangle -I FORWARD -j FW_LIMIT_SPEED");
	do_iptables_cmd("-t nat -A PREROUTING -j " FW_OUTPUT);
	do_iptables_cmd("-t nat -A " FW_OUTPUT " -j " FW_WHITELIST);
	do_iptables_cmd("-t nat -A " FW_OUTPUT " -j " FW_GLOBAL);
	do_iptables_cmd("-t nat -A " FW_OUTPUT " -j " FW_ALLOW_MAC);

	fw_read_whitelist();
        debug_printf("firewall inited whitelist");
	fw_iptables_read_firewallrule();
        debug_printf("firewall inited firewallrule");
	fw_iptables_authserver();
	
	/*允许局域网内部通信*/
	do_iptables_cmd("-t nat -A " FW_OUTPUT " -d 192.168.0.0/16 -j ACCEPT");
	//do_iptables_cmd("-I INPUT -p tcp --dport 9020 -j ACCEPT");

	/*开启限速设置*/
	fw_init_speed_limit();

	do_iptables_cmd("-t nat -A" FW_OUTPUT " -p tcp -s 192.168.0.0/16 -j REDIRECT --to-port 9020");
        debug_printf("firewall inited finished");
}

/**
 * 授权服务器初始化
 */
void fw_iptables_authserver(){
	char cmd[128];
	FC fw_config;
	fw_config = get_current_config();
	sprintf(cmd,"-t nat -A %s -d %s -j ACCEPT", FW_OUTPUT,fw_config.AuthServer);
	do_iptables_cmd(cmd);
}

/**
 * 读取防火墙设置
 */
void fw_iptables_read_firewallrule(){
	FC fw_config;
	int frsize;
	int index=0;
	char rule[128];
	FwRule fr;
	fw_config = get_current_config();
	frsize = get_firewallrule_size();
	for(;index<frsize;index++){
		fr = fw_config.FirewallRule[index];

		if(!strncasecmp(fr.type,"all",3)){
			sprintf(rule,"-t nat -A " FW_GLOBAL " -d %s -j ACCEPT", fr.host);
		}else{
			if(fr.port == 0){
				sprintf(rule,"-t nat -A " FW_GLOBAL " -p %s -d %s -j ACCEPT", fr.type,fr.host);
			}else{
				sprintf(rule,"-t nat -A " FW_GLOBAL " -p %s -d %s --dport %d -j ACCEPT", fr.type,fr.host, fr.port);
			}
		}
		do_iptables_cmd(rule);
	}

}

void fw_read_whitelist(){
	FC fw_config;
	int wlsize;
	char ip[128];
	char cmd[128];
	int index=0;
	fw_config = get_current_config();
	wlsize = get_whitelist_size();
	for(;index<wlsize;index++){
		strcpy(ip,fw_config.WhiteList[index]);
		sprintf(cmd,"-t nat -A " FW_WHITELIST " -s %s -j ACCEPT", ip);
		//printf("%s\n",cmd);
		do_iptables_cmd(cmd);
	}
}

/** 
* 向防火墙添加用户 
*/ 
void fw_add_allow_mac(Client client){
	char cmd[512];
	sprintf(cmd, "-t nat -A " FW_ALLOW_MAC " -m mac --mac-source %s -j ACCEPT", client.mac);
	do_iptables_cmd(cmd);

	sprintf(cmd, "-t mangle -A " FW_USER_OUT "  -p all -s %s -m mac --mac-source %s -j MARK --set-mark 110", client.ip,client.mac);
	do_iptables_cmd(cmd);

	sprintf(cmd, "-t mangle -A " FW_USER_IN "  -d %s -j ACCEPT", client.ip);
	do_iptables_cmd(cmd);
}

/** 
 * 删除防火墙用户 
 */ 
void fw_del_allow_mac(char *ip,char *mac){
	char cmd[512];
	sprintf(cmd, "-t nat -D " FW_ALLOW_MAC " -m mac --mac-source %s -j ACCEPT", mac);
	do_iptables_cmd(cmd);

	sprintf(cmd, "-t mangle -D " FW_USER_OUT "  -p all -s %s -m mac --mac-source %s -j MARK --set-mark 110", ip,mac);
	do_iptables_cmd(cmd);

	sprintf(cmd, "-t mangle -D " FW_USER_IN "  -d %s -j ACCEPT", ip);
	do_iptables_cmd(cmd);
}

void fw_init_speed_limit(){
	FC fw_config;
	SpeedRule sr;
	int index=0;
	int total;
	fw_config = get_current_config();
	total = get_speedlimit_size();
	for(;index<total;index++){
		sr = fw_config.SpeedLimit[index];
		fw_add_limit_speed(sr.ip,sr.speed);
	}
}

void fw_add_limit_speed(char *ip,float speed){
	char cmd[512];
	int real_speed;
	float rate=1.5;
	real_speed = ceil(speed/rate);

	sprintf(cmd, "-t mangle -I FW_LIMIT_SPEED -d %s -j DROP", ip);
	do_iptables_cmd(cmd);

	sprintf(cmd, "-t mangle -I FW_LIMIT_SPEED -d %s -m limit --limit %d/sec --limit-burst %d -j ACCEPT", ip, real_speed,real_speed);
	do_iptables_cmd(cmd);
}

void fw_del_limit_speed(char *ip,float speed){
    char cmd[512];
    int real_speed;
    float rate=1.5;
    real_speed = ceil(speed/rate);

    sprintf(cmd, "-t mangle -D FW_LIMIT_SPEED -d %s -j DROP", ip);
    do_iptables_cmd(cmd);

    sprintf(cmd, "-t mangle -D FW_LIMIT_SPEED -d %s -m limit --limit %d/sec --limit-burst %d -j ACCEPT", ip, real_speed,real_speed);
    do_iptables_cmd(cmd);
}

/**
 * 更新用户数据
 */
void fw_update_users_data(){
	if(get_total_client()){
		fw_update_users_out_data();
		fw_update_users_in_data();
	}	
}

/**
 * 更新用户上传信息
 */
void fw_update_users_out_data(){
	debug_printf("[firewall user out function]\n");
	int fd;
	FILE *pipe;
	char buffer[256];
	char *cmd;
	char *ip,*mac;
	ClientLink *client_info;
	unsigned long long counters;
	int parse_result,now;
	cmd = "iptables -t mangle -x -v -L " FW_USER_OUT " -n";
	pipe = popen(cmd,"r");
	now = get_current_time();

	/* skip the first two lines */
	while (('\n' != fgetc(pipe)) && !feof(pipe));
	while (('\n' != fgetc(pipe)) && !feof(pipe));
	ip = malloc(15*sizeof(char));
	mac = malloc(17*sizeof(char));
	
        lock_client_list();

	while(!feof(pipe)){
		parse_result = fscanf(pipe,"%*d %llu %*s %*s %*s %*s %*s %s %*s %*s %s %*s %*s %*s",&counters,  ip, mac);
		if (3 == parse_result && EOF != parse_result){
			client_info = get_client_by_mac(mac);
                        debug("==>after get_client_by_mac:%s\n",client_info->mac);
			if(client_info != NULL && counters != client_info->net_data.upload){
				client_info->net_data.upload = counters;
				client_info->last_update = get_current_time();
			}else if(now-client_info->last_update>900){
				fw_del_allow_mac(client_info->ip, client_info->mac);
                                debug_printf("=>firewall delete by mac end\n");
                                net_delete_user(client_info);
                                debug_printf("=>net delete by mac end\n");
                                del_client_by_mac(client_info->mac);
			}
		}
	}
        unlock_client_list();
	pclose(pipe);
        debug_printf("--fw_update_users_out_data before free ip");
	free(ip);
        debug_printf("--fw_update_users_out_data before free mac");
	free(mac);
}

/**
 * 更新用户下载信息
 */
void fw_update_users_in_data(){
	debug_printf("in\n");
	int fd,parse_result;
	FILE *pipe;
	char buffer[256];
	char *cmd;
	ClientLink *client_info;
	int now;
	char *ip;
	unsigned long long counters;

	cmd = "iptables -t mangle -x -v -L " FW_USER_IN " -n";
	pipe = popen(cmd,"r");
	now = get_current_time();

	ip = (char *)malloc(15*sizeof(char));
	/* skip the first two lines */
	while (('\n' != fgetc(pipe)) && !feof(pipe));
	while (('\n' != fgetc(pipe)) && !feof(pipe));

        lock_client_list();

	while(!feof(pipe)){
		parse_result = fscanf(pipe, "%*s %llu %*s %*s %*s %*s %*s %*s %15[0-9.]",&counters,ip);
		if (2 == parse_result && EOF != parse_result){
			client_info = get_client_by_ip(ip);
			if(client_info != NULL && counters != client_info->net_data.download){
				client_info->net_data.download = counters;
				client_info->last_update = now;
			}else if(client_info != NULL && (now-client_info->last_update>900)){
				fw_del_allow_mac(client_info->ip, client_info->mac);
                                net_delete_user(client_info);
                                del_client_by_mac(client_info->mac);
			}
		}
	}

        unlock_client_list();

	pclose(pipe);
	free(ip);
}

/**
 * 关闭防火墙
 */
void fw_iptables_destroy(){

	do_iptables_cmd("-t nat -D PREROUTING -j " FW_OUTPUT);
	do_iptables_cmd("-t nat -F " FW_OUTPUT);
	do_iptables_cmd("-t nat -F " FW_WHITELIST);
	do_iptables_cmd("-t nat -F " FW_ALLOW_MAC);
	do_iptables_cmd("-t nat -F " FW_GLOBAL);
	do_iptables_cmd("-t mangle -F " FW_USER_OUT);
	do_iptables_cmd("-t mangle -F " FW_USER_IN);	
	do_iptables_cmd("-t mangle -D PREROUTING -j " FW_USER_OUT);
	do_iptables_cmd("-t mangle -D POSTROUTING -j " FW_USER_IN);
	do_iptables_cmd("-t mangle -D FORWARD -j FW_LIMIT_SPEED");
	do_iptables_cmd("-t mangle -F FW_LIMIT_SPEED");

	do_iptables_cmd("-t mangle -X FW_LIMIT_SPEED");
	do_iptables_cmd("-t mangle -X " FW_USER_OUT);
	do_iptables_cmd("-t mangle -X " FW_USER_IN);

	do_iptables_cmd("-t nat -X " FW_GLOBAL);
	do_iptables_cmd("-t nat -X " FW_ALLOW_MAC);
	do_iptables_cmd("-t nat -X " FW_OUTPUT);
}
