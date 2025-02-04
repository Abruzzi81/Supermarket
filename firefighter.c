#include "common.h"
#include <signal.h>

int main(int argc, char *argv[]) {
    srand(time(NULL));
	
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <msgid>\n", argv[0]);
        exit(1);
    }

    int msgid = atoi(argv[1]);  // Pobierz identyfikator kolejki

    // Symulacja wykrycia pożaru
    sleep(5);  // Pożar w losowym czasie

    // Wysłanie sygnału do wszystkich procesów
    kill(0, SIGUSR1);  // Wysyłanie sygnału do grupy procesów

    return 0;
}
