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
#include <sys/stat.h>
struct peer_element node_table[10];//the worst case,became a star topology
int main(int argc, char* argv[])
{
	int s;
	
	char msg[80]="request_for_connection";//need modified
	int n;
	InitialTable(node_table);

	//check parameter asking for host port, in this part, the host port maybe public
	if(argc<4)
	{printf("%s pathname host port\n", argv[0]); return -1;}

	s=ConnectToSeverWithName(argv[2],atoi(argv[3]));	
	//write to server 
	if(write(s,msg,strlen(msg)+1)<0)
	{perror("write"); return -1;}
	
	if((n=read(s,msg, sizeof(msg)))<0)
	{perror("read"); return -1;}
	//printf("%s\n",  msg);
	char pathname[80];
    	strcpy(pathname,argv[1]);
	struct stat myFile;
	    if (stat(pathname, &myFile) < 0)
	    {// Doesn't exist
	        perror("file dir not exit!");
	    }
	    else if (!S_ISDIR(myFile.st_mode))
	    {// Exists but is not a directory
	        perror("not a dir! right format is ~/XX/XX");
	    }
		//close
		if(close(s)<0){perror("close");return -1;}
	
/*******************************************************************************************/
/**************End the central server part! construct peer to peer network******************/
/*******************************************************************************************/
	char line[80],line2[80];
	strcpy(line, msg);
	strcpy(line2,msg);
	char * pch;
	char temp_line[80];
	pch=strtok(line, "/");
	strcpy(temp_line,pch);
	//printf("temp_line is %s\n", temp_line);
	char peer_msg[80];
	char self_addr[80];
	int self_port;
	if(strcmp(temp_line, "getconnect!")==0)//find it is the frist node
	//just open its socket!
	{	
		pch = strtok(NULL,"/");
		strcpy(temp_line,pch);
		//printf("temp_line is %s\n", temp_line);
		pch=strtok(temp_line,":");
		strcpy(self_addr,pch);
		self_port= atoi(strtok(NULL, ":"));
		//printf("self_addr is %s\n", self_addr);
	}
	//if it is not the frist node, join other node!
	else
	{		
		pch = strtok(line2,"/");
		pch = strtok(NULL,"/");
		//strcpy(temp_line,pch);
		//printf(temp_line);
		char ance_addr[80];	
		strcpy(ance_addr,GetNewNodeIP(temp_line));
		int ance_port= GetPortNum(temp_line);
		//printf("ance_addr is %s, ance_port is %d\n",ance_addr, ance_port);
		char* ance_info=NewInfoConstructor(ance_addr,ance_port);
		AddToNodeTable(node_table, ance_info);
		strcpy(temp_line,pch);
		//printf("new temp is %s\n",temp_line);
		strcpy(self_addr,GetNewNodeIP(pch));		
		self_port=GetPortNum(pch);
		//printf("self_addr is %s\n", self_addr);	
		//send the connection message
		int sockfd;
		struct sockaddr_in ancestor_addr;
		char msg_p[80]="request_for_peer_network_connection";//need modified
		strcpy(msg_p,NewInfoConstructor(msg_p,self_port));
		//printf("msg is %s\n",msg_p);		
		int rev;
		sockfd=ConnectToServer(ance_addr, ance_port);
		//write to server 
		if(write(sockfd,msg_p,strlen(msg_p)+1)<0)
		{perror("write"); return -1;}
		//read from server
		if((rev=read(s,msg_p, sizeof(msg_p)))<0)
		{perror("read"); return -1;}
		printf("%s\n",  msg_p);
		//close
		if(close(s)<0){perror("close");return -1;}
	}	
/****************************************************************************************************/
/*******************************start open its own connection****************************************/
/****************************************************************************************************/
	
		int sockfd2=SocketInit(self_port);
		int num_of_node=0;
		fd_set readfds;
		int Active=1;// 1/0 Active/deconnected
		while(Active)
		{
			FD_ZERO(&readfds);          /* initialize the fd set */
    			FD_SET(sockfd2, &readfds); /* add socket fd */
    			FD_SET(0, &readfds);        /* add stdin fd (0) */
			if (select(sockfd2+1, &readfds, 0, 0, 0) < 0) 
			{error("ERROR in select");}
			char buf[80];
		
			/* if the user has entered a command, process it */
   		 	if (FD_ISSET(0, &readfds)) {
      				fgets(buf, 80, stdin);
     			 switch (buf[0]) {
     				 case 'q': /* quit*/
					if(QuitNetwork(node_table,self_port,argv[2],atoi(argv[3]))==1)
						{printf("sucessfully quit!\n"); Active=0;}
					 else{perror("ERROR, quit\n");exit(0);}
				 break;
			
				 case 'l': /* list */
					if(ListFiles(pathname)==0)
						//{printf("sucessfully list!\n");}
					{perror("ERROR, list\n");}
				 break;

				 case 'g': /* get */
					if(GetFiles(pathname,buf,node_table,self_addr,self_port)==0)
						//{printf("sucessfully list!\n");}
					{perror("ERROR, get\n");}
				 break;
				 
				 case 's': /* share */
					if(ShareFiles(pathname,buf,node_table)==0)
						//{printf("sucessfully list!\n");}
					{perror("ERROR, share\n");}
				 break;
			 default: /*bad input*/
				printf("ERROR: unknown command\n");
				fflush(stdout);
      			 }
    			}  
 

		/* if a connection request has arrived, process it */
    		if (FD_ISSET(sockfd2, &readfds)) {
			int newsockfd;
			struct sockaddr_in sin;
			int sinlen=sizeof(sin);
			if((newsockfd=accept(sockfd2,(struct sockaddr*)&sin, &sinlen))<0)
			{perror("accept");return -1;}
			char* peer_addr=inet_ntoa(sin.sin_addr);
			int peer_port=ntohs(sin.sin_port);
			//printf("From %s: %d.\n", peer_addr,peer_port);
			printf("start accept!!\n");
			int fd[2];
			char newnode_info[80]="nothing";
			pipe(fd);	
			pid_t pid=fork();	
			if(pid==0)
			{
				//read
				if(read(newsockfd,peer_msg,sizeof(peer_msg))<0)
				{perror("read");return -1;}
				printf(peer_msg);
				printf("\n");
				//check msg for request!
				/*********************************************************/
				/**********Request for Connection*************************/
				/*********************************************************/
				if(RequestConnection(peer_msg)==1)
				{
				char sendmsg[80]="hello my following peer!";
				//write				
				if(write(newsockfd,sendmsg,sizeof(sendmsg)+1)<0)
				{perror("write");return -1;}
        			close(fd[0]);
				peer_port=GetPortNum(peer_msg);
				char* info=NewInfoConstructor("Add_New_Node",peer_port);
				strcpy(newnode_info,info);
        			write(fd[1], &newnode_info, sizeof(newnode_info));
        			//printf("Child(%d) send value: %s\n", getpid(), newnode_info);
			        // close the read-descriptor
			        close(fd[1]);				
				}
				/*********************************************************/
				/****************Request for quit*************************/
				/*********************************************************/
				else if(RequestQuit(peer_msg)==1)
				{
				close(fd[0]);
				peer_port=GetPortNum(peer_msg);
				char* info=NewInfoConstructor("Delete_Node",peer_port);
				printf("admin: disconnected from '%s(%d)'\n", peer_addr, peer_port);
				strcpy(newnode_info,info);
				//strcat(newnode_info,info);
        			write(fd[1], &newnode_info, sizeof(newnode_info));
        			//printf("Child(%d) send value: %s\n", getpid(), newnode_info);
			        // close the read-descriptor
			        close(fd[1]);					
				}
				/*********************************************************/
				/****************Request for get**************************/
				/*********************************************************/
				else if(RequestGet(peer_msg)==1)
				{
				close(fd[0]);
				close(fd[1]);
				//get filename
				char filename[80];
				strcpy(filename,Getfilename(pathname,peer_msg));
				char sdbuf[80]; 				
				//if cannot open, send to neighbor
				FILE *fs = fopen(filename, "r");
				if(fs == NULL)
				{printf("OOps! File %s not found. ask other neighbor\n", filename);
					if(AskOtherNeighbors(peer_msg,node_table,peer_addr)==0)
					{perror("Error, ask other neighbors!\n");}				
				}
				//if can open, send it
				else{
					bzero(sdbuf, 80); 
					int fs_block_sz; 
					char receiver_ip[80];
					strcpy(receiver_ip,GetReceiverIp(peer_msg));
					int receiver_port=GetReceiverPort(peer_msg);
					int filesk=ConnectToServer(receiver_ip, receiver_port);
					while((fs_block_sz = fread(sdbuf, sizeof(char), 80, fs)) > 0)
					{
					    if(send(filesk, sdbuf, fs_block_sz, 0) < 0)
					    {perror("ERROR: Failed to send file\n");break;}
					    bzero(sdbuf, 80);
					}
					printf("Ok File %s from host was Sent!\n", filename);
					//close(fs);
					//close(filsk);
	       			   }
				}
				/*********************************************************/
				/****************Request for share************************/
				/*********************************************************/
				else if(RequestShare(peer_msg)==1){
					char sendmsg[80]="Ok, I am ready!";
				//write				
				if(write(newsockfd,sendmsg,sizeof(sendmsg)+1)<0)
				{perror("write");return -1;}
				int filefd=SocketInit(self_port+20000);
				int filefs;
				struct sockaddr_in sin;
				int sinlen=sizeof(sin);
				if((filefs=accept(filefd,(struct sockaddr*)&sin, &sinlen))<0)
				{perror("accept");return -1;}
				//get file
				char filename[80];
				strcpy(filename,Getfilename(pathname,peer_msg));
				char revbuf[80]; 
				FILE *fr = fopen(filename, "a");
				if(fr == NULL)
				{printf("File %s Cannot be opened file on server.\n", filename);}
				else
				{
					bzero(revbuf, 80); 
					int fr_block_sz = 0;
					while((fr_block_sz = recv(filefs, revbuf, 80, 0)) > 0) 
					{
					int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
					if(write_sz < fr_block_sz){error("File write failed on server.\n");}
					bzero(revbuf, 80);
					if (fr_block_sz == 0 || fr_block_sz != 512) {break;}
					}
					
					if(fr_block_sz < 0)
					{
  					 if (errno == EAGAIN){printf("recv() timed out.\n");}
	        			 else{fprintf(stderr, "recv() failed due to errno = %d\n", errno);exit(1);}
					}
       				 
					printf("Ok received from client!\n");				
				}
				}
				//else{perror("wrong message!");}
				if(close(newsockfd)<0)
				{perror("close"); return -1;}
				exit(EXIT_SUCCESS);
			}
			else{
				close(fd[1]);
        			read(fd[0], &newnode_info, sizeof(newnode_info));
       				//printf("parent(%d) received value: %s\n", getpid(), newnode_info);
        			close(fd[0]);
				/*********************************************************/
				/**********Add new node***********************************/
				/*********************************************************/
				if(RequestAdd(newnode_info)==1){
				char newNode_addr[80];
				int newPeerPort=GetPortNum(newnode_info);
				strcpy(newNode_addr,NewInfoConstructor(peer_addr,newPeerPort));
				int index_of_peer=AddToNodeTable(node_table, newNode_addr);
				}
				/*********************************************************/
				/**********Delete peer node*******************************/
				/*********************************************************/
				if(RequestDelete(newnode_info)==1){
				char Node_addr[80];
				int PeerPort=GetPortNum(newnode_info);
				strcpy(Node_addr,NewInfoConstructor(peer_addr,PeerPort));
				int index_of_peer=DeleteNode(node_table, Node_addr);
				//printf("index_of_peer is %d\n",index_of_peer);	
				}		
			}
			
		   }//end select
		}//end while
		if(close(sockfd2)<0)
			{perror("close"); return -1;}	
	return 0;
}
