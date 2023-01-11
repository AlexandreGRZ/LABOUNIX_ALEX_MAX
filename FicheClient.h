#ifndef FICHE_CLIENT_H
#define FICHE_CLIENT_H

#define FICHIER_CLIENTS "clients.dat"

typedef struct
{
  char  nom[20];
  int   hash;
} CLIENT;

int estPresent(const char* nom);

int hash(const char* motDePasse);

void ajouteClient(const char* nom, const char* motDePasse);

int verifieMotDePasse(int pos, const char* motDePasse);

int listeClients(CLIENT *vecteur);

#endif