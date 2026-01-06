//
// Created by Alex on 1/4/2026.
//
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "shared_memory.h"

#include <stdbool.h>

int casVMiliSekundach() {
    struct timeval tv;
    mingw_gettimeofday(&tv, NULL);
    return (((int)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

void defaultNastaveniaHry(HRA* hra) {
    hra->jeInicializovana = false;
    hra->tik = 0;
    hra->startHry = casVMiliSekundach();
    hra->poslednyTik = casVMiliSekundach();
    hra->stavHry = MENU;
    hra->hernyMod = STANDARDNY;
    hra->typSveta = BEZ_PREKAZOK;
    hra->zmiznutiePosledneho = 0;
    hra->trvanieHry = 0;
    for (int i = 0; i < VYSKA_PLOCHY; ++i) {
        for (int j = 0; j < SIRKA_PLOCHY; ++j) {
            hra->hernaPlocha[i][j] = ' ';
        }
    }
    for (int i = 0; i < MAX_POCET_HRACOV; ++i) {
        hra->ovocie[i].suradnice.suradnicaX = -1;
        hra->ovocie[i].suradnice.suradnicaY = -1;
        SLOT_HRACA* slot = &hra->hraci[i];
        slot->stavHraca = NEPRIPOJENY;
        slot->skore = 0;
        slot->stopPoPauze = 0;
        slot->casVhre = 0;
        slot->hadik.aktualnaDlzka = 0;
        slot->hadik.jeZivy = false;
        slot->hadik.aktualnySmer = VPRAVO;
    }
}

int serverOtvorenie(SHM* pamat, _Bool inicializovana) {
    return 0;
}

int klientOtvorenie(SHM* pamat, _Bool inicializovana) {
    if (!pamat) { return -1; }
    pamat->fd = -1;
    pamat->hra = NULL;
    int fd = shm_open(NAZOV_SHM, O_RDWR, 0666);
    if (fd < 0) return -1;

    void* mapa = mmap(NULL, sizeof(HRA), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapa == MAP_FAILED) {
        close(fd);
        return -1;
    }

    pamat->fd = fd;
    pamat->hra = (HRA*)mapa;

    int start = casVMiliSekundach();
    while (1) {
        if (pamat->hra->inicializovana == 1) {
            return 0;
        }

        int elapsed = casVMiliSekundach() - start;
        if (elapsed >= 3000) {
            zatvorSHM(pamat);
            return -1;
        }

        usleep(10 * 1000);
    }
}

void zatvorSHM(SHM* pamat) {
    if (!pamat) {
        return;
    }
    if (pamat->hra) {
        munmap(pamat->hra, sizeof(HRA));
        pamat->hra = NULL;
    }
    if (pamat->fd >= 0) {
        close(pamat->fd);
        pamat->fd = -1;
    }
}

int zrusSHM() {
    return shm_unlink(NAZOV_SHM);
}
