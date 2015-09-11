#!/bin/sh
echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -F
iptables -t nat -A POSTROUTING -o wlan -j MASQUERADE
iptables -A FORWARD -i eth3 -j ACCEPT
