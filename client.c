#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>

#include <math.h>

#include "myassert.h"

#include "master_client.h"

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<nombre>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombres premiers en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}

/************************************************************************
*								Mes Fonctions							*
************************************************************************/
// Récupération du sémaphore
static int my_semget()
{
    int id = semget(MA_CLE, 2, IPC_EXCL);
	myassert(id != -1, "Erreur my_semget: Echec de la récupération du sémaphore");
    return id;
}


//-----------------------------------------------------------------
// Entrer en SC(communication avec le master commencé)
static void entrerSC(int semId)
{
    struct sembuf monOp1 = {0,-1,0};
	int ret = semop(semId, &monOp1, 1);
	myassert(ret != -1, "Erreur entrerSC: Echec de l'entrée en section critique");
}

//-----------------------------------------------------------------
// Sortie de SC (communication avec le master terminé)
static void sortirSC(int semId)
{
    struct sembuf monOp1 = {0,+1,0};
	int ret = semop(semId, &monOp1, 1);
	myassert(ret != -1, "Erreur sortirSC: Echec de la sortie de section critique");
}


//------------------------------------------------------------------------
//crible d'erastosthene

typedef struct
{
    bool *tab;
    int tailleTab;
    int number;
    pthread_mutex_t theMutex;
} ThreadData;

void * codeThread(void * arg)
{
    ThreadData *data = (ThreadData *) arg;

    
    
    for(int i = 2 ; (data->number * i) - 2 < data->tailleTab; i++){
    
    	// Verrouillé: Entrer en SC
    	pthread_mutex_lock(&(data->theMutex));
    	data->tab[(data->number * i) - 2] = false;
    	//Deverouillé: Sortie de la SC
    	pthread_mutex_unlock(&(data->theMutex));
    }
    
    
    
    return NULL;
}



void local_compute(int number){
	
	pthread_mutex_t monMutex = PTHREAD_MUTEX_INITIALIZER;
	
	bool erastosthene[number - 1];
	
	for(int i = 0; i < number -1; i++){
	erastosthene[i] = true;
	}
	
	
	
	int nbThread = sqrt(number) - 1 ;
	
	pthread_t tabId[nbThread];
	
	ThreadData datas[nbThread];
	
	for (int i = 0; i < nbThread; i++)
    {
        datas[i].tab = erastosthene;
        datas[i].tailleTab = number -1;
        datas[i].number = i+2;
        datas[i].theMutex = monMutex;
    }
	
	
	// lancement des threads
    for (int i = 0; i < nbThread; i++)
    {
        // et donc on passe un pointeur sur une struct différente chaque fois
        int ret = pthread_create(&(tabId[i]), NULL, codeThread, &(datas[i]));
        myassert(ret == 0, "Echec du lancement d'un thread");
    }

    // attente de la fin des threads
    for (int i = 0; i < nbThread; i++)
    {
        int ret = pthread_join(tabId[i], NULL);
        myassert(ret == 0, "Echec de l'attente de la fin d'un thread");
    }
    
    //affichage des nombres premiers
    for(int i = 0; i < number - 1 ; i ++){
    	if(erastosthene[i]){
    		printf("%d ", i + 2);
    	}
    }
    printf("\n");
	
	pthread_mutex_destroy(&monMutex);
	
}

/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
	
	int number = -1;
    int order = parseArgs(argc, argv, &number);
    
    if(order == 4){
    //LOCAL_COMPUTE
		printf("LOCAL_COMPUTE\n");
		local_compute(number);
    }
    
    else{
    	
   		
   		int semId = my_semget();
   		entrerSC(semId);
		entrerSync(semId);
		
    	
    	
    	masterClientMessage sendingMessage;
		sendingMessage.isPrime = false;
		sendingMessage.order = order;
		sendingMessage.number = number;
		
		masterClientMessage receivingMessage;
		
		
    
		switch (sendingMessage.order){
		
			//STOP
			case -1: {
				printf("STOP\n");
				sendMessage(CLIENT_TO_MASTER_TUBE, &sendingMessage);
				receiveMessage(MASTER_TO_CLIENT_TUBE, &receivingMessage);
				printf("%d \n", receivingMessage.number);
			 	break;
			}
			//COMPUTE
			case 1: {
				printf("COMPUTE %d\n", sendingMessage.number);
				sendMessage(CLIENT_TO_MASTER_TUBE, &sendingMessage);
				receiveMessage(MASTER_TO_CLIENT_TUBE, &receivingMessage);
				printf("%d \n", receivingMessage.isPrime);
			 	break;
			}
			//HOW_MANY_PRIME
			case 2: {
				printf("HOW_MANY_PRIME\n");
				sendMessage(CLIENT_TO_MASTER_TUBE, &sendingMessage);
				receiveMessage(MASTER_TO_CLIENT_TUBE, &receivingMessage);
				
				printf("%d \n", receivingMessage.number);
			 	break;
			}
			//HIGHEST_PRIME 
			case 3: {
				printf("HIGHEST_PRIME\n");
				sendMessage(CLIENT_TO_MASTER_TUBE, &sendingMessage);
				receiveMessage(MASTER_TO_CLIENT_TUBE, &receivingMessage);
				printf("%d \n", receivingMessage.number);
			 	break;
			}
		}
		sortirSC(semId);
		sortirSync(semId);
	}
    
    return EXIT_SUCCESS;
}
