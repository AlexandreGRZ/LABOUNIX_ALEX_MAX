#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
char *pShm;
void handlerSIGUSR1(int sig);
int fd;
MESSAGE msg;

int main()
{
  // Armement des signaux
  // TO DO
  struct sigaction A;
  A.sa_handler = handlerSIGUSR1;
  A.sa_flags = 0;
  sigaction(SIGUSR1,&A,NULL);

  // Masquage des signaux
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask,SIGUSR1);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant de la mémoire partagée
  if((idShm = shmget(CLE, 0, 0)) == -1 )
  {
    perror("(PUBLICITE) Erreur de récupération de mémoire ppartagé");
    exit(1);
  }
  
  
  // Attachement à la mémoire partagée
  if((pShm = (char*)shmat(idShm, NULL, 0)) == (char*) - 1)
  {
    perror("(PUBLICITE) Erreur lors de l' attachement a la mémoire partagé ");
    exit(1);
  }


  // Mise en place de la publicité en mémoire partagée
  char pub[51], tmpchar;
  strcpy(pub,"Bienvenue sur le site du Maraicher en ligne !");

  for (int i=0 ; i<=50 ; i++) pShm[i] = ' ';
  pShm[51] = '\0';
  int indDebut = 25 - strlen(pub)/2;
  for (int i=0 ; i<strlen(pub) ; i++) pShm[indDebut + i] = pub[i];

  while(1)
  {
    // Envoi d'une requete UPDATE_PUB au serveur

    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = UPDATE_PUB;

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("(PUBLICITE) Erreur d'envoie de message au serveur ");
      exit(1);
    }

    sleep(1); 

    // Decallage vers la gauche
    tmpchar =pShm[0];
    for (int i=0;i<50;i++)
    {
      pShm[i]=pShm[i+1];
    }
    pShm[50]=tmpchar;
  }
}

void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());
  printf("coucou\n");
  // Lecture message NEW_PUB
  char pub[51], tmpchar;
  MESSAGE m;
  printf("coucou\n");
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(SERVEUR) Erreur de msgrcv");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }
  printf("coucou\n");
  strcpy(pub, m.data4);
  printf("coucou\n");
  for (int i=0 ; i<=50 ; i++) pShm[i] = ' ';
  printf("coucou\n");
  pShm[51] = '\0';
  printf("coucou\n");
  int indDebut = 25 - strlen(pub)/2;
  printf("coucou\n");
  for (int i=0 ; i<strlen(pub) ; i++) pShm[indDebut + i] = pub[i];
  printf("coucou\n");



  // Mise en place de la publicité en mémoire partagée
}
