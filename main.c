
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "klient.h"
#include "server.h"

int main(int argc, char **argv) {
	if(argc >= 2 && strcmp(argv[1], "server") == 0) {
		_Bool novaInicializacia = false;
		if(argc >= 3 && strcmp(argv[2], "nova") == 0) {
			novaInicializacia = true;
		}
		return spustiServer(novaInicializacia);
	}
	if(argc >= 3 && strcmp(argv[1], "klient") == 0) {
		int hrac = atoi(argv[2]);
		return spustiKlienta(hrac);
	}
    return 0;
}
