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

void *connection_handler(void *arg){

  int valid= 1;


  int port=*(int*) arg;

  printf("port : %d\n",port);

  struct sockaddr_in adresse_msg,client;//nouvelle socket pour messages

  //taille de la socket client
  socklen_t taille=sizeof(client);

  char buffer[RCVSIZE];

  char extension[5];

  //creation socket
  int desc2 = socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (desc2 < 0) {
    perror("cannot create socket msg\n");
    //return -1;
  }

  //UDP
  setsockopt(desc2, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  adresse_msg.sin_family= AF_INET;
  adresse_msg.sin_port= htons(port);
  adresse_msg.sin_addr.s_addr= htonl(INADDR_ANY);

  //Bind
  if (bind(desc2, (struct sockaddr*) &adresse_msg, sizeof(adresse_msg)) == -1) {
    perror("Bind fail\n");
    close(desc2);
    //return -1
  }

  // Reception fichier : client vers Serveur
  fd_set fs;
  // Modification
  FD_ZERO(&fs);
  FD_SET(desc2,&fs);
  memset(extension,0,5);
  recvfrom(desc2,extension,5,0,(struct sockaddr *)&client,&taille);
  char reception[16] = "reception.";
  strcat(reception,extension);
  printf("%s\n",reception);

  // Code ok
  FILE * fichierReceive;
  fichierReceive=fopen(reception,"wb");
  if (!fichierReceive)
      perror("Erreur ouverture du fichier\n");
  int numeroAck = 0;

  FD_ZERO(&fs);
  FD_SET(desc2,&fs);
  memset(buffer,0,RCVSIZE);
  select(desc2+1,&fs,NULL,NULL,NULL);
  recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
  fwrite(buffer,sizeof(char),RCVSIZE,fichierReceive);
  // Envoi de ACK_NUMERO
  numeroAck++;
  char* ack = ackNum(numeroAck);
  int sendAck = sendto(desc2,ack,strlen(ack),0,(struct sockaddr *)&client,taille);
  if (sendAck == -1)
    perror("Erreur d'envoi de ACK\n");

  while(strcmp(buffer,"END")!=0){
    FD_ZERO(&fs);
    FD_SET(desc2,&fs);
    memset(buffer,0,RCVSIZE);
    select(desc2+1,&fs,NULL,NULL,NULL);
    recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
    fwrite(buffer,sizeof(char),RCVSIZE,fichierReceive);
    // Envoi de ACK_NUMERO
    numeroAck++;
    ack = ackNum(numeroAck);
    sendAck = sendto(desc2,ack,strlen(ack),0,(struct sockaddr *)&client,taille);
    if (sendAck == (-1))
      perror("Erreur d'envoi de ACK\n");
    //free(ack);
  }
  free(ack);
  fclose(fichierReceive);


  // Ecrire du texte après l'envoi du fichier
  int msgSize= recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
  while (msgSize > 0) {
    printf("%s",buffer);
    memset(buffer,0,RCVSIZE);
    msgSize= recvfrom(desc2,buffer,RCVSIZE,0,(struct sockaddr *)&client,&taille);
   }


}

int tailleFichier(FILE* fichier) {
  long int taille;
  if (fichier!=NULL){
    fseek (fichier, 0, SEEK_END);
    taille = ftell (fichier);
    return taille;
  }
  else  return (-1);  // Renvoi -1 si le fichier n'as pas pu être ouvert
}

char* ackNum(int i){
  char *ack=NULL;
  ack=malloc(20*sizeof(char));
  char entier[8];
  sprintf(entier,"%d",i);
  strcat(ack,"ACK_");
  strcat(ack,entier);
  return(ack);
}
