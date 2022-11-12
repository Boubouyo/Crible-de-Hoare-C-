#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"

#include "master_client.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// fonctions éventuelles internes au fichier

//Ouvre le tube nommé d'écriture
int openWritingTube(const char * writingTube){
	int fd = open(writingTube, O_WRONLY);
    myassert(fd != -1, "Erreur openWritingTube: Echec de l'ouverture en écriture du tube nommé");
    return fd;
}

//Ouvre le tube nommé de lecture
int openReadingTube(const char * readingTube){
	int fd = open(readingTube, O_RDONLY);
    myassert(fd != -1, "Erreur openWritingTube: Echec de l'ouverture en lecture du tube nommé");
    return fd;
}

//Lecture sur le tube nommé
void readingInTube(int fd, struct masterClientMessage* message){

	int ret = read(fd, message, sizeof(struct masterClientMessage));
    myassert((ret == sizeof(struct masterClientMessage)) || (ret == 0), "Erreur readingInTube: Taille du message reçu incorrecte");
}

//Ecriture sur le tube nommé
void writingInTube(int fd, const struct masterClientMessage* message){

	int ret = write(fd, message, sizeof(struct masterClientMessage));
    myassert(ret == sizeof(struct masterClientMessage), "Erreur writingInTube: Taille du message envoyé incorrecte");
    
}


// fonctions éventuelles proposées dans le .h

void sendMessage(const char * writingTube, const struct masterClientMessage * message){
	int fd = openWritingTube(writingTube);
	writingInTube(fd, message);
	close(fd);
}

void receiveMessage(const char * readingTube, struct masterClientMessage * message){
	int fd = openReadingTube(readingTube);
	readingInTube(fd, message);
	close(fd);
}





