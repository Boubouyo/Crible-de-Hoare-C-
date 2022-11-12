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

// fonctions éventuelles proposées dans le .h

int openWritingTube(const char * writingTube){
	int fd = open(writingTube, O_WRONLY);
    myassert(fd != -1, "Erreur openWritingTube: Echec de l'ouverture en écriture du tube nommé");
    return fd;
}

int openReadingTube(const char * readingTube){
	int fd = open(readingTube, O_RDONLY);
    myassert(fd != -1, "Erreur openWritingTube: Echec de l'ouverture en lecture du tube nommé");
    return fd;
}

void writingInTube(int fd, const int* message){

	int ret = write(fd, message, sizeof(int));
    myassert(ret == sizeof(char), "Erreur writingInTube: Taille du message envoyé incorrecte");
    
}

void readingInTube(int fd, int* message){

	int ret = read(fd, message, sizeof(char));
    myassert((ret == sizeof(char)) || (ret == 0), "Erreur readingInTube: Taille du message reçu incorrecte");
}

void closeTube(int fd){
	close(fd);
}







