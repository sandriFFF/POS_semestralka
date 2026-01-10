
#include <stdbool.h>
#include <stdlib.h>

#include "klient.h"
#include "server.h"

int main(int argc, char **argv) {
	if(argc >= 2 && argv[1] == "server") {
		_Bool novaInicializacia = false;
		if(argc >= 3 && argv[2] == "nova") {
			novaInicializacia = true;
		}
		return spustiServer(novaInicializacia);
	}
	if(argc >= 3 && argv[1] == "klient") {
		int hrac = atoi(argv[2]);
		return spustiKlienta(hrac);
	}
    return 0;
}
