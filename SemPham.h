#ifndef SEMPHAM_H

#define SEMPHAM_H
#include <sys/sem.h>
#include <iostream>
using namespace std;
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


int sem_wait(int idSem, bool nowait);

int sem_signal(int idSem);


#endif