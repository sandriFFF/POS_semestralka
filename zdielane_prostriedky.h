//
// Created by Alex on 1/4/2026.
//

#ifndef SEMESTRALKA_HAD_H
#define SEMESTRALKA_HAD_H
#define MAX_POCET_HRACOV 2
#define SIRKA_PLOCHY 60
#define VYSKA_PLOCHY 60
#include <pthread.h>
#include <stdint.h>

typedef enum {
    HORE,
    DOLE,
    VPRAVO,
    VLAVO,
} SMER;

typedef enum {
    ZIADNA_AKCIA,
    ZABOC_DOLAVA,
    ZABOC_DOPRAVA,
    PAUZA,
    POKRACOVANIE,
    OPUSTENIE_HRY,
    PRIPOJENIE,
} AKCIA_HADIKA;

typedef enum {
    PRIPOJENY,
    NEPRIPOJENY,
    MRTVY,
    PAUZNUTY,
    ODIDENY,
} STAV_HRACA;

typedef enum {
    STANDARDNY,
    CASOVY,
} HERNE_MODY;

typedef enum {
    BEZ_PREKAZOK,
    S_PREKAZKAMI,
} TYPY_SVETOV;

typedef enum {
    NESKONCILA,
    SKONCILA,
    MENU,
} STAV_HRY;

typedef struct {
    int suradnicaX;
    int suradnicaY;
} POZICIA;

typedef struct {
    POZICIA telo[SIRKA_PLOCHY * VYSKA_PLOCHY];
    int aktualnaDlzka;
    int hlavaHadika;
    _Bool jeZivy;
    SMER aktualnySmer;
} HADIK;

typedef struct {
    POZICIA suradnice;
} OVOCIE;

typedef struct {
    STAV_HRACA stavHraca;
    HADIK hadik;
    int skore;
    int stopPoPauze;
    int casVhre;
} SLOT_HRACA;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t signal;
    char hernaPlocha[VYSKA_PLOCHY][SIRKA_PLOCHY];
    SLOT_HRACA hraci[MAX_POCET_HRACOV];
    OVOCIE ovocie[MAX_POCET_HRACOV];
    HERNE_MODY hernyMod;
    TYPY_SVETOV typSveta;
    STAV_HRY stavHry;
    int zmiznutiePosledneho;
    int trvanieHry;
} HRA;


#endif //SEMESTRALKA_HAD_H