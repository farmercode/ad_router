firewall : CFLAGS = -g -pthread 
firewall : firewall.o fw_socket.o config.o iptables.o fw_thread.o gw_http.o lib_string.o client_list.o common.o
	cc -pthread firewall.o fw_socket.o config.o iptables.o fw_thread.o client_list.o gw_http.o common.o  lib_string.o -o firewall -lm -lcstl
clean :
	rm -f *.o
	rm -f firewall
install :
	cp firewall /usr/bin/firewall
