
#include "common.h"

pthread_mutex_t *mutex;
void *shm_addr;
int customer_id;


int main(int argc, char *argv[]) {
	srand(time(NULL));
	
	// Tworzenie klucza
    key_t key = ftok("shmfile", 'A');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Uzyskanie identyfikatora segmentu pamięci dzielonej
    int shmid = shmget(key, sizeof(int) + sizeof(pthread_mutex_t), 0600);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
	
    // Przyłączanie segmentu pamięci dzielonej
    shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (void *)-1) {
        perror("shmat");
        exit(1);
    }
	// Wskaźnik do zmiennej w pamięci dzielonej
    int *shm_ptr = (int *)shm_addr;
    pthread_mutex_t *mutex = (pthread_mutex_t *)(shm_ptr + 1);  // Mutex po int
	
	// Inicjalizacja mutexa w pamięci dzielonej
    if (pthread_mutex_init(mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }
	
	// Zablokowanie mutexa
    if (pthread_mutex_lock(mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(1);
    }
	
	*shm_ptr = *shm_ptr + 1;
	
	// Zwolnienie mutexa
    if (pthread_mutex_unlock(mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(1);
    }

    if (argc < 3) {
        fprintf(stderr, "Użycie: %s <customer_id> <msgid>\n", argv[0]);
        exit(1);
    }
    customer_id = atoi(argv[1]);
    int msgid = atoi(argv[2]);  // Pobierz identyfikator kolejki
	
	
    printf("Klient %d wszedł do sklepu.\n", customer_id);

    // Symulacja czasu spędzonego w sklepie (miedzy 1 a 20 sekund)
    sleep(rand() % 20 + 1);

    // Wysłanie komunikatu do kierownika (ustawienie się w kolejce)
    message_t msg;
    msg.mtype = 1;  // Typ komunikatu: ustawienie w kolejce
    msg.customer_id = customer_id;
    if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }

    // Oczekiwanie na odpowiedź od kierownika (przydział do kasy)
    message_t response;
    if (msgrcv(msgid, &response, sizeof(response), customer_id + 3, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }

	if (response.cashier_id == -1) {
		printf("Klient %d opuscil sklep\n", customer_id);
	}
	else {
		printf("Klient %d jest obslugiwany przy kasie %d.\n", customer_id, response.cashier_id);

		// Symulacja obsługi przy kasie
		sleep(rand() % 10 + 1);  // Czas obsługi (miedzy 1 a 10 sekund)

		// Wysłanie komunikatu do kierownika (opuszczenie kasy)
		msg.mtype = 2;  // Typ komunikatu: opuszczenie kasy
		msg.customer_id = customer_id;
		msg.cashier_id = response.cashier_id;
		if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {
			perror("msgsnd");
			exit(1);
		}
		printf("Klient %d opuścił sklep po obsłudze przy kasie %d.\n", customer_id, response.cashier_id);
	}

	// Zmniejszanie liczby klientow w sklepie
	// Zablokowanie mutexa
    if (pthread_mutex_lock(mutex) != 0) {
        perror("pthread_mutex_lock");
        exit(1);
    }
	
	*shm_ptr = *shm_ptr - 1;
	
	// Zwolnienie mutexa
    if (pthread_mutex_unlock(mutex) != 0) {
        perror("pthread_mutex_unlock");
        exit(1);
    }


    // Odłączenie segmentu pamięci dzielonej
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(1);
    }
	
    return 0;
}
