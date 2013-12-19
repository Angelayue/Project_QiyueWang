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
#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/time.h>

int ConnectToServer(char* hostname, int port) {
  int sockfd;
		struct sockaddr_in sin;
		//create socket
		if((sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
		{perror("socket");return -1;}

		//setting up socket
		sin.sin_family=AF_INET;
		sin.sin_port=htons(port);
		sin.sin_addr.s_addr=inet_addr(hostname);
	
		//connect
		if(connect(sockfd,(struct sockaddr*)&sin, sizeof(sin))<0)
		{perror("connect");return -1;}

  return(sockfd);
}

int ConnectToSeverWithName(char* hostname,int port){
 int s;
struct sockaddr_in sin;
	//create socket
	if((s=socket(PF_INET,SOCK_STREAM,0))<0)
	{perror("socket");return -1;}

	//setting up socket
	sin.sin_family=AF_INET;
	sin.sin_port=htons(port);
	//hand the name to address function 
	struct hostent *server;
	server = gethostbyname(hostname);
    	if (server == NULL) 
	{perror("ERROR, no such host\n");exit(0);}
	bcopy((char *)server->h_addr, (char *)&sin.sin_addr.s_addr, server->h_length);
	//connect
	if(connect(s,(struct sockaddr*)&sin, sizeof(sin))<0)
	{perror("connect");return -1;}
return(s);
}

int SocketInit(int port) 
{
		struct sockaddr_in peer_serv;
		int sockfd;
		//socklen_t clilen;
		sockfd = socket(AF_INET, SOCK_STREAM,0);
		if(sockfd<0)
		{perror("opening socket"); return -1;}
		bzero((char*)&peer_serv, sizeof(peer_serv));
		peer_serv.sin_family = AF_INET;
		peer_serv.sin_addr.s_addr = INADDR_ANY;
		peer_serv.sin_port=htons(port);
		//bind		
		if(bind(sockfd, (struct sockaddr *) &peer_serv, sizeof(peer_serv))<0)
		{perror("bind");return -1;}
		//listen
		if(listen(sockfd,5)<0)
		{perror("listen");return -1;}
  		return(sockfd);
}

int AcceptConnection(int sockfd) {
	  int newsockfd;                   
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,&clilen);
			if(newsockfd<0)
			{perror("accept");return -1;}
	  return newsockfd;
}

int AddToPeerTable(struct peer_element *peer_table, char* peer_addr){
	int index_of_peer=0;
	int in_table=0; //0/1 not_in_table/in_table
	int in_table_index=0;
	int k=0;
	int vacant_slot=0;
	for(k=0;k<10;k++)
	{
		if (strcmp(peer_table[k].peer_ip_addr,"1.1.1.1")==0)
		{vacant_slot++;}
		if (strcmp(peer_addr,peer_table[k].peer_ip_addr)==0){
			printf("already in %dth elements of the table, update its information!\n",k);
			in_table=1;
			in_table_index=k;
		}
	}
	//printf("vacant_slot is %d\n",vacant_slot);
	if(in_table==0){
	index_of_peer=10-vacant_slot;	
	strcpy(peer_table[index_of_peer].peer_ip_addr,peer_addr);
	peer_table[index_of_peer].peer_port=20000+index_of_peer;//ask peer open this port	
	peer_table[index_of_peer].peer_number=index_of_peer;
	printf("new peer join! %d th node %s\n",index_of_peer,peer_table[index_of_peer].peer_ip_addr);
	}
	else{
	index_of_peer=in_table_index;
	strcpy(peer_table[index_of_peer].peer_ip_addr,peer_addr);
	//ask TA about update portnumber, does it need to be implement, forward to the connected peer??
	peer_table[index_of_peer].peer_port=20000+index_of_peer;	
	peer_table[index_of_peer].peer_number=index_of_peer;
	printf("already in %d th node %s\n",index_of_peer,peer_table[index_of_peer].peer_ip_addr);
	}
	return index_of_peer;
}

int AddToNodeTable(struct peer_element *peer_table, char* new_info){
	int index_of_peer=0;
	int in_table=0; //0/1 not_in_table/in_table
	int in_table_index=0;
	int k=0;
	int vacant_slot=0;
	char peer_addr[80];
	strcpy(peer_addr,GetNewNodeIP(new_info));
	for(k=0;k<10;k++)
	{
		if (strcmp(peer_table[k].peer_ip_addr,"1.1.1.1")==0)
		{vacant_slot++;}
		if (strcmp(peer_addr,peer_table[k].peer_ip_addr)==0){
			printf("already in %dth elements of the table, update its information!\n",k);
			in_table=1;
			in_table_index=k;
		}
	}
	//printf("vacant_slot is %d\n",vacant_slot);
	if(in_table==0){
	index_of_peer=10-vacant_slot;	
	strcpy(peer_table[index_of_peer].peer_ip_addr,peer_addr);
	peer_table[index_of_peer].peer_port=GetPortNum(new_info);//ask peer open this port	
	peer_table[index_of_peer].peer_number=index_of_peer;
	printf("new peer join! %d th node %s with port %d\n",index_of_peer,peer_table[index_of_peer].peer_ip_addr,peer_table[index_of_peer].peer_port);
	}
	else{
	index_of_peer=in_table_index;
	strcpy(peer_table[index_of_peer].peer_ip_addr,peer_addr);
	//ask TA about update portnumber, does it need to be implement, forward to the connected peer??
	peer_table[index_of_peer].peer_port=GetPortNum(new_info);	
	peer_table[index_of_peer].peer_number=index_of_peer;
	printf("already in %d th node %s\n",index_of_peer,peer_table[index_of_peer].peer_ip_addr);
	}
	return index_of_peer;
}

int DeleteNode(struct peer_element *peer_table, char* new_info){
	char peer_addr[80];
	strcpy(peer_addr,GetNewNodeIP(new_info));
	int k=0;
	int returnval=1;
	for(k=0;k<10;k++){
		if (strcmp(peer_addr,peer_table[k].peer_ip_addr)==0){
			printf("find in %dth elements of the table,Delete it!\n",k);
			strcpy(peer_table[k].peer_ip_addr, "1.1.1.1");	
			peer_table[k].peer_port=0;	
			returnval=k;
		}
	}
	return returnval;
}

char* NewInfoConstructor(char* msg, int peer_port){
	char self[80];
	strcpy(self,msg);
	char self_port[6];
	int j=peer_port;
	snprintf(self_port, 6,"%d",j);
	strcat(self,":");
	strcat(self,self_port);
	return self;
}

char* GetNewNodeIP(char* newnode_info){
	char line[80];
	strcpy(line, newnode_info);
	char * pch;
	char temp_line[80];
	pch=strtok(line, ":");
	strcpy(temp_line,pch);
	//printf("new ip is %s\n", temp_line);
	return temp_line;
}

int GetPortNum(char* peer_msg){
	int portnum=0;
	char line[80];
	strcpy(line,peer_msg);
	char* pch=strtok(line, ":");
	pch=strtok(NULL,":");
	//printf("pch is %d\n",atoi(pch));
	portnum=atoi(pch);
	return portnum;
}

char* GetCommand(char* info){
	char line[80];
	strcpy(line, info);
	char * pch;
	char temp_line[80];
	pch=strtok(line, ":");
	strcpy(temp_line,pch);
	//printf("new command is %s\n", temp_line);
	return temp_line;
}

char* GetReceiverIp(char* peer_msg){
	char line[80];
	strcpy(line,peer_msg);
	char * pch;
	pch=strtok(line, " ");
	pch=strtok(NULL," ");
	pch=strtok(NULL," ");
	char temp_line[80];
	strcpy(temp_line,pch);
	int len = strlen(temp_line);
	if(temp_line[len-1] == '\n' )
    	temp_line[len-1] = '\0';
//	printf("new ip is %s\n", temp_line);
	return temp_line;
}

int GetReceiverPort(char* peer_msg){
	char line[80];
	strcpy(line,peer_msg);
	char * pch;
	pch=strtok(line, " ");
	pch=strtok(NULL," ");
	pch=strtok(NULL," ");
	pch=strtok(NULL,"\0");
	char temp_line[80];
	strcpy(temp_line,pch);
	//printf("new port is %s\n", temp_line);
	return atoi(temp_line);
}

int GetRevPortNum(char* revaddr, struct peer_element *peer_table){
	int returnval=0;
	int k=0;
	char peer_addr[80];
	strcpy(peer_addr, revaddr);
	int len = strlen(peer_addr);
	if(peer_addr[len-1] == '\n' )
    	peer_addr[len-1] = '\0';
	//printf("peer_addr is %s\n",peer_addr);
	//printf("peer_table[0].peer_ip_addr is %s\n",peer_table[0].peer_ip_addr);
	for(k=0;k<10;k++){
		if (strcmp(peer_addr,peer_table[k].peer_ip_addr)==0){
			printf("find in %dth elements of the table\n",k);
			returnval=peer_table[k].peer_port;
		}
	}
	return returnval;
}

void InitialTable(struct peer_element *peer_table){
int k=0;
for(k=0;k<10;k++){
	strcpy(peer_table[k].peer_ip_addr,"1.1.1.1");
	peer_table[k].peer_port=0;
	peer_table[k].peer_number=k;
}
}



void PrintTable(struct peer_element *peer_table){
	int k=0;
	for(k=0;k<10;k++){
	printf("peer_table[%d].peer_ip_addr is %s\n",k,peer_table[k].peer_ip_addr);
	printf("peer_table[%d].peer_port is %d \n",k,peer_table[k].peer_port);
//	peer_table[k].peer_number=k;
	}
}

int RequestConnection(char* msg){
	int returnVal=0;
	char* command=GetCommand(msg);
	if(strcmp(command,"request_for_peer_network_connection")==0)
	{returnVal=1;}
	return returnVal;
}

int RequestQuit(char* peer_msg){
	int returnVal=0;
	char* command=GetCommand(peer_msg);
	if(strcmp(command,"Quit_from_peer_network_connection")==0)
	{returnVal=1;}
	return returnVal;
}

int RequestAdd(char* newnode_info){
	int returnVal=0;
	char* command=GetCommand(newnode_info);
	if(strcmp(command,"Add_New_Node")==0)
	{returnVal=1;}
	return returnVal;
}

int RequestDelete(char* newnode_info){
	int returnVal=0;
	char* command=GetCommand(newnode_info);
	if(strcmp(command,"Delete_Node")==0)
	{returnVal=1;}
	return returnVal;
}

int RequestGet(char* peer_msg){
	int returnVal=0;
	char line[80];
	strcpy(line, peer_msg);
	char * pch;
	char temp_line[80];
	pch=strtok(line, " ");
	strcpy(temp_line,pch);
	if(strcmp(temp_line,"GET")==0){returnVal=1;}
	return returnVal;	
}

int RequestShare(char* peer_msg){
	int returnVal=0;
	char line[80];
	strcpy(line, peer_msg);
	char * pch;
	char temp_line[80];
	pch=strtok(line, " ");
	strcpy(temp_line,pch);
	if(strcmp(temp_line,"Share")==0){returnVal=1;}
	return returnVal;	
}

int QuitNetwork(struct peer_element *peer_table, int self_port, char* central, int central_port){
	int k=0;
	for (k=0;k<10;k++){
	 if(strcmp(peer_table[k].peer_ip_addr,"1.1.1.1")!=0 &&
		 strcmp(peer_table[k].peer_ip_addr,"No_longer_exit!")!=0){	
		int sockfd;
		char msg_p[80]="Quit_from_peer_network_connection";
		strcpy(msg_p,NewInfoConstructor(msg_p,self_port));
	//	printf("msg is %s\n",msg_p);		
		sockfd=ConnectToServer(peer_table[k].peer_ip_addr, peer_table[k].peer_port);
		//write to server 
		if(write(sockfd,msg_p,strlen(msg_p)+1)<0)
		{perror("write"); return -1;}
		//close
		if(close(sockfd)<0){perror("close");return -1;}
		}
	}
	//quit central server
	int centralfd;
	char msg_p[80]="Quit_from_peer_network_connection";
	strcpy(msg_p,NewInfoConstructor(msg_p,self_port));
	centralfd=ConnectToSeverWithName(central, central_port);
	if(write(centralfd,msg_p,strlen(msg_p)+1)<0)
		{perror("write"); return -1;}
	//close
		if(close(centralfd)<0){perror("close");return -1;}
	return 1;	
}

int ListFiles(char* pathname){
	int returnval=1;
	char* arg_list[] = {
   	 "ls",     /* argv[0], the name of the program.  */
	pathname,
   	 NULL      /* The argument list must end with a NULL.  */
  	};
	pid_t pid=fork();
	if(pid==0){
		execvp ("ls",arg_list);
		fprintf (stderr, "an error occurred in execvp\n");
		returnval=0;
	}
	return returnval;
}

int ShareFiles(char* pathname,char* commond,struct peer_element *peer_table){
	int returnval=0;
	//get the filename
	char filename[80];
	strcpy(filename,Getfilename(pathname,commond));
	//printf("filename is %s!!!\n",filename);
	//if cannot open, print a msg
	FILE *fs = fopen(filename, "r");
	if(fs == NULL)
	{perror("OOps! no such file!");}
	//if can open, send it
	else{
	//get the receiver's addr
	char revaddr[80];
	strcpy(revaddr, GetReceiverIp(commond));
	//printf("revaddr is %s\n",revaddr);
	struct hostent *server;
        server = gethostbyname(revaddr);
        if (server == NULL)
        {perror("ERROR, no such peer\n");exit(0);}
	strcpy(revaddr,inet_ntoa(*(struct in_addr *)server->h_addr_list[0]));
	//get the port number
	int revport=GetRevPortNum(revaddr,peer_table);
	if(revport==0)
	{printf("Sorry, cannot find in share list!!\n");}
	else{
		//connect to rev
		int sock=ConnectToServer(revaddr, revport);
		//write a short msg for notice send file
		char msg[80];
		strcpy(msg,"Share ");
		strcat(msg, filename);
		if(write(sock,msg,strlen(msg)+1)<0)
		{perror("write"); return -1;}
		int n;
		if((n=read(sock,msg, sizeof(msg)))<0)
		{perror("read"); return -1;}
		//printf("%s\n",  msg);
		close(sock);		
		if(strcmp(msg, "Ok, I am ready!")==0)
		{
			int sd=ConnectToServer(revaddr, revport+20000);
			//send file
			char sdbuf[80]; 				
			bzero(sdbuf, 80); 
			int fs_block_sz; 
			while((fs_block_sz = fread(sdbuf, sizeof(char), 80, fs)) > 0)
			{
				if(send(sd, sdbuf, fs_block_sz, 0) < 0)
				 {perror("ERROR: Failed to send file\n");break;}
				 bzero(sdbuf, 80);
			 }
		 printf("Ok File %s from host was sent!\n", filename);
			returnval=1;
		close(sock);
		}
		}
	}
	return returnval;
}

int GetFiles(char* pathname,char* commond,struct peer_element *peer_table,char* self_addr, int self_port){
	int returnval=0;
	//get the filename
	char filename[80];
	strcpy(filename,Getfilename(pathname,commond));
	//printf("filename is %s",filename);
	//construct new get msg
	char Getmsg[80];
	strcpy(Getmsg,ConstituteGetMsg(filename,self_addr,self_port+10000));
	//printf("Getmsg is %s\n",Getmsg);
	//send the msg
	if(SendMsgToAllNeighbor(Getmsg, peer_table)==0){perror("Error: sendMsg");}
	//open listening socket
	int sockfd=SocketInit(self_port+10000);
	int newsockfd;
	struct sockaddr_in sin;
	int sinlen=sizeof(sin);
	
	
	//get file

	
	int maxfdp;
	fd_set fds;/*M*/
	struct timeval timeout={1,0};/*M*/

		FD_ZERO(&fds);/*M*/
	        FD_SET(sockfd,&fds);/*M*/
        	maxfdp=sockfd+1;/*M*/
        	select(maxfdp,&fds,NULL,NULL,&timeout);/*M*/
        	if(FD_ISSET(sockfd,&fds)){
		if((newsockfd=accept(sockfd,(struct sockaddr*)&sin, &sinlen))<0)
		{perror("accept");return -1;}
		char revbuf[80]; 
		FILE *fr = fopen(filename, "a");
		if(fr == NULL)
		{printf("File %s Cannot be opened file on server.\n", filename);}
		bzero(revbuf, 80); 
		int fr_block_sz = 0;
		while((fr_block_sz = recv(newsockfd, revbuf, 80, 0)) > 0) 
		{
			int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
			if(write_sz < fr_block_sz){error("File write failed on server.\n");}
			bzero(revbuf, 80);
			if (fr_block_sz == 0 || fr_block_sz != 512) {break;}
		}
		if(fr_block_sz < 0)	
		{
			if (errno == EAGAIN){printf("recv() timed out.\n");}
	        	else {fprintf(stderr, "recv() failed due to errno = %d\n", errno);exit(1);}
		}
		printf("Ok received from client!\n");
		fclose(fr); 
		}
		else{printf("time out\n");}

	
	returnval=1;
	
	close(newsockfd);
	close (sockfd);
	return returnval;
}

int SendMsgToAllNeighbor(char* GetMsg, struct peer_element *peer_table){
	int returnval=1;
	int k=0;	
	for(k=0;k<10;k++){
		if(strcmp(peer_table[k].peer_ip_addr,"1.1.1.1")!=0)
		{
		  int sockfd;
		  sockfd=ConnectToServer(peer_table[k].peer_ip_addr, peer_table[k].peer_port);
		//write to server 
		if(write(sockfd,GetMsg,strlen(GetMsg)+1)<0)
		{perror("write"); returnval= 0;}
		//close
		if(close(sockfd)<0){perror("close");returnval=0;}
		}	
	}
	return returnval;
} 

int  AskOtherNeighbors(char* GetMsg, struct peer_element *peer_table,char* exclude_ip){	
	int returnval=1;
	int k=0;	
	for(k=0;k<10;k++){
		if(strcmp(peer_table[k].peer_ip_addr,"1.1.1.1")!=0 && strcmp(peer_table[k].peer_ip_addr,exclude_ip)!=0)
		{
		  int sockfd;
		  sockfd=ConnectToServer(peer_table[k].peer_ip_addr, peer_table[k].peer_port);
		//write to server 
		if(write(sockfd,GetMsg,strlen(GetMsg)+1)<0)
		{perror("write"); returnval= 0;}
		//close
		if(close(sockfd)<0){perror("close");returnval=0;}
		}	
	}
	return returnval;
}

char* Getfilename(char* pathname, char* commond){
	char line[80];
	strcpy(line,commond);
	int len = strlen(line);
	if( line[len-1] == '\n' )
    	line[len-1] = '\0';
	char* pch=strtok(line, " ");
	pch=strtok(NULL," ");
	//pch=strtok(NULL,"\n");
	//printf("pch is %s\n",pch);
	return pch;
}

char* ConstituteGetMsg(char* filename, char* self_addr, int port_num){
	char getMsg[80]="GET ";
	//printf("yes\n");
	char line[20];
	strcpy(line, filename);
	char addr[20];
	strcpy(addr, self_addr);
	//printf("addr is %s\n",addr);
	int len = strlen(addr);
	if(addr[len-1] == '\n' )
    	addr[len-1] = '\0';
	//printf("addr is %s\n",addr);
	strcat(getMsg,line);
	strcat(getMsg," ");
	strcat(getMsg,addr);
	strcat(getMsg," ");
	int j=port_num;
	char self_port[6];
	snprintf(self_port, 6,"%d",j);
	strcat(getMsg, self_port);
	//printf("getMsg is %s\n",getMsg);
	return getMsg;
}
