#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string>
using namespace std;

#include "protocole.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

extern WindowClient *w;

MESSAGE msg;

int idQ, idShm;
bool logged;
char* pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;

void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);

#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de l'identifiant de la file de messages
    //fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    // TO DO
    if((idQ = msgget(CLE, 0)) == -1)
    {
      perror("Erreur de création de file");
      exit(1);
    }

    // Recuperation de l'identifiant de la mémoire partagée
    //fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la mémoire partagée\n",getpid());
    // TO DO
    if((idShm = shmget(CLE, 0, 0)) == -1 )
    {
      perror("(CLIENT) Erreur de récupération de mémoire partagé");
      exit(1);
    }
    // Attachement à la mémoire partagée
    // TO DO
    if((pShm = (char*)shmat(idShm, NULL, 0)) == (char*) - 1)
    {
      perror("(CLIENT) Erreur lors de l' attachement a la mémoire partagé ");
      exit(1);
    }
    // Armement des signaux
    // TO DO
    struct sigaction A;
    A.sa_handler = handlerSIGUSR1;
    A.sa_flags = 0;
    sigaction(SIGUSR1,&A,NULL);

    struct sigaction B;
    B.sa_handler = handlerSIGUSR2;
    B.sa_flags = 0;
    sigaction(SIGUSR2,&B,NULL);

    // Envoi d'une requete de connexion au serveur
    // TO DO

   

    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = CONNECT;

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("Erreur lors de l'envoie du Message CONNECT au serveur ");
      msgctl(idQ, IPC_RMID, NULL);
      exit(1);
    }

    // Exemples à supprimer
  
}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  // TO DO (étape 1)
  // Envoi d'une requete DECONNECT au serveur

  msg.type = 1;
  msg.expediteur = getpid();
  


  // envoi d'un logout si logged

    if (logged==1)
    {
      msg.requete= LOGOUT ;
        

      if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
        perror("erreur d'envoi");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }
    }
  msg.requete = DECONNECT;
  // Envoi d'une requete de deconnexion au serveur
    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long),0)) == -1)
    {
      perror("Erreur d'envoie du message DECONNECT au serveur");
      msgctl(idQ, IPC_RMID, NULL);
      exit(1);
    }

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
    // Envoi d'une requete de login au serveur
    // TO DO
    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = LOGIN;
    msg.data1 = w -> isNouveauClientChecked();
    strcpy(msg.data2,getNom() );
    strcpy(msg.data3,getMotDePasse());

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("ERREUR d'envoie du message LOGIN au serveur");
      msgctl(idQ, IPC_RMID, NULL);
      exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
    // Envoi d'une requete CANCEL_ALL au serveur (au cas où le panier n'est pas vide)
    // TO DO

    // Envoi d'une requete de logout au serveur
    // TO DO
      msg.type=1 ;
      msg.expediteur=getpid();
      msg.requete= LOGOUT ;
      

      if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
      {
        perror("erreur d'envoi");
        msgctl(idQ,IPC_RMID,NULL);
        exit(1);
      }
      logoutOK();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = CONSULT;
    msg.data1 = articleEnCours.id + 1;
    
    printf("Avant msgsnd \n");
    if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
    {
      perror("erreur d'envoi");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
    // TO DO (étape 3)
    // Envoi d'une requete CONSULT au serveur
    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = CONSULT;
    msg.data1 = articleEnCours.id - 1;
    
    printf("Avant msgsnd \n");
    if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
    {
      perror("erreur d'envoi");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
    // TO DO (étape 5)
    // Envoi d'une requete ACHAT au serveur

    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = ACHAT;
    msg.data1 = articleEnCours.id;

    char str[20];
    sprintf(str, "%d", getQuantite());

    strcpy(msg.data2, str);

    if(msgsnd(idQ,&msg,sizeof(MESSAGE)-sizeof(long),0)==-1)
    {
      perror("(CLIENT) ERREUR lors de l'envoie d'un achat");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
    // TO DO (étape 6)
    int indice = getIndiceArticleSelectionne();

    if(indice == -1)
    {
      dialogueErreur("SUPPRIMER ARTICLE", "veuillez selectionner un article");
    }// Envoi d'une requete CANCEL au serveur
    else
    {
        msg.type = 1;
        msg.expediteur = getpid();
        msg.requete = CANCEL;

        msg.data1 = indice;

        fprintf(stderr, "(CLIENT)l'id du fruit selectioné est %d", indice);

        if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
        {
          perror("(CLIENT) ERREUR lors de l'envoie du message CADDIE au serveur");
          exit(1);
        }

    }
    
    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur

    msg.requete = CADDIE;

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("(CLIENT) ERREUR lors de l'envoie du message CADDIE au serveur");
      exit(1);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL_ALL au serveur
    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = CANCEL_ALL;

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("(CLIENT) ERREUR lors de l'envoie du message CADDIE au serveur");
      exit(1);
    }
    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
    msg.requete = CADDIE;

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("(CLIENT) ERREUR lors de l'envoie du message CADDIE au serveur");
      exit(1);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
    // TO DO (étape 7)
    // Envoi d'une requete PAYER au serveur

    char tmp[100];
    sprintf(tmp,"Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.",totalCaddie);
    dialogueMessage("Payer...",tmp);

    msg.type = 1;
    msg.expediteur = getpid();
    msg.requete = PAYER;

    if((msgsnd(idQ, &msg, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
    {
      perror("(CLIENT) ERREUR lors de l'envoie du message CADDIE au serveur");
      exit(1);
    }


    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    MESSAGE m, reponse;
    printf("ENtrer dans le handler \n");
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) != -1)  // !!! a modifier en temps voulu !!!
    {
      switch(m.requete)
      {
        case LOGIN :
                    fprintf(stderr,"(CLIENT %d) Requete login reçue de %d\n",getpid(),m.expediteur);
                    if(m.data1 == 1)
                    {
                      w -> loginOK();
                      logged = 1;
                      w -> dialogueMessage("CONNEXION", m.data4);
                      m.type = 1;
                      m.expediteur = getpid();
                      m.requete = CONSULT;
                      m.data1 = 1;
                      
                      printf("Avant msgsnd \n");
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1)
                      {
                        perror("erreur d'envoi");
                        msgctl(idQ,IPC_RMID,NULL);
                        exit(1);
                      }
                      printf("apres \n");

                    }
                    else
                    {
                      w-> dialogueErreur("CONNEXION", m.data4);
                    }
                    break;

        case CONSULT : // TO DO (étape 3)
                    printf("Entre du CONSULT");
                    articleEnCours.id = m.data1;
                    strcpy(articleEnCours.intitule, m.data2);
                    articleEnCours.prix = m.data5;
                    articleEnCours.stock = atoi(m.data3);
                    strcpy(articleEnCours.image, m.data4);

                    w -> setArticle(articleEnCours.intitule, articleEnCours.prix, articleEnCours.stock, articleEnCours.image);
                    break;

        case ACHAT : // TO DO (étape 5)

                    printf("entrer du ACHAT");

                    if(atoi(m.data3) != 0)
                    {
                      articleEnCours.id = m.data1;
                      strcpy(articleEnCours.intitule, m.data2);
                      articleEnCours.prix = m.data5;
                      fprintf(stderr, m.data3);
                      int stock =  atoi(m.data3);
                      articleEnCours.stock -= stock;
                      strcpy(articleEnCours.image, m.data4);

                      w -> setArticle(articleEnCours.intitule, articleEnCours.prix, articleEnCours.stock, articleEnCours.image);

                      w -> dialogueMessage(m.data3, "unité acheter avec succes");
                      reponse.type = 1;
                      reponse.expediteur = getpid();
                      reponse.requete = CADDIE;

                      w -> videTablePanier();

                      totalCaddie = 0.0;


                      if((msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long), 0)) == -1)
                      {
                        perror("(CLIENT) ERREUR lors de l'envoie du message CADDIE au serveur");
                        exit(1);
                      }

                      
                      
                    }
                    else
                    {
                      w -> dialogueErreur("ACHAT", "Il n'y a pas assez dans le stock");
                    }
                    break;

         case CADDIE : // TO DO (étape 5)
                  {
                    int i = 0;
                    while(i != -1)
                    {
                      w -> ajouteArticleTablePanier(m.data2, m.data5, atoi(m.data3));

                      totalCaddie += m.data5;

                      w -> setTotal(totalCaddie);

                      i = msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),IPC_NOWAIT);
                      if( i == -1)
                      {
                        break;
                      }
                    }
                  }
                    
                    
                    break;

         case TIME_OUT : // TO DO (étape 6)
                    fprintf(stderr, "(CLIENT) reception d'un TIME_OUT");
                    w -> logoutOK();

                    w->videTablePanier();
                    totalCaddie = 0.0;
                    w->setTotal(-1.0);

                    w -> dialogueMessage("DECONNEXION", "Vous avez été déconnecter pour cause d'ignactiviter");
                    break;

         case BUSY : // TO DO (étape 7)
                    {
                      int i = 0;
                      while(i != -1)
                      { 

                        w -> dialogueErreur("MAINTENANCE", "Le serveur est en maintenance !");
                        i = msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),IPC_NOWAIT);
                        if( i == -1)
                        {
                          break;
                        }
                      }
                    }
                    


                    
                    break;

         default :
                    break;
      }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR2(int sig)
{
  w->setPublicite(pShm);
}