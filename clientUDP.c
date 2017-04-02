#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "fonctionClient.h"

#define RCVSIZE 1024

int main (int argc, char *argv[]) {
  fd_set fs;
  FD_ZERO(&fs);
  struct sockaddr_in adresse, adresseEchange;
  int port;
  if (argc==3){
	port = atoi(argv[2]);
  }
  else{
	port = 5002;
  }
  int valid= 1;
  char msg[RCVSIZE];
  char blanmsg[RCVSIZE];
  char acquittement[20];

  //create socket
  int desc= socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (desc < 0) {
    perror("cannot create socket\n");
    return -1;
  }

  setsockopt(desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  adresse.sin_family= AF_INET;
  adresse.sin_port= htons(port);
   if (argc==3){
	inet_aton(argv[1],&adresse.sin_addr.s_addr);
  }
  else{
	adresse.sin_addr.s_addr= htonl(INADDR_LOOPBACK);
  }
  printf("adresse : %d\n",adresse.sin_addr.s_addr);



  socklen_t taille=sizeof(adresse);

  //3 handshake
  printf("debut 3 handshake\n");
  char syn[]="SYN";
  ssize_t send=sendto(desc,syn,strlen(syn),0,(struct sockaddr *)&adresse,taille);
  printf("SYN envoyé, attend ...\n");
  FD_SET(desc,&fs);
  struct timeval timeout;
  timeout.tv_sec=10;
  timeout.tv_usec=500000;
  select(desc+1,&fs,NULL,NULL,&timeout);
  while(FD_ISSET(desc,&fs)==0){
    printf("renvoi du message\n");
    FD_ZERO(&fs);
    FD_SET(desc,&fs);
    send=sendto(desc,syn,strlen(syn),0,(struct sockaddr *)&adresse,taille);
    printf("Send valeur : %li %s\n",send,syn);
    select(desc+1,&fs,NULL,NULL,&timeout);
  }
  int synacksize=recvfrom(desc,blanmsg,sizeof(blanmsg),0,(struct sockaddr *)&adresse,&taille);
  printf("%s reçu\n",blanmsg);

  //recuperation du port de la nouvelle connexion
  int newport;
  char synack[20];
  sscanf(blanmsg, "%s %d",synack,&newport);
  printf("nouveau port : %d \n",newport);
  //end

//-----------
  //creation SOCKET ECHANGE
  int descEchange= socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (descEchange < 0) {
    perror("cannot create socket\n");
    return -1;
  }

  adresseEchange.sin_family= AF_INET;
  adresseEchange.sin_port= htons(newport);
  adresseEchange.sin_addr.s_addr=adresse.sin_addr.s_addr;
  //fin creation socket echange

  char ack[]="ACK";
  send=sendto(desc,ack,strlen(ack),0,(struct sockaddr *)&adresse,taille);
  printf("ACK envoyé, fermeture de desc\n");
  close(desc);//on n'en a plus besoin à ce niveau

  //fin 3 handshake
  // Connexion etablit


  // Envoi d'un fichier

  // Récupère l'extension du fichier pour obtenir le bon format en reception
  printf("Quelle est le nom du fichier ?\n");
  char nomfichier[100]="";
  scanf("%s",&nomfichier);
  printf("My file is : %s\n",nomfichier);
  char* extension=malloc(5*sizeof(char));
  extension=extFile(nomfichier);

  // On envoit le
  FILE * fichierSend;
  fichierSend=fopen(nomfichier,"rb+");
  if (!fichierSend) {
      perror("Erreur ouverture du fichier ou le fichier n'existe pas\n");
      exit(EXIT_FAILURE);
  }
  int tailleDeTrame=0;
  // Envoi le l'extension du fichier
  sendto(descEchange,extension,sizeof(extension),0,(struct sockaddr *)&adresseEchange,sizeof(adresseEchange));


  while(!feof(fichierSend)) {
    tailleDeTrame = fread(blanmsg,sizeof(char),sizeof(blanmsg),fichierSend);
    int envoieTrame = sendto(descEchange,blanmsg,sizeof(blanmsg),0,(struct sockaddr *)&adresseEchange,sizeof(adresseEchange));
    if (envoieTrame < 0) {
      printf("test erreur = %d\n",envoieTrame);
      perror("Erreur d'envoi !\n");
    }
    // Recoit ACK_NUMERO du serveur
    int ackReceive= recvfrom(descEchange,acquittement,sizeof(acquittement),0,(struct sockaddr *)&adresseEchange,&taille);
    printf("%s\n",acquittement);
  }
  char endFile[]="END";
  sendto(descEchange,endFile,sizeof(endFile),0,(struct sockaddr *)&adresseEchange,sizeof(adresseEchange));
  printf("Fichier envoyé\n");
  free(extension);
  fclose(fichierSend);
  // fin d'envoi de fichier


  // Envoi de texte
  int cont= 1;
  memset(blanmsg,0,RCVSIZE);
  while (cont){
    FD_SET(desc,&fs);
    printf("Saisie du message\n");
    fgets(msg, RCVSIZE, stdin);
    int send=sendto(descEchange,msg,strlen(msg),0,(struct sockaddr *)&adresseEchange,taille);
    printf("Message envoye %d\n",send-1);
    //int msgSize=recvfrom(desc, blanmsg, sizeof(blanmsg),0,(struct sockaddr *)&adresse,&taille);
    //printf("Reception reponse : ");
    //printf("%s\n",blanmsg);
    memset(blanmsg,0,RCVSIZE);
    if (strcmp(msg,"stop\n") == 0) {
      cont= 0;
    }
  }

close(descEchange);
return 0;
}
