
typedef struct _speedrule {
	char *ip;
	float speed;
} SpeedRule;

typedef struct FirewallRule {
	char *host;
	char *type;
	int port;
} FwRule;


typedef struct FireConfig
{
	char *AuthServer;
        char *AuthServerIp;
	int debug_mode;
	int port;
	char *UUID;
	int ASPort;
	char *AuthPath;
	int mode;
	char WhiteList[512][128];
	FwRule FirewallRule[512];
	SpeedRule SpeedLimit[512];
} FC;

FC get_current_config();
void get_cmd_opt(int argc,char **argv);
void init_config();
int get_firewallrule_size();
int get_whitelist_size();
int get_speedlimit_size();
void read_config();
void string_spilt(char *str);
void parse_firewallrule(FILE *file);
void parse_whitelist(FILE *file);
void parse_speed_limit(FILE *file);
