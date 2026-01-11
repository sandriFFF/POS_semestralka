//
// Created by Alex on 1/4/2026.
//

#include "klient.h"

#include <stdio.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

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
	for (int y = 0; y < VYSKA_PLOCHY; y++) {
		for (int x = 0; x < SIRKA_PLOCHY; x++) {
			snapshot->buf[y][x] = hra->hernaPlocha[y][x];
		}
	}
	snapshot->hernyCasMs = hra->trvanieHry;
	for (int i = 0; i < MAX_POCET_HRACOV; i++) {
		int x = hra->ovocie[i].suradnice.suradnicaX;
		int y = hra->ovocie[i].suradnice.suradnicaY;
		if (x >= 0 && x < SIRKA_PLOCHY && y >= 0 && y < VYSKA_PLOCHY) {
			snapshot->buf[y][x] = '*';
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

// --- Minimal terminal UI --------------------------------------
static struct termios g_oldTerm;
static int g_oldFlags = -1;
static volatile sig_atomic_t g_running = 1;

static void onSigInt(int sig) {
	(void)sig;
	g_running = 0;
}

static void terminalEnableRaw(void) {
	// Save current settings
	tcgetattr(STDIN_FILENO, &g_oldTerm);
	struct termios raw = g_oldTerm;
	raw.c_lflag &= (tcflag_t)~(ECHO | ICANON);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &raw);

	// Make stdin non-blocking
	g_oldFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, g_oldFlags | O_NONBLOCK);
}

static void terminalDisableRaw(void) {
	// Restore terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &g_oldTerm);
	if (g_oldFlags != -1) {
		fcntl(STDIN_FILENO, F_SETFL, g_oldFlags);
	}
	// Show cursor
	printf("\033[?25h\n");
	fflush(stdout);
}

static int readKeyNonBlocking(void) {
	unsigned char c;
	ssize_t n = read(STDIN_FILENO, &c, 1);
	if (n == 0) return -1;
	if (n < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) return -1;
		return -1;
	}
	if (c != 27) return (int)c; // not ESC

	// Try to parse arrow keys: ESC [ D / C
	unsigned char seq[2];
	ssize_t n1 = read(STDIN_FILENO, &seq[0], 1);
	ssize_t n2 = read(STDIN_FILENO, &seq[1], 1);
	if (n1 == 1 && n2 == 1 && seq[0] == '[') {
		if (seq[1] == 'D') return 1000; // LEFT
		if (seq[1] == 'C') return 1001; // RIGHT
	}
	return -1;
}

void vykresliSnapshot(const SNAPSHOT* s) {
	// Clear screen, move cursor home, hide cursor
	printf("\033[2J\033[H\033[?25l");
	printf("Hadik | hrac=%d | stavHry=%d | mod=%d | svet=%d | cas=%.1fs\n",
	       s->indexHraca, (int)s->stav, (int)s->mod, (int)s->svet, s->hernyCasMs / 1000.0);
	printf("H0: skore=%d cas=%.1fs stav=%d   |   H1: skore=%d cas=%.1fs stav=%d\n",
	       s->skore[0], s->casVhre[0] / 1000.0, (int)s->stavHraca[0],
	       s->skore[1], s->casVhre[1] / 1000.0, (int)s->stavHraca[1]);
	printf("Ovl: s=pripoj p=pauza c=pokracuj l=odid q=quit | sipky L/R alebo a/d = zaboc\n\n");

	// top border
	putchar('+');
	for (int x = 0; x < SIRKA_PLOCHY; ++x) putchar('-');
	putchar('+');
	putchar('\n');

	for (int y = 0; y < VYSKA_PLOCHY; ++y) {
		putchar('|');
		for (int x = 0; x < SIRKA_PLOCHY; ++x) {
			putchar(s->buf[y][x]);
		}
		putchar('|');
		putchar('\n');
	}

	// bottom border
	putchar('+');
	for (int x = 0; x < SIRKA_PLOCHY; ++x) putchar('-');
	putchar('+');
	putchar('\n');

	if (s->stav == MENU) {
		printf("\nHra je v MENU (server este nebezi alebo nebola spustena).\n");
	} else if (s->stav == SKONCILA) {
		printf("\nHra SKONCILA. (Klient moze len citat stav)\n");
	}

	fflush(stdout);
}

void spracujVstup(HRA* hra, int indexHraca, int ch) {
	if (ch == 1000 || ch == 'a') {
		posliAkciu(hra, indexHraca, ZABOC_DOLAVA);
	} else if (ch == 1001 || ch == 'd') {
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
	while (g_running) {
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
		perror("klientOtvorenie");
		return 254;
	}


	HRA* hra = shm.hra;

	if (indexHraca < 0) {
		indexHraca = vyberHraca(hra);
		if (indexHraca < 0) indexHraca = 0;
	}
	if (indexHraca >= MAX_POCET_HRACOV) indexHraca = MAX_POCET_HRACOV - 1;

	// terminal init
	signal(SIGINT, onSigInt);
	terminalEnableRaw();

	pthread_t renderTh;
	RENDER_ARG arg;
	arg.hra = hra;
	arg.indexHraca = indexHraca;

	if (pthread_create(&renderTh, NULL, renderVlakno, &arg) != 0) {
		terminalDisableRaw();
		zatvorSHM(&shm);
		fprintf(stderr, "Nepodarilo sa spustit render thread.\n");
		return 1;
	}

	while (g_running) {
		int ch = readKeyNonBlocking();
		if (ch == 'q') {
			g_running = 0;
			posliAkciu(hra, indexHraca, OPUSTENIE_HRY);
			break;
		}
		if (ch != -1) {
			spracujVstup(hra, indexHraca, ch);
		}
		usleep(5 * 1000);
	}

	pthread_mutex_lock(&hra->mutex);
	pthread_cond_broadcast(&hra->signal);
	pthread_mutex_unlock(&hra->mutex);

	pthread_join(renderTh, NULL);

	terminalDisableRaw();
	zatvorSHM(&shm);
	return 0;
}

