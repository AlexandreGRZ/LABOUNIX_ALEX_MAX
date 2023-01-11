#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include "protocole.h" // contient la cle et la structure d'un message

#include "SemPham.h"
#include "FicheClient.h"
int idQ,idShm,idSem, boolen, idfilspub, idfilscaddie, idfilsAccesBd, jmp;
int fdPipe[2];
TAB_CONNEXIONS *tab;
sigjmp_buf contexte;

void afficheTab();

void handlerSIGINT(int sig);
void handlerSIGCHLD(int sig);

int main()
{
  // Armement des signaux
  // TO DO
  struct sigaction A;
  A.sa_handler = handlerSIGINT;
  A.sa_flags = 0;
  sigaction(SIGINT,&A,NULL);

  A.sa_handler = handlerSIGCHLD;
  A.sa_flags = 0;
  sigaction(SIGCHLD,&A,NULL);
  // Creation des ressources
  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0666)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  // TO BE CONTINUED

  // Creation du pipe
  // TO DO

  if((pipe(fdPipe)) == -1)
  {
    perror("(SERVEUR) Erreur lors de l'ouverture/creation du pipe");
    exit(1);
  }
  //creation du sémaphore 
  if ((idSem = semget(CLE,2,IPC_CREAT | IPC_EXCL | 0666)) == -1)
  {
    perror("Erreur de semget");
    exit(1);
  }

  if (semctl(idSem,0,SETVAL,1) == -1)
  {
  perror("Erreur de semctl");
  exit(1);
  }
  
  //création du processus de Connexion bd

  idfilsAccesBd = fork();

  if(idfilsAccesBd == 0)
  {
    close(fdPipe[1]);
    char str[10];
    sprintf(str, "%d", fdPipe[0]);
    if(execl("./AccesBD", "AccesBD", str, NULL))
    {
      perror("(SERVEUR) ERREUR dans la création du processus de AccesBD");
      exit(1);
    }
  }

  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 
  int i;
  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  // TO DO
  if((idShm = shmget(CLE, 52, IPC_CREAT | IPC_EXCL | 0666)) == -1)
  {
    perror("(SERVEUR) ERREUR lors de la création de la mémoire partagé");
    shmctl(idQ, IPC_RMID, NULL);
    exit(1);
  }

  idfilspub = fork();

  if(idfilspub == 0)
  {
    if(execl("./Publicite", "Publicité", NULL) == -1)
    {
      perror("erreur lors de la pub");
    }
  }
  else
  {
    tab->pidPublicite = idfilspub;
  }

  // Creation du processus AccesBD (étape 4)
  // TO DO

  MESSAGE m;
  MESSAGE reponse;

  while(1)
  {
    jmp = sigsetjmp(contexte, 1);

  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    switch(m.requete)
    {
      case CONNECT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);
                      for(i = 0, boolen = 0; i < 6 && boolen == 0; i++)
                      {
                        if(tab -> connexions[i].pidFenetre == 0)
                        {
                          tab -> connexions[i].pidFenetre = m.expediteur;
                          boolen = 1;
                        }
                      }
                      break;

      case DECONNECT : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);
                      for(i = 0, boolen = 0; i < 6 && boolen == 0; i++)
                      {
                        if(tab -> connexions[i].pidFenetre = m.expediteur)
                        {
                          tab -> connexions[i].pidFenetre = 0;
                          boolen = 1;
                        }
                      }
                      break;
      case LOGIN :    // TO DO


                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          fprintf(stderr,"(SERVEUR %d) Requete BUSY reçue de %d\n",getpid(),m.expediteur);
                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);
                        if (m.data1==1)
                        {
                          if (estPresent(m.data2)>0)
                          {
                            strcpy(m.data4,"«Client  déjà  existant!»");
                            m.data1=0;
                          }
                          else 
                          {
                            strcpy(m.data4,"«Nouveau client créé:bienvenue!»");

                              for (int i=0 ; i<6 ; i++)
                              {
                                if(tab->connexions[i].pidFenetre == m.expediteur)
                                  {
                                    strcpy(tab->connexions[i].nom,m.data2);
                                    i=6;
                                  }
                              } 
                              ajouteClient(m.data2 ,m.data3);
                              m.data1=1;
                            
                          }
                        }
                        else 
                        {
                          if (estPresent(m.data2)>0)
                          {
                              if (verifieMotDePasse(estPresent(m.data2), m.data3 )==1)
                              {
                                strcpy(m.data4,"«Re-bonjour cher client!»");
                                  for (int i=0 ; i<6 ; i++)
                                  {
                                    if(tab->connexions[i].pidFenetre == m.expediteur)
                                      {
                                        strcpy(tab->connexions[i].nom,m.data2);
                                        i=6;
                                      }
                                  } 
                                  m.data1=1;
                              }
                              else 
                              {
                                  strcpy(m.data4,"«Mot  de passe incorrect...»");
                                  m.data1=0;
                              }
                          }
                          else 
                          {
                            strcpy(m.data4,"«Client Inconnu ...»");
                            m.data1=0;
                          }

                        }
                        m.type = m.expediteur;
                        fprintf(stderr, "(SERVEUR) evoie du message LOGIN au Client");
                        if((msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                        {
                          perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                          msgctl(idQ, IPC_RMID, NULL);
                          exit(1);
                        }

                        printf("Message envoiyer\n");
                        //Envoie du signal Siguser 1 au client
                        
                        if(m.data1 == 1)
                        { 
                          for(int i=0;i<6 && m.data1==1;i++)
                            {
                              if(tab->connexions[i].pidFenetre == m.expediteur)
                              {
                                if((idfilscaddie = fork()) == 0)  
                                {
                                  //code executé par le fils (caddie)
                                  
                                  close(fdPipe[0]);
                                  char str[10];
                                  sprintf(str ,"%d", fdPipe[1]);
                                  if(execlp("./Caddie", "Caddie", str, NULL) == -1)
                                  {
                                    perror("Erreur d execution de Caddie\n");
                                    exit(1);
                                  }
                                }
                                else
                                {
                                  tab->connexions[i].pidCaddie = idfilscaddie;
                                }
                                
                              }
                            }
                        }

                        

                        reponse.expediteur = m.type;
                        reponse.requete = LOGIN;
                        for(int i = 0; i < 6 ; i++)
                        {
                          if(tab -> connexions[i].pidFenetre == reponse.expediteur)
                          {

                            reponse.type = tab -> connexions[i].pidCaddie;

                            i = 6;
                          }
                        }
                        if((msgsnd(idQ,&reponse,sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                        {
                          perror("(SERVEUR)ERREUR lors de l'envoie du pid du client au caddie");
                          exit(1);
                        }


                        kill(m.type, SIGUSR1);

                        sem_signal(idSem);

                      }

                      
                      break; 


      case LOGOUT :   // TO DO
                      int i;
                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          fprintf(stderr,"(SERVEUR %d) Requete BUSY reçue de %d\n",getpid(),m.expediteur);
                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                        printf("avant la boucle \n");
                        for (i = 0 ; i < 6 ; i++)
                        {
                          int indice = tab->connexions[i].pidFenetre;
                          if(indice == m.expediteur)
                          {

                            strcpy(tab->connexions[i].nom,"");
                            //suppression du caddie et de son processus 
                            reponse.type = tab->connexions[i].pidCaddie;
                            reponse.requete = LOGOUT;
                            reponse.expediteur=m.expediteur;
                            if (reponse.type!=0)
                            {
                              if (msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                  perror("Erreur de msgsnd ");
                                  msgctl(idQ,IPC_RMID,NULL);
                                  exit(1);
                              }
                              

                                i=6;
                            }
                          }
                        } 

                        sem_signal(idSem);
                        
                      }
                      
                      
                      
                      break;

      case UPDATE_PUB :  // TO DO

                      for (i = 0 ; i < 6 ; i++)
                      {
                        int indice = tab->connexions[i].pidFenetre;
                        if(indice != 0)
                        {
                            kill(indice, SIGUSR2); 
                        }
                      } 
                      break;

      case CONSULT :  // TO DO
                      
                      if((sem_wait(idSem, true)) == -1)
                      {   
                          fprintf(stderr,"(SERVEUR %d) Requete BUSY reçue de %d\n",getpid(),reponse.expediteur);
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                        for (i = 0 ; i < 6 ; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            reponse.type = tab->connexions[i].pidCaddie;
                            reponse.expediteur = m.expediteur;
                            reponse.requete = m.requete;
                            reponse.data1 = m.data1;

                            if((msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0)) == -1)
                            {
                                perror("Erreur de msgsnd ");
                                msgctl(idQ,IPC_RMID,NULL);
                                exit(1);
                            }

                            i = 6;
                          }
                        } 
                        sem_signal(idSem);
                      }
                      
                      
                      break;

      case ACHAT :    // TO DO
                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                        for (i = 0 ; i < 6 ; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;

                            i = 6;
                          }
                        } 

                        if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1)
                        {
                          perror("erreur d'envoi");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }
                        sem_signal(idSem);
                      }
                      
                      break;

      case CADDIE :   // TO DO

                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                        for (i = 0 ; i < 6 ; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;

                            i = 6;
                          }
                        } 

                        if(msgsnd(idQ,&m,sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                          perror("erreur d'envoi");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }
                        sem_signal(idSem);

                      }


                      
                      break;

      case CANCEL :   // TO DO
                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                        fprintf(stderr, "(SERVERUR) l'id du fruit est %d\n", m.data1);
                        for (i = 0 ; i < 6 ; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;

                            i = 6;
                          }
                        } 

                        if(msgsnd(idQ,&m,sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                          perror("erreur d'envoi");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }
                        sem_signal(idSem);
                      }
                      
                      break;

      case CANCEL_ALL : // TO DO

                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                        fprintf(stderr, "(SERVERUR) l'id du fruit est %d\n", m.data1);
                        for (i = 0 ; i < 6 ; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;

                            i = 6;
                          }
                        } 

                        if(msgsnd(idQ,&m,sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                          perror("erreur d'envoi");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }

                        sem_signal(idSem);

                      }
                      
                      break;

      case PAYER : // TO DO
                      if((sem_wait(idSem, true)) == -1)
                      {
                          reponse.type = m.expediteur;
                          reponse.expediteur = 1;
                          reponse.requete = BUSY;

                          if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                          {
                            perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                            msgctl(idQ, IPC_RMID, NULL);
                            exit(1);
                          }
                          kill(reponse.type, SIGUSR1);
                      }
                      else
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                        for (i = 0 ; i < 6 ; i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;

                            i = 6;
                          }
                        } 

                        if(msgsnd(idQ,&m,sizeof(MESSAGE) - sizeof(long), 0) == -1)
                        {
                          perror("erreur d'envoi");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }

                        sem_signal(idSem);
                      }
                      
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);

                      m.type = tab->pidPublicite;

                      if((msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                      {
                        perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
                        msgctl(idQ, IPC_RMID, NULL);
                        exit(1);
                      }

                      kill(tab->pidPublicite, SIGUSR1);
                      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : %d\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : %d\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
    tab->connexions[i].nom,
    tab->connexions[i].pidCaddie);
    fprintf(stderr,"\n");
}

void handlerSIGINT(int sig)
{
  shmctl(idShm,IPC_RMID,NULL);
  msgctl(idQ,IPC_RMID,NULL);

  if (close(fdPipe[0]) == -1)
  {
    perror("Erreur fermeture sortie du pipe");
    exit(1);
  }
  if (close(fdPipe[1]) == -1)
  {
    perror("Erreur fermeture entree du pipe");
    exit(1);
  }

  if (semctl(idSem,0,IPC_RMID, NULL) == -1)
  {
    perror("Erreur de semctl");
    exit(1);
  }
  

  exit(1);
  
}

void handlerSIGCHLD(int sig2)
{
  fprintf(stderr,"(SERVEUR %d) reception de sigchld\n",getpid());
  idfilscaddie = wait(NULL);

  for (int i = 0; i < 6; i++)
  {
    if (tab->connexions[i].pidCaddie == idfilscaddie)
    {
      tab->connexions[i].pidCaddie = 0;
      strcpy(tab->connexions[i].nom, "\0");
      i = 6;
    }
  }
   siglongjmp(contexte, 406);
}