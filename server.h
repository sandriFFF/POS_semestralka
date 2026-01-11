//
// Created by Alex on 1/4/2026.
//

#ifndef SEMESTRALKA_SERVER_H
#define SEMESTRALKA_SERVER_H

#include "zdielane_prostriedky.h"

SMER otocenieDoprava(SMER smer);
SMER otocenieDolava(SMER smer);

_Bool rovnakaPozicia(POZICIA* poziciaA, POZICIA* poziciaB);
POZICIA dalsiaPozicia(POZICIA* pozicia, SMER smer);
_Bool jeVnutriPlochy(POZICIA* pozicia);
_Bool hadikObsahujePoziciu(const HADIK* hadik, POZICIA* pozicia);

void posunHadika(POZICIA* novaPoziciaHlavy, HADIK* hadik, _Bool rast);

_Bool ovocieNaPloche(OVOCIE* ovocie);
void deaktivujOvocie(OVOCIE* ovocie);
int indexOvociaNaDanejPozicii(HRA* hra, POZICIA* pozicia);
_Bool generujOvocie(HRA* hra);
void skontrolujOvocie(HRA* hra);

_Bool jePrekazka(HRA* hra, POZICIA* pozicia);
_Bool jePoziciaObsadenaHadikom(HRA* hra, POZICIA* pozicia);

int najdiMiestoSpawnu(HRA* hra, POZICIA* pozicia);
void vytvorHadika(HRA* hra, int indexHraca);
void spracujAkcie(HRA* hra);
void pohniHadikov(HRA* hra);
void skontrolujKoniecHry(HRA* hra);
int spustiServer(_Bool novaInicializacia);

#endif //SEMESTRALKA_SERVER_H