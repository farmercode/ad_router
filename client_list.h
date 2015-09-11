
#ifndef _CLIENTLIST_H_
#define _CLIENTLIST_H_

typedef struct _client{
	char *ip;
	char *mac;
	char *token;
	int socket;
        char *http_content;
} Client;

typedef struct _net_data {
	unsigned long long download;
	unsigned long long upload;
} NetData;

typedef struct _client_link {
	char *ip;
	char *mac;
	char *token;
	int login_time;
        float speed;
	NetData net_data;
	time_t last_update;
	struct _client_link *next;
} ClientLink;

void init_client_mutex();

void lock_client_list();

void unlock_client_list();

int get_total_client();

void client_append_to_list(char *ip,char *mac,char *token);

ClientLink *get_client_by_mac(char *mac);

ClientLink *get_client_by_ip(char *ip);

int check_client_by_mac(char *mac);

void del_client_by_mac(char *mac);

char *print_client_list();

void net_delete_user(ClientLink *client);

char *client_list_for_post();

#endif /* _CLIENTLIST_H_ */