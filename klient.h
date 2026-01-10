//
// Created by Alex on 1/4/2026.
//

#ifndef SEMESTRALKA_KLIENT_H
#define SEMESTRALKA_KLIENT_H
#define _POSIX_C_SOURCE 200809L
#include "zdielane_prostriedky.h"

typedef struct {
    char buf[VYSKA_PLOCHY][SIRKA_PLOCHY];

    int hernyCasMs;
    STAV_HRY stav;
    HERNE_MODY mod;
    TYPY_SVETOV svet;

    int skore[MAX_POCET_HRACOV];
    int casVhre[MAX_POCET_HRACOV];
    STAV_HRACA stavHraca[MAX_POCET_HRACOV];

    int indexHraca;
} SNAPSHOT;

typedef struct {
    HRA* hra;
    int indexHraca;
} RENDER_ARG;

void posliAkciu(HRA* hra, int indexHraca, AKCIA_HADIKA akciaH);
int vyberHraca(HRA* hra);

void vytvorSnapshot(HRA* hra, SNAPSHOT* snapshot, int indexHraca);
void vykresliSnapshot(const SNAPSHOT* s);

void spracujVstup(HRA* hra, int indexHraca, int ch);

void* renderVlakno(void* arg);

int spustiKlienta(int indexHraca);
#endif //SEMESTRALKA_KLIENT_H