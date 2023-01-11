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
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

ARTICLE articles[10];
int nbArticles = 0;

int fdWpipe;
int pidClient;

MYSQL* connexion;

int stock, OctetW, indice;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{ 

  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO
  struct sigaction A;
  A.sa_handler = handlerSIGALRM;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;
  if (sigaction(SIGALRM,&A,NULL) == -1)
  {
  perror("Erreur de sigaction");
  exit(1);
  }

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE m;
  MESSAGE reponse;
  
  char requete[200];
  char newUser[20];
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;
  int boolen = 0;
  int i = 0, time = 0;

  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);

  while(1)
  { 
    printf("Dans la boucle \n");
    alarm(60);
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv");
      exit(1);
    }
    else
    { 
      time = alarm(0);
      switch(m.requete)
      {
        case LOGIN :    // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                        pidClient = m.expediteur;
                        break;

        case LOGOUT :   // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);

                        reponse.requete = CANCEL;
                        reponse.expediteur = 5;
                        for(i = 0; i < nbArticles; i++)
                        {
                            
                            fprintf(stderr, "(CADDIE) L'id du fruit est %d \n", articles[i].id);
                            reponse.data1 = articles[i].id;

                            char str[20];

                            sprintf(str, "%d \n", articles[i].stock);
                            strcpy(reponse.data2, str);
                            
                            


                            if(( OctetW = write(fdWpipe, &reponse, sizeof(MESSAGE))) != sizeof(MESSAGE))
                            {
                              perror("(CADDIE) ERREUR lors de l'ecriture dans le pipe");
                              exit(1);
                            }
                        }
                        // On vide le panier
                        for(i = 0; i < nbArticles; i++)
                        {
                          articles[i].id = -1;
                        }
                        
                        nbArticles = 0;



                        exit(1);
                        break;

        case CONSULT :  // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                        
                        reponse.expediteur = getpid();
                        reponse.type = pidClient;
                        reponse.requete = CONSULT;
                        reponse.data1 = m.data1;

                        if(( OctetW = write(fdWpipe, &reponse, sizeof(MESSAGE))) != sizeof(MESSAGE))
                        {
                          perror("(CADDIE) ERREUR lors de l'ecriture dans le pipe");
                          exit(1);
                        }
                        printf("%d", &OctetW);
                        if((msgrcv(idQ, &m, sizeof(MESSAGE) - sizeof(long), getpid(), 0)) == -1)
                        {
                          perror("(CADDIE) ERREUR lors de la reception du message de ACCESBD");
                          exit(1);
                        }
                        else
                        { 
                          if(m.data1 != -1)
                          {
                            m.type = pidClient;
                            m.expediteur = getpid();
                            m.requete = CONSULT;

                            fprintf(stderr,"(CADDIE %d) Requete CONSULT envoyé a %d\n",getpid(),m.type);
                            if ((stock = atoi(m.data3)) > 0);
                            {
                              if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                              {
                                perror("(Caddie) Erreur de msgsnd");
                                msgctl(idQ,IPC_RMID,NULL);
                                exit(1);
                              }
                              kill(pidClient, SIGUSR1);
                            }
                          }
                          else
                          {

                          }

                        }
                        
                        break;

        case ACHAT :    // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                        // on transfert la requete à AccesBD
                        m.expediteur = getpid();

                        if(( OctetW = write(fdWpipe, &m, sizeof(MESSAGE))) != sizeof(MESSAGE))
                        {
                          perror("(CADDIE) ERREUR lors de l'ecriture dans le pipe");
                          exit(1);
                        }
                        printf("%d", &OctetW);
                        if((msgrcv(idQ, &m, sizeof(MESSAGE) - sizeof(long), getpid(), 0)) == -1)
                        {
                          perror("(CADDIE) ERREUR lors de la reception du message de ACCESBD");
                          exit(1);
                        }
                        else
                        { 

                          if(atoi(m.data3) != 0)
                          {
                            fprintf(stderr, "(CADDIE) l'id de l'aliment acheté est %d\n", m.data1);
                            articles[nbArticles].id = m.data1;
                            strcpy(articles[nbArticles].intitule, m.data2);
                            articles[nbArticles].prix = m.data5;
                            articles[nbArticles].stock = atoi(m.data3);
                            strcpy(articles[nbArticles].image, m.data4);
                            fprintf(stderr, "(CADDIE) l'id de l'aliment acheté est %d\n", articles[nbArticles].id);
                            nbArticles += 1;

                            m.type = pidClient;
                            m.expediteur = getpid();
                            m.requete = ACHAT;


                            if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                            {
                              perror("(Caddie) Erreur de msgsnd");
                              msgctl(idQ,IPC_RMID,NULL);
                              exit(1);
                            }
                            kill(pidClient, SIGUSR1);
                          }
                          else
                          {
                            m.type = pidClient;
                            m.expediteur = getpid();
                            m.requete = ACHAT;


                            if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                            {
                              perror("(Caddie) Erreur de msgsnd");
                              msgctl(idQ,IPC_RMID,NULL);
                              exit(1);
                            }
                            kill(pidClient, SIGUSR1);
                          }

                          
                        }

                        
                        // on attend la réponse venant de AccesBD
                          
                        // Envoi de la reponse au client

                        break;

        case CADDIE :   // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);

                        m.type = pidClient;
                        m.expediteur = getpid();
                        m.requete = CADDIE;

                        for(i = 0; i < nbArticles; i++)
                        {
                          
                          m.data1 = articles[i].id;

                          strcpy(m.data2, articles[i].intitule);
                          
                          m.data5 = articles[i].prix;

                          char str[20];
                          sprintf(str, "%d", articles[i].stock);

                          strcpy(m.data3, str);

                          strcpy(m.data4, articles[i].image);

                          

                          if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
                          {
                            perror("(Caddie) Erreur de msgsnd");
                            msgctl(idQ,IPC_RMID,NULL);
                            exit(1);
                          }
                          kill(pidClient, SIGUSR1);


                        }
                        break;

        case CANCEL :   // TO DO
                        indice = m.data1;
                        fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                        
                        fprintf(stderr, "(CADDIE) L'id du fruit est %d \n", m.data1);
                        // on transmet la requete à AccesBD
                        reponse.expediteur = getpid();
                        reponse.requete = CANCEL;
                        for(i = 0; i <= indice; i++)
                        {
                          if(i == indice)
                          { 
                            fprintf(stderr, "(CADDIE) L'id du fruit est %d \n", articles[i].id);
                            reponse.data1 = articles[i].id;

                            char str[20];

                            sprintf(str, "%d \n", articles[i].stock);
                            strcpy(reponse.data2, str);
                          }
                        }
                        fprintf(stderr, "(CADDIE) L'id du fruit est %d \n", reponse.data1);
                        fprintf(stderr, "(CADDIE)le stock est de %d \n", atoi(reponse.data2));

                        

                        if(( OctetW = write(fdWpipe, &reponse, sizeof(MESSAGE))) != sizeof(MESSAGE))
                        {
                          perror("(CADDIE) ERREUR lors de l'ecriture dans le pipe");
                          exit(1);
                        }
                        
                      
                        // Suppression de l'aricle du panier
                        boolen = 0;
                        i = 0;
                        if(i == (nbArticles - 1))
                        {
                          nbArticles = 0;
                        }
                        else
                        {
                          for(i = 0; i <= (nbArticles - 1); i++)
                          { 
                            if(boolen == 1)
                            {
                              articles[i].id = articles[i + 1].id;
                              strcpy(articles[i].intitule, articles[i + 1].intitule);
                              articles[i].prix = articles[i + 1].prix;
                              articles[i].stock = articles[i + 1].stock;
                              strcpy(articles[i].image ,articles[i + 1].image);
                            }

                            if(boolen == 0)
                            {
                              if(articles[i].id == reponse.data1)
                              {
                                articles[i].id = articles[i + 1].id;
                                strcpy(articles[i].intitule, articles[i + 1].intitule);
                                articles[i].prix = articles[i + 1].prix;
                                articles[i].stock = articles[i + 1].stock;
                                strcpy(articles[i].image ,articles[i + 1].image);
                                
                                boolen = 1;
                              }
                            }

                          }
                          nbArticles -= 1;
                        }
                      
                        break;

        case CANCEL_ALL : // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                        // On envoie a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier
                        reponse.requete = CANCEL;
                        reponse.expediteur = 5;
                        for(i = 0; i < nbArticles; i++)
                        {
                            
                            fprintf(stderr, "(CADDIE) L'id du fruit est %d \n", articles[i].id);
                            reponse.data1 = articles[i].id;

                            char str[20];

                            sprintf(str, "%d \n", articles[i].stock);
                            strcpy(reponse.data2, str);
                            
                            


                            if(( OctetW = write(fdWpipe, &reponse, sizeof(MESSAGE))) != sizeof(MESSAGE))
                            {
                              perror("(CADDIE) ERREUR lors de l'ecriture dans le pipe");
                              exit(1);
                            }
                        }
                        // On vide le panier
                        for(i = 0; i < nbArticles; i++)
                        {
                          articles[i].id = -1;
                        }
                        
                        nbArticles = 0;

                        
                        break;

        case PAYER :    // TO DO
                        fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                        // On vide le panier
                        nbArticles = 0;
                        break;
      }
    }

    
  }
}

void handlerSIGALRM(int sig)
{ 
  MESSAGE reponse;
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier
  reponse.requete = CANCEL;
  reponse.expediteur = 5;
  for(int i = 0; i < nbArticles; i++)
  {
      
      fprintf(stderr, "(CADDIE) L'id du fruit est %d \n", articles[i].id);
      reponse.data1 = articles[i].id;

      char str[20];

      sprintf(str, "%d \n", articles[i].stock);
      strcpy(reponse.data2, str);
      
      


      if(( OctetW = write(fdWpipe, &reponse, sizeof(MESSAGE))) != sizeof(MESSAGE))
      {
        perror("(CADDIE) ERREUR lors de l'ecriture dans le pipe");
        exit(1);
      }
  }
  // On vide le panier
  for(int i = 0; i < nbArticles; i++)
  {
    articles[i].id = -1;
  }
  
  nbArticles = 0;

  // Envoi d'un Time Out au client (s'il existe toujours)

  reponse.expediteur = getpid();
  reponse.type = pidClient;
  reponse.requete = TIME_OUT;

  if(msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0) == -1)
  {
    perror("(Caddie) Erreur de msgsnd");
    msgctl(idQ,IPC_RMID,NULL);
    exit(1);
  }
  fprintf(stderr, "(CADDIE) TIME_OUT envoyer au client %d", pidClient);
  kill(pidClient, SIGUSR1);

  exit(0);
}