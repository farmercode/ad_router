
#ifndef _IPTABLES_H
#define _IPTABLES_H

#define FW_OUTPUT "FW_OUTPUT"
#define FW_WHITELIST "FW_WHITELIST"
#define FW_GLOBAL "FW_GLOBAL"
#define FW_ALLOW_MAC "FW_ALLOW_MAC"
#define FW_USER_OUT "FW_USER_OUT"
#define FW_USER_IN "FW_USER_IN"

#include "client_list.h"

int do_iptables_cmd(char *cmd);

void fw_iptables_init(void);

void fw_iptables_authserver();

void fw_iptables_read_firewallrule();

void fw_read_whitelist();

void fw_add_allow_mac(Client client);

void fw_init_speed_limit();

void fw_add_limit_speed(char *ip,float speed);

void fw_del_limit_speed(char *ip,float speed);

void fw_update_users_data();

void fw_update_users_out_data();

void fw_update_users_in_data();

void fw_iptables_destroy();

#endif /* _IPTABLES_H */
