// Recupérer l'extension d'un fichier
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fonctionClient.h"

int myStrlen(char chaine[]) {
    int nbChar=0;
    while (chaine[nbChar]!='\0') {
      printf("%c\n",chaine[nbChar]);
      nbChar++;
    }
    return nbChar;
}

char* extFile(char nomfichier[]) {
  char *extension=NULL;
  extension=malloc(5*sizeof(char));
  int indice = 0;
  while (nomfichier[indice]!='.')  indice++;
  indice++;
  int i;
  for (i = indice;i<=strlen(nomfichier);i++) {
    extension[i-indice]=nomfichier[i];
  }
  return (extension);
}
