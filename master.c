#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(int fdFromWorker, int fdToWorker)
{
	bool stop = false;
	while(!stop){
		struct masterClientMessage sendingMessage;
		sendingMessage.isPrime = false;
		sendingMessage.order = -1;
		sendingMessage.number = -1;
		
		struct masterClientMessage receivingMessage;
		
		receiveMessage(CLIENT_TO_MASTER_TUBE, &receivingMessage);
		
		switch (receivingMessage.order){
			//STOP
			case -1: {
				printf("STOP\n");
				printf("%d\n", receivingMessage.order);
				sendingMessage.number = 10;
				sendMessage(MASTER_TO_CLIENT_TUBE, &sendingMessage);
				stop = true;
			 	break;
			}
			//COMPUTE
			case 1: {
				printf("COMPUTE\n");
				printf("%d %d\n", receivingMessage.order, receivingMessage.number);
				
				sendingMessage.isPrime = true;
				int ret = write(fdToWorker, &receivingMessage.number, sizeof(int));
    			myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
    			
    			ret = read(fdFromWorker, &sendingMessage.number, sizeof(int));
    			myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
    			printf("%d \n", sendingMessage.number);
				sendMessage(MASTER_TO_CLIENT_TUBE, &sendingMessage);
			 	break;
			}
			//HOW_MANY_PRIME
			case 2: {
				printf("HOW_MANY_PRIME\n");
				printf("%d\n", receivingMessage.order);
				
				sendingMessage.number = 20;
				
				sendMessage(MASTER_TO_CLIENT_TUBE, &sendingMessage);
			 	break;
			}
			//HIGHEST_PRIME 
			case 3: {
				printf("HIGHEST_PRIME\n");
				printf("%d\n", receivingMessage.order);
				
				sendingMessage.number = 30;
				sendMessage(MASTER_TO_CLIENT_TUBE, &sendingMessage);
			 	break;
			}
		}
     }

    // boucle infinie :
    // - ouverture des tubes (cf. rq client.c)
    // - attente d'un ordre du client (via le tube nommé)
    // - si ORDER_STOP
    //       . envoyer ordre de fin au premier worker et attendre sa fin
    //       . envoyer un accusé de réception au client
    // - si ORDER_COMPUTE_PRIME
    //       . récupérer le nombre N à tester provenant du client
    //       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
    //             il faut connaître le plus nombre (M) déjà enovoyé aux workers
    //             on leur envoie tous les nombres entre M+1 et N-1
    //             note : chaque envoie déclenche une réponse des workers
    //       . envoyer N dans le pipeline
    //       . récupérer la réponse
    //       . la transmettre au client
    // - si ORDER_HOW_MANY_PRIME
    //       . transmettre la réponse au client
    // - si ORDER_HIGHEST_PRIME
    //       . transmettre la réponse au client
    // - fermer les tubes nommés
    // - attendre ordre du client avant de continuer (sémaphore : précédence)
    // - revenir en début de boucle
    //
    // il est important d'ouvrir et fermer les tubes nommés à chaque itération
    // voyez-vous pourquoi ?
}


// Fonction de création des tubes nommées utilisés par le master/client
static void createNamedTube(){
	int ret = mkfifo(MASTER_TO_CLIENT_TUBE, 0641);
	myassert(ret == 0, "Erreur création tube nommé: MasterToClient");
	
	ret = mkfifo(CLIENT_TO_MASTER_TUBE, 0641);
	myassert(ret == 0, "Erreur création tube nommé: ClientToMaster");

}

// Fonction de destruction des tubes nommées utilisés par le master/client
static void unlinkNamedTube(){
	int ret = unlink(MASTER_TO_CLIENT_TUBE);
	myassert(ret == 0, "Erreur fermeture tube nommé: MasterToClient");
	
	ret = unlink(CLIENT_TO_MASTER_TUBE);
	myassert(ret == 0, "Erreur fermeture tube nommé: ClientToMaster");

}

//---------------------------------------------------------------
// Création du sémaphore 
static int my_semget()
{
    int id = semget(MA_CLE, 2, IPC_CREAT|0641);
	myassert(id != -1, "Erreur my_semget: Echec de la création du sémaphore");
	int ret = semctl(id, 0, SETVAL, 1);
	myassert(ret != -1, "Erreur my_semget: Echec de l'initialisation du premier sémaphore");
	ret = semctl(id, 1, SETVAL, 1);
	myassert(ret != -1, "Erreur my_semget: Echec de l'initialisation du second sémaphore");
	
	
    return id;
}

// destruction du sémaphore
static void my_destroy(int semId)
{
    int ret = semctl(semId, -1, IPC_RMID);
	myassert(ret != -1, "Erreur my_destroy: Echec de la desturction du sémaphore");
}

//----------------------------------------------------------------------




/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    // - création des sémaphores
    int semId = my_semget();
    // - création des tubes nommés
    createNamedTube();
    // - création du premier worker
    
    int ret, retFork;
    int fdFromWorker[2];
    int fdToWorker[2];
    
    ret = pipe(fdToWorker);
    myassert(ret == 0, "Erreur création du pipe anonyme fdToWorker");
    ret = pipe(fdFromWorker);
    myassert(ret == 0, "Erreur création du pipe anonyme fdFromWorker");
    
    
    retFork = fork();
    myassert(retFork != -1, "Erreur: création du fils");
    
    if(retFork == 0){
    	close(fdFromWorker[0]);
    	close(fdToWorker[1]);
    	initWorker(2, fdFromWorker[1], fdToWorker[0]);
    }
    else{
    	close(fdFromWorker[1]);
    	close(fdToWorker[0]);
    }
    
    

    // boucle infinie
    loop(fdFromWorker[0], fdToWorker[1]);

    // destruction des tubes nommés, des sémaphores, ...
    
    entrerSync(semId);
    my_destroy(semId);
	unlinkNamedTube();
    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
