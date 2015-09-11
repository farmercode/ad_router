#ifndef _FW_THREAD_H
#define _FW_THREAD_H
#include <cstl/cmap.h>
#include "client_list.h"

void *gw_handler(void *argv);

char *http_get(char *content);

void *check_client_token(void *argv);

char *http_get_token(char *queryString);

void check_user_login(Client ct);

void *print_client_user_thread(void *client_info);

void *update_client_data_thread();

void *delete_firewall_user(void *argv);

void net_speed_limit(void *argv);

void destroy_client(Client ct);

#endif /*end _FW_THREAD_H*/
