#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>

#define RCVSIZE 1024

int main (int argc, char *argv[]) {

  struct sockaddr_in adresse1,adresse2, client;
  int port1;
  int port2;
  if (argc==3){
	port1 = atoi(argv[1]);
	port2 = atoi(argv[1]);
  }
  else{
	port1 = 5001;
	port2 = 5002;
  }
  int valid= 1;
  socklen_t alen= sizeof(client);
  char buffer[RCVSIZE];

  //create socket
  int desc1= socket(AF_INET, SOCK_STREAM, 0);
  int desc2 = socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (desc1 < 0) {
    perror("cannot create socket 1\n");
    return -1;
  }
  if (desc2 < 0) {
    perror("cannot create socket 2\n");
    return -1;
  }

  setsockopt(desc1, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
  setsockopt(desc2, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  adresse1.sin_family= AF_INET;
  adresse1.sin_port= htons(port1);
  adresse1.sin_addr.s_addr= htonl(INADDR_ANY);

  adresse2.sin_family= AF_INET;
  adresse2.sin_port= htons(port2);
  adresse2.sin_addr.s_addr= htonl(INADDR_ANY);

  if (bind(desc1, (struct sockaddr*) &adresse1, sizeof(adresse1)) == -1) {
    perror("Bind fail\n");
    close(desc1);
    return -1;
  }
  if (bind(desc2, (struct sockaddr*) &adresse2, sizeof(adresse2)) == -1) {
    perror("Bind fail\n");
    close(desc2);
    return -1;
  }


  if (listen(desc1, 5) < 0) {
    printf("Listen failed\n");
    return -1;
  }


  fd_set fs;

  while (1) {

    FD_ZERO(&fs);

    FD_SET(desc1,&fs);
    FD_SET(desc2,&fs);


    int activite = select(desc2+1,&fs,NULL,NULL,NULL);
    printf("select OK\n");
    if (activite < 0){
            printf("select error");
	    exit(0);
    }
    printf("Select\n");
    if(FD_ISSET(desc1,&fs)){
      printf("Connexion TCP\n");
      int rqst= accept(desc1, (struct sockaddr*)&client, &alen);
      printf("accept\n");
      int fils=fork();
      if (fils==0){
        close(desc1);
        int msgSize= recv(rqst,buffer,RCVSIZE,0);
        while (msgSize > 0) {
          send(rqst,buffer,msgSize,0);
          printf("%s",buffer);
          memset(buffer,0,RCVSIZE);
          msgSize= recv(rqst,buffer,RCVSIZE,0);
         }
        close(rqst);
      }
      else{
       close(rqst);
      }
   }
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
          //impression du message+reinitialisaion buffer
          struct timeval timeout;
          timeout.tv_sec=10;
          timeout.tv_usec=500000;
          printf("%s reçu \n", buffer);
          memset(buffer,0,RCVSIZE);
          char synack[]="SYN-ACK";
          int send=sendto(desc2,synack,strlen(synack),0,(struct sockaddr*)&client,sizeof(client));
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


          int msgSize= recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
          while (msgSize > 0) {
            sendto(desc2,buffer,msgSize,0,(struct sockaddr*)&client,sizeof(client));
            printf("%s",buffer);
            memset(buffer,0,RCVSIZE);
            msgSize= recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
           }
  }


  }

close(desc1);
return 0;
}
