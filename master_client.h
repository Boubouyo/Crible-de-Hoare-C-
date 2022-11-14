#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

#define MASTER_TO_CLIENT_TUBE "MasterToClient"
#define CLIENT_TO_MASTER_TUBE "ClientToMaster"

// Mutex
#define MON_FICHIER "master_client.h"
#define PROJ_ID 5

#define MA_CLE ftok(MON_FICHIER, PROJ_ID)

// Tube nommé
// Enlever isPrime car un booléen et un entier valant 0 ou 1(combiné a isPrime)
struct masterClientMessage{
	bool isPrime;
	int order;
	int number;
};

//Empeche le master de detruire les tubes et semaphores avant que le client est fini
void entrerSync(int semId);
//Permet au master de detruire les tubes et semaphores
void sortirSync(int semId);


//Envoie un message par le tube nommé writingTube
void sendMessage(const char * writingTube, const struct masterClientMessage * message);
//Recois un message par le tube nommé readingTube
void receiveMessage(const char * readingTube, struct masterClientMessage * message);

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation


#endif
