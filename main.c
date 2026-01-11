
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "klient.h"
#include "server.h"

int main(int argc, char **argv) {
	if(argc >= 2 && strcmp(argv[1], "server") == 0) {
		_Bool novaInicializacia = false;
		TYPY_SVETOV typSveta = BEZ_PREKAZOK;
		if(argc >= 3 && strcmp(argv[2], "nova") == 0) {
			novaInicializacia = true;
		}
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "prekazky") == 0 || strcmp(argv[i], "s_prekazkami") == 0) {
				typSveta = S_PREKAZKAMI;
			} else if (strcmp(argv[i], "bez") == 0 || strcmp(argv[i], "bez_prekazok") == 0) {
				typSveta = BEZ_PREKAZOK;
			}
		}
		return spustiServer(novaInicializacia, typSveta);
	}
	if(argc >= 3 && strcmp(argv[1], "klient") == 0) {
		int hrac = atoi(argv[2]);
		return spustiKlienta(hrac);
	}
	return 0;
}
