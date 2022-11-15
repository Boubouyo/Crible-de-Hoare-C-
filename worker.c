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

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
struct workerData{
	int number;
	int fdPreviousWorker;
	int fdNextWorker;
	int fdToMaster;
};

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
    
    
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(struct workerData* data)
{
	int v = 0;
	int ret = read(data->fdPreviousWorker, &v, sizeof(int));
    myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
    printf("Je suis le worker %d, et j'envois le nombre %d\n", data->number, v);
    if(data->number != 10){		
		ret = write(data->fdNextWorker, &v, sizeof(int));
		myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
    }
    else{
     	ret = write(data->fdToMaster, &v, sizeof(int));
    	myassert(ret == sizeof(int), "Erreur: Taille du message envoyé incorrecte");
    
    }
    // boucle infinie :
    //    attendre l'arrivée d'un nombre à tester
    //    si ordre d'arrêt
    //       si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    //       sortir de la boucle
    //    sinon c'est un nombre à tester, 4 possibilités :
    //           - le nombre est premier
    //           - le nombre n'est pas premier
    //           - s'il y a un worker suivant lui transmettre le nombre
    //           - s'il n'y a pas de worker suivant, le créer
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
	struct workerData data;
    parseArgs(argc, argv , &data);
    
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier
    if(atoi(argv[1]) != 10){
		int ret, retFork;
		int fdToWorker[2];
		ret = pipe(fdToWorker);
		myassert(ret == 0, "Erreur création du pipe anonyme fdToWorker");
		retFork = fork();
		myassert(retFork != -1, "Erreur: création du fils");
		
		if(retFork == 0){
			close(fdToWorker[1]);
			initWorker(atoi(argv[1]) + 1, atoi(argv[2]), fdToWorker[0]);
		}
		else{
			close(fdToWorker[0]);
			data.fdNextWorker = fdToWorker[1];
		}
			
			
		
	}
	printf("Je suis dans le Worker %d \n", atoi(argv[1]));
    loop(&data);
    
    
	

    // libérer les ressources : fermeture des files descriptors par exemple
    close(data.fdPreviousWorker);
    
    if(data.number != 10)
    close(data.fdNextWorker);
    
    close(data.fdToMaster);

    return EXIT_SUCCESS;
}
