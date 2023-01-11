#include "FicheClient.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h> 
#include <stdio.h>



int estPresent(const char* nom)
{
  int f ;
  CLIENT client ;
  int cd =1;
  int i =1;

  f = open(FICHIER_CLIENTS,O_RDONLY);
    lseek(f, 0 ,SEEK_CUR);

  if (f==-1)
  {
    return -1 ;
  }
  else
  {
    do{

        if (read(f , &client, sizeof(CLIENT) )!=0)
        {
            printf("le client present ? est %s",client.nom);
          if (strcmp (client.nom , nom )==0)
          {
            close (f);

            return i;
          } 
          else i++ ;
        }
        else
        {
          cd=0 ;
        }
    }while (cd!=0) ; 

  }
  close (f);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  // TO DO
  int somme=0 ; 

  for (int i =0 ; i<20 && motDePasse[i]!= '\0';i++)
  {
  somme += (i+1) * motDePasse[i];
  }


  return somme%97;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteClient(const char* nom, const char* motDePasse)
{
  // TO DO

  int f ;
  CLIENT client ;
  
  
        f = open(FICHIER_CLIENTS,O_RDWR );
        if (f==-1) 
        {
          f = open(FICHIER_CLIENTS,O_WRONLY |O_CREAT |O_APPEND,0600);
          strcpy(client.nom, nom) ; 
          client.hash = hash(motDePasse); 
          lseek(f, 0 ,SEEK_END);
          write(f, &client , sizeof(CLIENT));
          close (f);
        }
        else 
        {
          strcpy(client.nom, nom) ; 
          client.hash = hash(motDePasse); 
          lseek(f, 0 ,SEEK_END);
          write(f, &client , sizeof(CLIENT));
          close (f);
        
      
      }
  
  

}

////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  // TO DO
   int f ;
  CLIENT client ;

  f = open(FICHIER_CLIENTS,O_RDONLY);

  if (f==-1)return -1 ;

  lseek (f, (pos-1)*sizeof(CLIENT) , SEEK_SET);
  read(f , &client, sizeof(CLIENT) );

  if (hash(motDePasse)==client.hash)
  {
    close(f);
    return 1 ;
  }
  else 
  {
    close(f);
    return 0 ;
  }

}

////////////////////////////////////////////////////////////////////////////////////
int listeClients(CLIENT *vecteur)
{
  // TO DO
  int nbclient=0;
  int f ;
  

  
  if ((f = open(FICHIER_CLIENTS,O_RDONLY))==-1)
  {
    return 0 ;
  }
  else 
  {
      while (read(f , vecteur, sizeof(CLIENT) )!=0) 
    {
      vecteur++;
      nbclient++;
    }
    
    close(f);
    return nbclient;
  }

}
