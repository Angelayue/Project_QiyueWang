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

struct peer_element peer_table[10];
	
int main(int argc, char *argv[])
{
	
	int s,t;
	int num_of_peer=0;
	char msg[80];
	fd_set readfds;
	//peer_element *peer_table=(peer_element)malloc(10*sizeof(element));
	InitialTable(peer_table);
	//struct peer_element peer_table[10];
	//check prameter
	if(argc<2){printf("%s port\n", argv[0]); return -1;}//maybe modified
	s=SocketInit(atoi(argv[1]));
	int index_of_peer=0;
	int Active=1;
	while(Active)
	{
		if(index_of_peer==num_of_peer)
		{num_of_peer=num_of_peer+1;}	
		//PrintTable(peer_table);
		if(num_of_peer>9)
			{perror("cannnot accept more peers!");}//need more modification		


		FD_ZERO(&readfds);          /* initialize the fd set */
    		FD_SET(s, &readfds); /* add socket fd */
    		FD_SET(0, &readfds);        /* add stdin fd (0) */
		if (select(s+1, &readfds, 0, 0, 0) < 0) 
		{error("ERROR in select");}
		char buf[80];
		
		/* if the user has entered a command, process it */
   		 if (FD_ISSET(0, &readfds)) {
      			fgets(buf, 80, stdin);
     			 switch (buf[0]) {
     		 case 'q': /* print the connection cnt */
			Active=0;//quit
			break;
		default: /*bad input*/
			printf("ERROR: unknown command\n");
			fflush(stdout);
      			}
    		}  
 

		/* if a connection request has arrived, process it */
    		if (FD_ISSET(s, &readfds)) {
		//accept
		struct sockaddr_in sin;
		int sinlen=sizeof(sin);
		if((t=accept(s,(struct sockaddr*)&sin, &sinlen))<0)
		{perror("accept");return -1;}
		//get msg
		char* peer_addr=inet_ntoa(sin.sin_addr);
		int peer_port=ntohs(sin.sin_port);
		//printf("From %s: %d.\n", peer_addr,peer_port);
		//add to table
		index_of_peer=AddToPeerTable(peer_table, peer_addr);
		//printf("index_of_peer is %d\n",index_of_peer);	
		peer_port=peer_table[index_of_peer].peer_port;
		
		//child process: serve client!
		int fd[2];
		char newnode_info[80]="nothing";
		pipe(fd);
		pid_t pid=fork();
		if(pid==0){
			if(read(t,msg,sizeof(msg))<0)
			{perror("read");return -1;}
	
			//check msg for request!
			if(strcmp(msg,"request_for_connection")==0){
	
			//choose a node in the network
			if(index_of_peer!=0){
			int r_index=rand()%num_of_peer;
			//make sure whether the index is not itself
			while(r_index==index_of_peer){r_index=rand()%num_of_peer;}
			printf("r_index is %d\n",r_index);	
			char newmsg[80];
			strncpy(newmsg,peer_table[r_index].peer_ip_addr,sizeof(peer_table[r_index].peer_ip_addr));
			strcat(newmsg,":");
			char port[6];
			int i = peer_table[r_index].peer_port;
			snprintf(port, 6,"%d",i);
			//itoa(i,port,10);			
			strcat(newmsg,port);
			strcpy(msg, newmsg);
			}

			else{
			strcpy(msg,"getconnect!");
			}
	
	
			char self[80];
			strcpy(self,peer_addr);
			char self_port[6];
			int j=peer_port;
			snprintf(self_port, 6,"%d",j);
			strcat(self,":");
			strcat(self,self_port);
			strcat(msg,"/");
			strcat(msg,self);
	
			if(write(t,msg,sizeof(msg)+1)<0)
			{perror("write");return -1;}
	
			if(close(t)<0)
			{perror("close"); return -1;}
			
		}//end of send msg to peer
		else if(RequestQuit(msg)==1)
				{
				close(fd[0]);
				peer_port=GetPortNum(msg);
				char* info=NewInfoConstructor("Delete_Node",peer_port);
				printf("admin: disconnected from '%s(%d)'\n", peer_addr, peer_port);
				strcpy(newnode_info,info);
				//strcat(newnode_info,info);
        			write(fd[1], &newnode_info, sizeof(newnode_info));
        			//printf("Child(%d) send value: %s\n", getpid(), newnode_info);
			        // close the read-descriptor
			        close(fd[1]);					
				}
		exit(EXIT_SUCCESS);
	    }//end pid
		else{
				close(fd[1]);
				read(fd[0], &newnode_info, sizeof(newnode_info));
			//	printf("parent(%d) received value: %s\n", getpid(), newnode_info);
        			close(fd[0]);
				if(RequestDelete(newnode_info)==1){        			
				char Node_addr[80];
				int PeerPort=GetPortNum(newnode_info);
				strcpy(Node_addr,NewInfoConstructor(peer_addr,PeerPort));
				int index_of_peer=DeleteNode(peer_table, Node_addr);
			//	printf("index_of_peer is %d\n",index_of_peer);	
				}		
		}
	}//end of select
	
	}	//end of while
	//need more modifed (add a while)	
	if(close(s)<0)
	{perror("close"); return -1;}
	return 0;
}
