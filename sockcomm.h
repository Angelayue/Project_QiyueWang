#ifndef __SOCKCOMM_H
#define __SOCKCOMM_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <netdb.h>
#include <string.h>
#include "sockcomm.h"
#include <ctype.h> 
#include <signal.h>

typedef struct peer_element{ 
	char peer_ip_addr[80];
	int peer_port;
	int peer_number;
}peer_element;

void PrintTable(struct peer_element *peer_table);
void InitialTable(struct peer_element *peer_table);
int ConnectToServer(char* hostname, int port);
int ConnectToSeverWithName(char* hostname,int port);
int SocketInit(int port);
int AcceptConnection(int sockfd);

int AddToPeerTable(struct peer_element *peer_table, char* peer_addr);
int AddToNodeTable(struct peer_element *peer_table, char* new_info);
int DeleteNode(struct peer_element *peer_table, char* new_info);

char* NewInfoConstructor(char* msg, int peer_port);
char* GetNewNodeIP(char* newnode_info);
char* GetCommand(char* info);
char* GetReceiverIp(char* peer_msg);
int GetPortNum(char* peer_msg);
int GetRevPortNum(char* revaddr, struct peer_element *peer_table);

int RequestConnection(char* msg);
int RequestQuit(char* peer_msg);
int RequestAdd(char* newnode_info);
int RequestDelete(char* newnode_info);
int RequestGet(char* peer_msg);
int RequestShare(char* peer_msg);

int AskOtherNeighbors(char* GetMsg, struct peer_element *peer_table,char* exclude_ip);

int QuitNetwork(struct peer_element *peer_table, int self_port, char* central, int central_port);
int ListFiles(char* pathname);
char* Getfilename(char* pathname, char* commond);
int SendMsgToAllNeighbor(char* GetMsg, struct peer_element *peer_table);
int GetFiles(char* pathname,char* commond,struct peer_element *peer_table,char* self_addr, int self_port);
int ShareFiles(char* pathname, char* commond,struct peer_element *peer_table);
char* ConstituteGetMsg(char* filename, char* self_addr, int port_num);
#endif

