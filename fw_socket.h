
char *arp_get_mac(char * ip);
int create_socket();
int create_server(int port);
int create_connect(char *host,int port);
char *get_ip_by_domain(char *domain);
int is_ipv4(char *ip);

