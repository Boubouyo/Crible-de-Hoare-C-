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
void loop(/* paramètres */)
{
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
void createNamedTube(){
	int ret = mkfifo(MASTER_TO_CLIENT_TUBE, 0641);
	myassert(ret == 0, "Erreur création tube nommé: MasterToClient");
	
	ret = mkfifo(CLIENT_TO_MASTER_TUBE, 0641);
	myassert(ret == 0, "Erreur création tube nommé: ClientToMaster");

}

// Fonction de destruction des tubes nommées utilisés par le master/client
void unlinkNamedTube(){
	int ret = unlink(MASTER_TO_CLIENT_TUBE);
	myassert(ret == 0, "Erreur fermeture tube nommé: MasterToClient");
	
	ret = unlink(CLIENT_TO_MASTER_TUBE);
	myassert(ret == 0, "Erreur fermeture tube nommé: ClientToMaster");

}



/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    // - création des sémaphores
    // - création des tubes nommés
    createNamedTube();
    // - création du premier worker
    int message = 5;
    
    int fd1 = openWritingTube(MASTER_TO_CLIENT_TUBE);
    writingInTube(fd1, &message);
    closeTube(fd1);
    
    int fd2 = openReadingTube(CLIENT_TO_MASTER_TUBE);
    
    readingInTube(fd2, &message);
    closeTube(fd2);
    
    
    
    
    printf("%d \n", message);
    
    
    
    

    // boucle infinie
    loop(/* paramètres */);

    // destruction des tubes nommés, des sémaphores, ...
    
	unlinkNamedTube();
    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
