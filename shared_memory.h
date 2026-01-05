//
// Created by Alex on 1/4/2026.
//

#ifndef SEMESTRALKA_SHARED_MEMORY_H
#define SEMESTRALKA_SHARED_MEMORY_H
#include "zdielane_prostriedky.h"

typedef struct {
    HRA* hra;
    int fd;
} SHM;

int serverOtvorenie(SHM* pamat, _Bool inicializovana);
int klientOtvorenie(SHM* pamat, _Bool inicializovana);
void zatvorSHM(SHM* pamat);
int zrusSHM();

#endif //SEMESTRALKA_SHARED_MEMORY_H