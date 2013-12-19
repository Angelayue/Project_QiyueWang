
all:
	gcc -usr/lib/lsocket -lnsl central-server.c sockcomm.c -o central-server
	gcc -usr/lib/lsocket -lnsl peer.c sockcomm.c -o peer
