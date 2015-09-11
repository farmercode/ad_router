
#ifndef _CSTL_CMAP_H
#define _CSTL_CMAP_H
#include <cstl/cmap.h>

map_t *create_http_container();

void insert_container(map_t *contaniner,char *fieldname,char *fieldvalue);

char *http_server();

char *http_redirect(char *url);

char *get_auth_server_url();

char *http_get_content(char *host,char *get);

char *http_post(char *host,char *path,char *content);

char *get_auth_server_host();

char *parse_auth_status(char *content);

int auth_status_match(char *line);

char *parse_http_content(char *http);

map_t *pare_form_field(char *form);

#endif /* _CSTL_CMAP_H */

