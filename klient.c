//
// Created by Alex on 1/4/2026.
//

#include "klient.h"

#include <ncurses.h>

#include "shared_memory.h"

void posliAkciu(HRA* hra, int indexHraca, AKCIA_HADIKA akciaH) {
	if(indexHraca < 0 || indexHraca >= MAX_POCET_HRACOV) return;
	pthread_mutex_lock(&hra->mutex);
	hra->hraci[indexHraca].akcia = akciaH;
	pthread_cond_broadcast(&hra->signal);
    pthread_mutex_unlock(&hra->mutex);
}

int vyberHraca(HRA* hra) {
	int vyber = -1;
	for (int i = 0; i < MAX_POCET_HRACOV; i++) {
		STAV_HRACA stavHraca = hra->hraci[i].stavHraca;
		if (stavHraca == NEPRIPOJENY || stavHraca == MRTVY || stavHraca == ODIDENY) {
		    vyber = i;
			break;
		}
	}
	return vyber;
}

void vytvorSnapshot(HRA* hra, SNAPSHOT* snapshot, int indexHraca) {
	snapshot->indexHraca = indexHraca;
	snapshot->stav = hra->stavHry;
	snapshot->mod = hra->hernyMod;
	snapshot->svet = hra->typSveta;
	for (int i = 0; i < SIRKA_PLOCHY; i++) {
		for (int j = 0; j < VYSKA_PLOCHY; j++) {
			snapshot->buf[i][j] = hra->hernaPlocha[i][j];
		}
	}
	snapshot->hernyCasMs = hra->trvanieHry;
	for (int i = 0; i < MAX_POCET_HRACOV; i++) {
		int x = hra->ovocie[i].suradnice.suradnicaX;
		int y = hra->ovocie[i].suradnice.suradnicaY;
		if (x >= 0 && x < SIRKA_PLOCHY && y >= 0 && y < VYSKA_PLOCHY) {
			snapshot->buf[x][y] = '*';
		}
	}
	for (int i = 0; i < MAX_POCET_HRACOV; ++i) {
		HADIK* h = &hra->hraci[i].hadik;
		if (!h->jeZivy || h->aktualnaDlzka <= 0) continue;
//demonstracne ucely, inak kazdy @#######
		POZICIA ph = h->telo[0];
		if (ph.suradnicaX >= 0 && ph.suradnicaY >= 0 &&
			ph.suradnicaX < SIRKA_PLOCHY && ph.suradnicaY < VYSKA_PLOCHY) {
			snapshot->buf[ph.suradnicaY][ph.suradnicaX] = (i == 0) ? 'O' : '@';
			}

		for (int k = 1; k < h->aktualnaDlzka; ++k) {
			POZICIA p = h->telo[k];
			if (p.suradnicaX >= 0 && p.suradnicaY >= 0 &&
				p.suradnicaX < SIRKA_PLOCHY && p.suradnicaY < VYSKA_PLOCHY) {
				snapshot->buf[p.suradnicaY][p.suradnicaX] = (i == 0) ? 'o' : '#';
				}
		}
	}

	for (int i = 0; i < MAX_POCET_HRACOV; ++i) {
		snapshot->skore[i] = hra->hraci[i].skore;
		snapshot->casVhre[i] = hra->hraci[i].casVHre;
		snapshot->stavHraca[i] = hra->hraci[i].stavHraca;
	}
}
void vykresliSnapshot(const SNAPSHOT* s) {
	erase();

	mvprintw(0, 0, "Hadik | hrac=%d | stavHry=%d | mod=%d | svet=%d | cas=%.1fs",
			 s->indexHraca, (int)s->stav, (int)s->mod, (int)s->svet, s->hernyCasMs / 1000.0);

	mvprintw(1, 0, "H0: skore=%d cas=%.1fs stav=%d   |   H1: skore=%d cas=%.1fs stav=%d",
			 s->skore[0], s->casVhre[0] / 1000.0, (int)s->stavHraca[0],
			 s->skore[1], s->casVhre[1] / 1000.0, (int)s->stavHraca[1]);

	mvprintw(2, 0, "Ovl: j=pripoj p=pauza r=pokracuj l=odid q=quit | sipky L/R alebo a/d = zaboc");

	int top = 4;
	int left = 0;

	mvaddch(top, left, '+');
	for (int x = 0; x < SIRKA_PLOCHY; ++x) mvaddch(top, left + 1 + x, '-');
	mvaddch(top, left + 1 + SIRKA_PLOCHY, '+');

	for (int y = 0; y < VYSKA_PLOCHY; ++y) {
		mvaddch(top + 1 + y, left, '|');
		for (int x = 0; x < SIRKA_PLOCHY; ++x) {
			mvaddch(top + 1 + y, left + 1 + x, s->buf[y][x]);
		}
		mvaddch(top + 1 + y, left + 1 + SIRKA_PLOCHY, '|');
	}

	mvaddch(top + 1 + VYSKA_PLOCHY, left, '+');
	for (int x = 0; x < SIRKA_PLOCHY; ++x) mvaddch(top + 1 + VYSKA_PLOCHY, left + 1 + x, '-');
	mvaddch(top + 1 + VYSKA_PLOCHY, left + 1 + SIRKA_PLOCHY, '+');

	if (s->stav == MENU) {
		mvprintw(top + 2 + VYSKA_PLOCHY, 0, "Hra je v MENU (server este nebezi alebo nebola spustena).");
	} else if (s->stav == SKONCILA) {
		mvprintw(top + 2 + VYSKA_PLOCHY, 0, "Hra SKONCILA. (Klient moze len citat stav)");
	}

	refresh();
}

void spracujVstup(HRA* hra, int indexHraca, int ch) {
	if (ch == 'a') {
		posliAkciu(hra, indexHraca, ZABOC_DOLAVA);
	} else if (ch == 'd') {
		posliAkciu(hra, indexHraca, ZABOC_DOPRAVA);
	} else if (ch == 'p') {
		posliAkciu(hra, indexHraca, PAUZA);
	} else if (ch == 'c') {
		posliAkciu(hra, indexHraca, POKRACOVANIE);
	} else if (ch == 's') {
		posliAkciu(hra, indexHraca, PRIPOJENIE);
	} else if (ch == 'l') {
		posliAkciu(hra, indexHraca, OPUSTENIE_HRY);
	}
}

void* renderVlakno(void* arg) {
	RENDER_ARG* vykreslovac = (RENDER_ARG*) arg;
	HRA* hra = vykreslovac->hra;
	int indexHraca = vykreslovac->indexHraca;
	SNAPSHOT snapshot;
	pthread_mutex_lock(&hra->mutex);
	vytvorSnapshot(hra, &snapshot, indexHraca);
	pthread_cond_broadcast(&hra->signal);
	pthread_mutex_unlock(&hra->mutex);
	vykresliSnapshot(&snapshot);
	pthread_mutex_lock(&hra->mutex);
	while (1) {
		if (hra->stavHry == SKONCILA) break;
		pthread_cond_wait(&hra->signal, &hra->mutex);
		if (hra->stavHry == SKONCILA) break;
		vytvorSnapshot(hra, &snapshot, indexHraca);
		pthread_mutex_unlock(&hra->mutex);
		vykresliSnapshot(&snapshot);
		pthread_mutex_lock(&hra->mutex);
	}
	pthread_mutex_unlock(&hra->mutex);
	return NULL;
}

int spustiKlienta(int indexHraca) {
	SHM shm;
	if (klientOtvorenie(&shm, 1) != 0) {
		return -2;
	}

	HRA* hra = shm.hra;

	if (indexHraca < 0) {
		indexHraca = vyberHraca(hra);
		if (indexHraca < 0) indexHraca = 0;
	}
	if (indexHraca >= MAX_POCET_HRACOV) indexHraca = MAX_POCET_HRACOV - 1;

	// ncurses init - chat
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	curs_set(0);

	pthread_t renderTh;
	RENDER_ARG arg;
	arg.hra = hra;
	arg.indexHraca = indexHraca;

	if (pthread_create(&renderTh, NULL, renderVlakno, &arg) != 0) {
		endwin();
		zatvorSHM(&shm);
		fprintf(stderr, "Nepodarilo sa spustit render thread.\n");
		return 1;
	}

	while (1) {
		int ch = getch();
		spracujVstup(hra, indexHraca, ch);
		usleep(5 * 1000);
	}

	pthread_mutex_lock(&hra->mutex);
	pthread_cond_broadcast(&hra->signal);
	pthread_mutex_unlock(&hra->mutex);

	pthread_join(renderTh, NULL);

	endwin();
	zatvorSHM(&shm);
	return 0;
}
