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
int OctetL;
MYSQL* MysqlBase;

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);

  // Connexion à la base de donnée
  // TO DO

  
  MYSQL_RES  *resultat;
  MYSQL_ROW  Tuple;

  MysqlBase = mysql_init(NULL);

  if (mysql_real_connect(MysqlBase,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(ACCESBD) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  MESSAGE m, reponse;
  char requete[200];
  int TailleDuMessage ;

  while(1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO
    if((OctetL = read(fdRpipe, &m, sizeof(MESSAGE))) == -1 )
    {
        perror("(ACCESBD) ERREUR lors de la lecture du pipe");
        exit(1);
    }
    if(OctetL == 0)
    {
      mysql_close(MysqlBase);
      exit(EXIT_SUCCESS);
    }

    switch(m.requete)
    {
      case CONSULT :  // TO DO
      printf("coucou \n");
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      fprintf(stderr, "(ACCESBD) l'id du legume/fruit est %d", m.data1);
                      sprintf(requete,"select * from UNIX_FINAL where id = %d", m.data1);

                      if (mysql_query(MysqlBase, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysql-query");
                      }
                      if((resultat = mysql_store_result(MysqlBase)) == NULL)
                      {
                        fprintf (stderr,"Erreur de mysql store");
                      }

                      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        
                        reponse.requete = CONSULT ;
                        reponse.type = m.expediteur;
                        

                        reponse.expediteur = getpid();
                        
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                        strcpy(reponse.data4, Tuple[4]);
                        strcpy(reponse.data3, Tuple[3]);
                        reponse.data5= atof(Tuple[2]);


                        fprintf(stderr,"(ACCESBD %d) Requete CONSULT envoyé a %d\n",getpid(),reponse.type);
                        
                        TailleDuMessage = msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);


                        if(TailleDuMessage == -1)
                        {
                          perror("(ACCESBD) ERREUR de msgsnd");
                          msgctl(idQ,IPC_RMID,NULL);
                          exit(1);
                        }

                        fprintf(stderr, "(ACCESBD) Msgsnd envoie\n");
                        
                        
                      }
                      else
                      {
                        reponse.data1 = -1;
                        reponse.requete = CONSULT ;
                        reponse.type = m.expediteur;
                        reponse.expediteur = getpid();

                        fprintf(stderr,"(ACCESBD %d) Requete CONSULT envoyé a %d\n",getpid(),reponse.type);
                        
                        TailleDuMessage = msgsnd(idQ,&reponse,sizeof(MESSAGE)-sizeof(long),0);

                      }

                      // Preparation de la reponse

                      // Envoi de la reponse au bon caddie
                      break;

      case ACHAT :    // TO DO
                      int stock;
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      sprintf(requete,"select * from UNIX_FINAL where id = %d", m.data1);

                      if (mysql_query(MysqlBase, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysql-query");
                      }
                      if((resultat = mysql_store_result(MysqlBase)) == NULL)
                      {
                        fprintf (stderr,"Erreur de mysql store");
                      }
                      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                        strcpy(reponse.data4, Tuple[4]);
                        strcpy(reponse.data3, Tuple[3]);
                        reponse.data5= atof(Tuple[2]);

                        reponse.requete = ACHAT;
                        reponse.type = m.expediteur;

                        reponse.expediteur = getpid();

                        stock = atoi(reponse.data3);

                        if(stock >= atoi(m.data2))
                        {
                          stock -= atoi(m.data2);

                          sprintf(requete, "update UNIX_FINAL set stock = %d where id = %d",stock , m.data1);
                          if (mysql_query(MysqlBase, requete) != 0)
                          {
                            fprintf (stderr, "Erreur de Mysql-query");
                          }
                          else
                          {
                            strcpy(reponse.data3,m.data2);

                            if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                            {
                              perror("(ACCesBD) ERREUR lors de l'envoie du message au CADDIE");
                              exit(1);
                            }
                          }
                            
                        }
                        else
                        {   

                            char str[20];
                            sprintf(str, "0");
                            strcpy(reponse.data3, str);

                            if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                            {
                              perror("(ACCesBD) ERREUR lors de l'envoie du message au CADDIE");
                              exit(1);
                            }
                        }
                        
                      }


                      // Finalisation et envoi de la reponse
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      sprintf(requete,"select * from UNIX_FINAL where id = %d", m.data1);
                      fprintf(stderr, "l'id de l'alliment est %d\n", m.data1);
                      if (mysql_query(MysqlBase, requete) != 0)
                      {
                        fprintf (stderr, "Erreur de Mysql-query");
                      }
                      if((resultat = mysql_store_result(MysqlBase)) == NULL)
                      {
                        fprintf (stderr,"Erreur de mysql store");
                      }
                      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
                      {
                        reponse.data1 = atoi(Tuple[0]);
                        strcpy(reponse.data2, Tuple[1]);
                        strcpy(reponse.data4, Tuple[4]);
                        strcpy(reponse.data3, Tuple[3]);
                        reponse.data5= atof(Tuple[2]);

                        stock = atoi(reponse.data3);

                        stock += atoi(m.data2);

                        fprintf(stderr, "(ACCESBD) le stock doit redevenir %d de %d \n", stock, m.data1);
                        sprintf(requete, "update UNIX_FINAL set stock = %d where id = %d", stock, m.data1);
                        if (mysql_query(MysqlBase, requete) != 0)
                        {
                          fprintf (stderr, "Erreur de Mysql-query");
                        }
                        else
                        {
                          fprintf(stderr, "(ACCESBD) La base de donnée a été mise a jour");
                          if(m.expediteur != 5)
                          {
                            char str[20];
                            sprintf(str, "%d", stock);

                            strcpy(reponse.data3, str);

                            reponse.type = m.expediteur;
                            reponse.requete = CONSULT;
                            
                            if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                            {
                              perror("(ACCesBD) ERREUR lors de l'envoie du message au CADDIE");
                              exit(1);
                            }
                          }
                        }
                      }


                      
                      // Mise à jour du stock en BD
                      break;

    }


    
  }
}

