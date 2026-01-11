//
// Created by Alex on 1/4/2026.
//

#include "server.h"
#include "shared_memory.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int nahodneZIntervalu(int dolnaH, int hornaH) {
    return rand() % (hornaH - 1) + dolnaH;
}

SMER otocenieDoprava(SMER smer) {
    switch (smer) {
        case VLAVO: return HORE;
        case VPRAVO: return DOLE;
        case DOLE: return VLAVO;
        case HORE: return VPRAVO;
        default: return smer;
    }
}
SMER otocenieDolava(SMER smer) {
    switch (smer) {
        case VLAVO: return DOLE;
        case VPRAVO: return HORE;
        case DOLE: return VPRAVO;
        case HORE: return VLAVO;
        default: return smer;
    }
}
_Bool rovnakaPozicia(POZICIA* poziciaA, POZICIA* poziciaB) {
    return (poziciaA->suradnicaX == poziciaB->suradnicaX && poziciaA->suradnicaY == poziciaB->suradnicaY);
}

POZICIA dalsiaPozicia(POZICIA* pozicia, SMER smer) {
    if (smer == VLAVO) {
        pozicia->suradnicaX--;
    } else if (smer == VPRAVO) {
        pozicia->suradnicaX++;
    } else if (smer == DOLE) {
        pozicia->suradnicaY++;
    } else if (smer == HORE) {
        pozicia->suradnicaY--;
    }
    return *pozicia;
}
_Bool jeVnutriPlochy(POZICIA* pozicia) {
    return pozicia->suradnicaX < SIRKA_PLOCHY && pozicia->suradnicaY < VYSKA_PLOCHY && pozicia->suradnicaX >= 0 && pozicia->suradnicaY >= 0;
}
_Bool hadikObsahujePoziciu(const HADIK* hadik, POZICIA* pozicia) {
    if (!hadik->jeZivy || hadik->aktualnaDlzka <= 0) {
        return false;
    }
    for (int i = 0; i < hadik->aktualnaDlzka; i++) {
        if (rovnakaPozicia(&hadik->telo[i], pozicia)) {
            return true;
        }
    }
    return false;
}

void posunHadika(POZICIA* novaPoziciaHlavy, HADIK* hadik, _Bool rast) {
    if (!novaPoziciaHlavy || !hadik || !hadik->jeZivy) return;

    int max = SIRKA_PLOCHY * VYSKA_PLOCHY;

    int staraDlzka = hadik->aktualnaDlzka;
    if (staraDlzka < 1) staraDlzka = 1;

    int novaDlzka = staraDlzka + (rast ? 1 : 0);
    if (novaDlzka > max) novaDlzka = max;

    for (int i = novaDlzka - 1; i >= 1; --i) {
        if (!rast && i >= staraDlzka) continue;
        hadik->telo[i] = hadik->telo[i - 1];
    }

    hadik->telo[0] = *novaPoziciaHlavy;
    hadik->aktualnaDlzka = novaDlzka;
}

_Bool ovocieNaPloche(OVOCIE* ovocie) {
    return ovocie->suradnice.suradnicaX != -1 && ovocie->suradnice.suradnicaY != -1;
}

void deaktivujOvocie(OVOCIE* ovocie) {
    ovocie->suradnice.suradnicaX = -1;
    ovocie->suradnice.suradnicaY = -1;
}
int indexOvociaNaDanejPozicii(HRA* hra, POZICIA* pozicia) {
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        if (!ovocieNaPloche(&hra->ovocie[i])) {
            continue;
        }
        if (rovnakaPozicia(&hra->ovocie[i].suradnice, pozicia)) {
            return i;
        }
    }
    return -1;
}

_Bool generujOvocie(HRA* hra) {
    int index = -1;
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        if (!ovocieNaPloche(&hra->ovocie[i])) {
            index = i;
            break;
        }
    }
    if (index < 0) {
        return false;
    }
    for (int i = 0; i < 10000; i++) {
        POZICIA pozicia = {
            nahodneZIntervalu(0, SIRKA_PLOCHY - 1),
            nahodneZIntervalu(0, VYSKA_PLOCHY - 1),
        };
        if (jePrekazka(hra, &pozicia)) {
            continue;
        }
        if (jePoziciaObsadenaHadikom(hra, &pozicia)) {
            continue;
        }
        if (indexOvociaNaDanejPozicii(hra, &pozicia) >= 0) {
            continue;
        }
        hra->ovocie[index].suradnice = pozicia;
        return true;
    }
    return false;
}

void skontrolujOvocie(HRA* hra) {
    int ziveHadiky = 0;
    int aktivneOvocie = 0;
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        if (hra->hraci[i].hadik.jeZivy) {
            ziveHadiky++;
        }
        if (ovocieNaPloche(&hra->ovocie[i])) {
            aktivneOvocie++;
        }
    }
    while (aktivneOvocie < ziveHadiky) {
        if (!generujOvocie(hra)) {
            break;
        }
        aktivneOvocie++;
    }
}

_Bool jePrekazka(HRA* hra, POZICIA* pozicia) {
    if (hra->typSveta != S_PREKAZKAMI) {
        return false;
    }
    return hra->hernaPlocha[pozicia->suradnicaX][pozicia->suradnicaY] == 'X';
}
_Bool jePoziciaObsadenaHadikom(HRA* hra, POZICIA* pozicia) {
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        if (hra->hraci[i].hadik.jeZivy && hadikObsahujePoziciu(&hra->hraci[i].hadik, pozicia)) {
            return true;
        }
    }
    return false;
}

int najdiMiestoSpawnu(HRA* hra, POZICIA* pozicia) {
    for (int i = 0; i < 10000; i++) {
        POZICIA poziciaP = {
            nahodneZIntervalu(0, SIRKA_PLOCHY - 1),
            nahodneZIntervalu(0, VYSKA_PLOCHY - 1),
        };
        if (jePrekazka(hra, &poziciaP)) {
            continue;
        }
        if (jePoziciaObsadenaHadikom(hra, &poziciaP)) {
            continue;
        }
        if (indexOvociaNaDanejPozicii(hra, &poziciaP) >= 0) {
            continue;
        }
        *pozicia = poziciaP;
        return 1;
    }
    return 0;
}
void vytvorHadika(HRA* hra, int indexHraca) {
    SLOT_HRACA* s = &hra->hraci[indexHraca];
    POZICIA sp;
    if (!najdiMiestoSpawnu(hra, &sp)) {
        sp.suradnicaX = SIRKA_PLOCHY / 2;
        sp.suradnicaY = VYSKA_PLOCHY / 2;
    }
    s->hadik.jeZivy = true;
    s->hadik.aktualnaDlzka = 3;
    s->hadik.aktualnySmer = (SMER)nahodneZIntervalu(0, 3);

    s->hadik.telo[0] = sp;

    POZICIA p1 = sp, p2 = sp;
    SMER d = s->hadik.aktualnySmer;
    if (d == VPRAVO) { p1.suradnicaX = (sp.suradnicaX - 1 + SIRKA_PLOCHY) % SIRKA_PLOCHY;
        p2.suradnicaX = (sp.suradnicaX - 2 + SIRKA_PLOCHY) % SIRKA_PLOCHY; }
    if (d == VLAVO)  { p1.suradnicaX = (sp.suradnicaX + 1) % SIRKA_PLOCHY;
        p2.suradnicaX = (sp.suradnicaX + 2) % SIRKA_PLOCHY; }
    if (d == DOLE)   { p1.suradnicaY = (sp.suradnicaY - 1 + VYSKA_PLOCHY) % VYSKA_PLOCHY;
        p2.suradnicaY = (sp.suradnicaY - 2 + VYSKA_PLOCHY) % VYSKA_PLOCHY; }
    if (d == HORE)   { p1.suradnicaY = (sp.suradnicaY + 1) % VYSKA_PLOCHY;
        p2.suradnicaY = (sp.suradnicaY + 2) % VYSKA_PLOCHY; }

    s->hadik.telo[1] = p1;
    s->hadik.telo[2] = p2;

}

void spracujAkcie(HRA* hra) {
    int aktualCas = casVMiliSekundach();
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        SLOT_HRACA* s = &hra->hraci[i];
        AKCIA_HADIKA akcia = s->akcia;
        if (akcia == ZIADNA_AKCIA) {
            continue;
        }
        s->akcia = ZIADNA_AKCIA;
        switch (akcia) {
            case ZABOC_DOLAVA:
                if (s->hadik.jeZivy) {
                    s->hadik.aktualnySmer = otocenieDolava(s->hadik.aktualnySmer);
                }
                break;
            case ZABOC_DOPRAVA:
                if (s->hadik.jeZivy) {
                    s->hadik.aktualnySmer = otocenieDoprava(s->hadik.aktualnySmer);
                }
                break;
            case PAUZA:
                if (s->stavHraca == PRIPOJENY) {
                    s->stavHraca = PAUZNUTY;
                }
                break;
            case POKRACOVANIE:
                if (s->stavHraca == PAUZNUTY) {
                    s->stopPoPauze = aktualCas + 3000;
                    s->stavHraca = PRIPOJENY;
                }
                break;
            case OPUSTENIE_HRY:
                s->stavHraca = ODIDENY;
                s->hadik.jeZivy = false;
                break;
            case PRIPOJENIE:
                if (hra->stavHry != SKONCILA) {
                    if (s->stavHraca == NEPRIPOJENY || s->stavHraca == MRTVY || s->stavHraca == ODIDENY) {
                        s->stavHraca = PRIPOJENY;
                        vytvorHadika(hra, i);
                    }
                }
                break;
            default: break;

        }
    }
}

void pohniHadikov(HRA* hra) {
    int aktualCas = casVMiliSekundach();
    for (int i = 0; i < MAX_POCET_HRACOV; i++) {
        SLOT_HRACA* s = &hra->hraci[i];
        if (s->stavHraca != PRIPOJENY) {
            continue;
        }
        if (!s->hadik.jeZivy) {
            continue;
        }
        if (s->stopPoPauze > aktualCas) {
            continue;
        }
        POZICIA hlava = s->hadik.telo[0];
        POZICIA novaPozicia;
        novaPozicia = dalsiaPozicia(&hlava, s->hadik.aktualnySmer);
        if (!jeVnutriPlochy(&novaPozicia) || jePrekazka(hra, &novaPozicia)) {
            s->hadik.jeZivy = false;
            s->stavHraca = MRTVY;
            continue;
        }
        int indexOvocia = indexOvociaNaDanejPozicii(hra, &novaPozicia);
        _Bool rast = indexOvocia >= 0;
        for (int j = 0; j < s->hadik.aktualnaDlzka; j++) {
            if (rovnakaPozicia(&s->hadik.telo[j], &novaPozicia) && s->hadik.jeZivy) {
                s->hadik.jeZivy = false;
                s->stavHraca = MRTVY;
            }
        }
        for (int j = 0; j < MAX_POCET_HRACOV; j++) {
            if (i == j) {
                continue;
            }
            if (!hra->hraci[j].hadik.jeZivy) {
                continue;
            }
            if (hadikObsahujePoziciu(&hra->hraci[j].hadik, &novaPozicia)) {
                s->hadik.jeZivy = false;
                s->stavHraca = MRTVY;
                break;
            }
        }
        if (s->stavHraca == MRTVY) {
            continue;
        }
        if (rast) {
            deaktivujOvocie(&hra->ovocie[indexOvocia]);
            s->skore++;
        }
        posunHadika(&novaPozicia, &s->hadik, rast);
        s->casVHre += 100;
    }
}

void skontrolujKoniecHry(HRA* hra) {
    int aktualCas = casVMiliSekundach();
    if (hra->hernyMod == CASOVY && hra->trvanieHry > 0) {
        if (aktualCas - hra->startHry >= hra->trvanieHry) {
            hra->stavHry = SKONCILA;
            return;
        }
    }
    if (hra->hernyMod == STANDARDNY) {
        _Bool zivy = false;
        for (int i = 0; i < MAX_POCET_HRACOV; i++) {
            if (hra->hraci[i].hadik.jeZivy) {
                zivy = true;
                break;
            }
        }
        if (!zivy) {
            if (hra->zmiznutiePosledneho == 0) {
                hra->zmiznutiePosledneho = aktualCas;
            } else if (aktualCas - hra->zmiznutiePosledneho >= 10000) {
                hra->stavHry = SKONCILA;
                return;
            }
        } else {
            hra->zmiznutiePosledneho = 0;
        }
    }
}
int spustiServer(_Bool novaInicializacia) {
    SHM pamat;
    if (serverOtvorenie(&pamat, novaInicializacia ? 0 : 1) != 0) {
        perror("serverOtvorenie");
        return -1;
    }
    srand(time(NULL));
    HRA* hra = pamat.hra;
    const int tik = 100;
    pthread_mutex_lock(&hra->mutex);
    hra->stavHry = BEZI;
    hra->startHry = casVMiliSekundach();
    hra->zmiznutiePosledneho = 0;
    pthread_cond_broadcast(&hra->signal);
    pthread_mutex_unlock(&hra->mutex);
    while (1) {
        pthread_mutex_lock(&hra->mutex);
        if (hra->stavHry == SKONCILA) {
            pthread_cond_broadcast(&hra->signal);
            pthread_mutex_unlock(&hra->mutex);
            break;
        }
        spracujAkcie(hra);
        pohniHadikov(hra);
        skontrolujOvocie(hra);
        skontrolujKoniecHry(hra);
        pthread_cond_broadcast(&hra->signal);
        pthread_mutex_unlock(&hra->mutex);
        usleep(tik * 1000);
    }
    pthread_mutex_lock(&hra->mutex);
    hra->stavHry = SKONCILA;
    pthread_cond_broadcast(&hra->signal);
    pthread_mutex_unlock(&hra->mutex);
    zatvorSHM(&pamat);
    return 0;
}