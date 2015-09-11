#/bin/sh

iptables -t mangle -N FW_USER_OUT
iptables -t mangle -N FW_USER_IN
iptables -t mangle -A INPUT -j FW_USER_OUT
iptables -t mangle -A FW_USER_OUT -s 192.168.10.12 -m mac --mac-source F3:6D:0A:0B:27:31 -j MARK --set-mark 110
iptables -t mangle -A FW_USER_IN -d 192.168.10.12 -j ACCEPT
#iptables -t mangle -A FW_USER_OUT -s 192.168.10.11 -m mac --mac-source F3:6D:0A:0B:27:31 -j MARK
