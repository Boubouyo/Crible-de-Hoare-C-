#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "myassert.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "master_worker.h"

// fonctions éventuelles internes au fichier

// fonctions éventuelles proposées dans le .h

void initWorker(int primeNumber, int fdToMaster, int fdfromMaster){
	
    char primeNumberBuffer[1000];
    sprintf(primeNumberBuffer, "%d", primeNumber);
    char fdToMasterBuffer[1000];
    sprintf(fdToMasterBuffer, "%d", fdToMaster);
    char fdfromMasterBuffer[1000];
    sprintf(fdfromMasterBuffer, "%d", fdfromMaster);
    char* const argv[5] = {"worker", primeNumberBuffer , fdToMasterBuffer, fdfromMasterBuffer ,NULL};
    execv(argv[0], argv);
    
}
