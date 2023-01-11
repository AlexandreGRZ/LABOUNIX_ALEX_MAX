#include "windowgerant.h"
#include "ui_windowgerant.h"
#include <iostream>
using namespace std;
#include <mysql.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "protocole.h"
#include <cstring>
#include <string>
#include "SemPham.h"

int idSem;

int idArticleSelectionne = -1;
MYSQL *connexion;
MYSQL_RES  *resultat;
MYSQL_ROW  Tuple;
char requete[200];
int idQ;

void MiseAJourDeLaBaseDeDonne();
WindowGerant::WindowGerant(QWidget *parent) : QMainWindow(parent),ui(new Ui::WindowGerant)
{
    ui->setupUi(this);

    // Configuration de la table du stock (ne pas modifer)
    ui->tableWidgetStock->setColumnCount(4);
    ui->tableWidgetStock->setRowCount(0);
    QStringList labelsTableStock;
    labelsTableStock << "Id" << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetStock->setHorizontalHeaderLabels(labelsTableStock);
    ui->tableWidgetStock->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetStock->horizontalHeader()->setVisible(true);
    ui->tableWidgetStock->horizontalHeader()->setDefaultSectionSize(120);
    ui->tableWidgetStock->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetStock->verticalHeader()->setVisible(false);
    ui->tableWidgetStock->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de la file de message
    // TO DO
    fprintf(stderr,"(GERANT %d) Recuperation de l'id de la file de messages\n",getpid());
    if ((idQ = msgget(CLE,0)) == -1)
    {
      perror("(GERANT) Erreur de msgget");
      exit(1);
    }
    // Récupération du sémaphore
    // TO DO
    if ((idSem = semget(CLE,0,0)) == -1)
    {
      perror("Erreur de semget");
      exit(1);
    }

    // Prise blocante du semaphore
    // TO DO

    sem_wait(idSem, false);
        

    

    // Connexion à la base de donnée
    connexion = mysql_init(NULL);
    fprintf(stderr,"(GERANT %d) Connexion à la BD\n",getpid());
    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
      fprintf(stderr,"(GERANT %d) Erreur de connexion à la base de données...\n",getpid());
      exit(1);  
    }

    // Recuperation des articles en BD
    // TO DO

    for(int i = 0; i < 22; i++)
    {
      sprintf(requete,"select * from UNIX_FINAL where id = %d", i);

      if (mysql_query(connexion, requete) != 0)
      {
        fprintf (stderr, "Erreur de Mysql-query");
      }
      if((resultat = mysql_store_result(connexion)) == NULL)
      {
        fprintf (stderr,"Erreur de mysql store");
      }

      if ((Tuple = mysql_fetch_row(resultat)) != NULL)
      {
         ajouteArticleTablePanier(atoi(Tuple[0]), Tuple[1], atof(Tuple[2]), atoi(Tuple[3]));
      }
    }
    
}

WindowGerant::~WindowGerant()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du stock (ne pas modifier) //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::ajouteArticleTablePanier(int id,const char* article,float prix,int quantite)
{
    char Id[20],Prix[20],Quantite[20];

    sprintf(Id,"%d",id);
    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetStock->rowCount();
    nbLignes++;
    ui->tableWidgetStock->setRowCount(nbLignes);
    ui->tableWidgetStock->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Id);
    ui->tableWidgetStock->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetStock->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetStock->setItem(nbLignes-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetStock->setItem(nbLignes-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::videTableStock()
{
    ui->tableWidgetStock->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetStock->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_tableWidgetStock_cellClicked(int row, int column)
{
    //cerr << "ligne=" << row << " colonne=" << column << endl;
    ui->lineEditIntitule->setText(ui->tableWidgetStock->item(row,1)->text());
    ui->lineEditPrix->setText(ui->tableWidgetStock->item(row,2)->text());
    ui->lineEditStock->setText(ui->tableWidgetStock->item(row,3)->text());
    idArticleSelectionne = atoi(ui->tableWidgetStock->item(row,0)->text().toStdString().c_str());
    //cerr << "id = " << idArticleSelectionne << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float WindowGerant::getPrix()
{
    return atof(ui->lineEditPrix->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getStock()
{
    return atoi(ui->lineEditStock->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowGerant::getPublicite()
{
  strcpy(publicite,ui->lineEditPublicite->text().toStdString().c_str());
  return publicite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::closeEvent(QCloseEvent *event)
{
  fprintf(stderr,"(GERANT %d) Clic sur croix de la fenetre\n",getpid());
  // TO DO
  // Deconnexion BD
  mysql_close(connexion);

  // Liberation du semaphore
  // TO DO
  sem_signal(idSem);

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonPublicite_clicked()
{
  fprintf(stderr,"(GERANT %d) Clic sur bouton Mettre a jour\n",getpid());
  // TO DO (étape 7)
  // Envoi d'une requete NEW_PUB au serveur

  const char *pPub;
  MESSAGE m;
  pPub = getPublicite();

  m.type = 1;
  m.expediteur = getpid();
  m.requete = NEW_PUB;
  strcpy(m.data4, pPub);

  if((msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
  {
    perror("(SERVEUR) ERREUR dans l'envoie de la réponse du login du client");
    msgctl(idQ, IPC_RMID, NULL);
    exit(1);
  }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonModifier_clicked()
{
  fprintf(stderr,"(GERANT %d) Clic sur bouton Modifier\n",getpid());
  // TO DO
  //cerr << "Prix  : --"  << getPrix() << "--" << endl;
  //cerr << "Stock : --"  << getStock() << "--" << endl;

  

  int stock = getStock();

  char Prix[20];
  sprintf(Prix,"%f",getPrix());
  string tmp(Prix);
  size_t x = tmp.find(",");
  if (x != string::npos) tmp.replace(x,1,".");

  strcpy(Prix, tmp.c_str());

  fprintf(stderr,"(GERANT %d) Modification en base de données pour id=%d\n",getpid(),idArticleSelectionne);
  fprintf(stderr, "(GERRANT) l'id est %d et le sotck est %d\n", idArticleSelectionne, stock);
  sprintf(requete, "update UNIX_FINAL set stock = %d where id = %d", stock, idArticleSelectionne);
  if (mysql_query(connexion, requete) != 0)
  {
    fprintf (stderr, "Erreur de Mysql-query");
  }
  else
  {
    fprintf(stderr, "(GERRANT) l'id est %d et le prix est %s\n", idArticleSelectionne, Prix);
    sprintf(requete, "update UNIX_FINAL set prix = %s where id = %d", Prix, idArticleSelectionne);
    if (mysql_query(connexion, requete) != 0)
    {
      fprintf (stderr, "Erreur de Mysql-query");
    }
    else
    {
        // Mise a jour table BD
        // TO DO
      videTableStock();

      fprintf(stderr, "(GERRANT) AVANT L4AFFICHAGE \n");
      for(int i = 0; i < 22; i++)
      {
        sprintf(requete,"select * from UNIX_FINAL where id = %d", i);

        if (mysql_query(connexion, requete) != 0)
        {
          fprintf (stderr, "Erreur de Mysql-query");
        }
        if((resultat = mysql_store_result(connexion)) == NULL)
        {
          fprintf (stderr,"Erreur de mysql store");
        }

        if ((Tuple = mysql_fetch_row(resultat)) != NULL)
        {
          ajouteArticleTablePanier(atoi(Tuple[0]), Tuple[1], atof(Tuple[2]), atoi(Tuple[3]));
        }
      }

    }
  }



}


void MiseAJourDeLaBaseDeDonne()
{
    
}
