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
#include <wait.h>

#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
typedef struct workerData{
	int number;
	int fdPreviousWorker;
	int fdNextWorker;
	int fdToMaster;
	bool hasNextWorker;
}workerData;

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[] , struct workerData* data)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");

    // remplir la structure
    data->number = atoi(argv[1]);
    data->fdToMaster = atoi(argv[2]);
    data->fdPreviousWorker = atoi(argv[3]);
    data->hasNextWorker = false;
    
    
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(struct workerData* data)
{
	// boucle infinie :
	while(true){

		//    attendre l'arrivée d'un nombre à tester
		int numberToTest = 0;
		int ret = read(data->fdPreviousWorker, &numberToTest, sizeof(int));
		
		//    si ordre d'arrêt
		if(numberToTest == -1){
		
		//       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
			if(data->hasNextWorker){
				ret = write(data->fdNextWorker, &numberToTest, sizeof(int));
				myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
				wait(NULL);
			}
			//       sortir de la boucle
			break;
		}
		
		//    sinon c'est un nombre à tester, 4 possibilités :
		//           - le nombre est premier
		else if(numberToTest == data->number){
			bool isPrime = true;
			ret = write(data->fdToMaster, &isPrime, sizeof(bool));
    		myassert(ret == sizeof(bool), "Erreur: Taille du message envoyé incorrecte");
		}
		
		//           - le nombre n'est pas premier
		else if((numberToTest % data->number) == 0){
			bool isPrime = false;
			ret = write(data->fdToMaster, &isPrime, sizeof(bool));
    		myassert(ret == sizeof(bool), "Erreur: Taille du message envoyé incorrecte");
		}
		else{
			//           - s'il y a un worker suivant lui transmettre le nombre
			if(data->hasNextWorker){
				ret = write(data->fdNextWorker, &numberToTest, sizeof(int));
				myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
			}
			//           - s'il n'y a pas de worker suivant, le créer
			else{
				int ret, retFork;
				int fdToWorker[2];
				ret = pipe(fdToWorker);
				myassert(ret == 0, "Erreur création du pipe anonyme fdToWorker");
				retFork = fork();
				myassert(retFork != -1, "Erreur: création du fils");
				
				if(retFork == 0){
					close(fdToWorker[1]);
					initWorker(numberToTest, data->fdToMaster, fdToWorker[0]);
				}
				else{
					close(fdToWorker[0]);
					data->fdNextWorker = fdToWorker[1];
					data->hasNextWorker = true;
					ret = write(data->fdNextWorker, &numberToTest, sizeof(int));
					myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
					
				}
			}
		}
    }
}

/************************************************************************
* 						Mes Fonctions
************************************************************************/




/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
	workerData data;
    parseArgs(argc, argv , &data);
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier
    
    
    loop(&data);
    
    
	

    // libérer les ressources : fermeture des files descriptors par exemple
    close(data.fdPreviousWorker);
    
    if(data.hasNextWorker){
    close(data.fdNextWorker);
    }
    
    close(data.fdToMaster);

    return EXIT_SUCCESS;
}
