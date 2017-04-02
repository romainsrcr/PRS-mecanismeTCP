#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <pthread.h>
#include "connexionthread.h"



int main (int argc, char *argv[]) {
  //initialisation de la socket
  struct sockaddr_in adresse2, client;
  int port2;
  if (argc==3){
	    port2 = atoi(argv[1]);
  }
  else{
	    port2 = 5002;
  }
  int valid= 1;
  char buffer[RCVSIZE];
  //creation socket
  int desc2 = socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (desc2 < 0) {
    perror("cannot create socket 2\n");
    return -1;
  }
  //UDP
  setsockopt(desc2, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  adresse2.sin_family= AF_INET;
  adresse2.sin_port= htons(port2);
  adresse2.sin_addr.s_addr= htonl(INADDR_ANY);
  //bind
  if (bind(desc2, (struct sockaddr*) &adresse2, sizeof(adresse2)) == -1) {
    perror("Bind fail\n");
    close(desc2);
    return -1;
  }
  fd_set fs;
  int portmsg=port2;
  //boucle
  while (1) {

    FD_ZERO(&fs);

    FD_SET(desc2,&fs);
    //select
    int activite = select(desc2+1,&fs,NULL,NULL,NULL);
    //handle error
    if (activite < 0){
            printf("select error");
	    exit(0);
    }
    printf("Select\n");
    if(FD_ISSET(desc2,&fs)){
           printf("Connexion UDP\n");
           socklen_t taille=sizeof(client);
           //Connexion 3 handshake
           FD_SET(desc2,&fs);
           select(desc2+1,&fs,NULL,NULL,NULL);
           if (FD_ISSET(desc2,&fs)) {
             int synsize=recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
           }
           else {
             continue;
           }
           struct timeval timeout;
           timeout.tv_sec=10;
           timeout.tv_usec=500000;

           //impression du message+reinitialisaion buffer
           printf("%s reçu \n", buffer);

           //thread
           portmsg++;
           pthread_t pth;	// this is our thread identifier
           pthread_create(&pth,NULL,connection_handler,(void*) &portmsg);
           char synack[20]="SYN-ACK ";
           char portnum[10];
           sprintf(portnum,"%d",portmsg);
           strcat(synack,portnum);
           printf("on envoie %s \n",synack);

           int send=sendto(desc2,synack,strlen(synack),0,(struct sockaddr*)&client,sizeof(client));
           //fin thread

           memset(buffer,0,RCVSIZE);
           printf("SYN-ACK envoyé, en attente de ACK\n");
           FD_ZERO(&fs);
           FD_SET(desc2,&fs);
           select(desc2+1,&fs,NULL,NULL,&timeout);
           while (FD_ISSET(desc2,&fs)==0){
             FD_ZERO(&fs);
             FD_SET(desc2,&fs);
             send=sendto(desc2,synack,strlen(synack),0,(struct sockaddr*)&client,sizeof(client));
             int retour= select(desc2+1,&fs,NULL,NULL,&timeout);
             printf("Retour : %d\n",retour);
           }
           //impression message
           int acksize=recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
           printf("%s reçu \n",buffer);
           memset(buffer,0,RCVSIZE);
           //fin


           /*int msgSize= recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
           while (msgSize > 0) {
             sendto(desc2,buffer,msgSize,0,(struct sockaddr*)&client,sizeof(client));
             printf("%s",buffer);
             memset(buffer,0,RCVSIZE);
             msgSize= recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
           }*/
      }
    }
    close(desc2);
    return 0;
  }
