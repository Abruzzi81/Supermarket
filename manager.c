#include "common.h"
#include <signal.h>

volatile sig_atomic_t fire_alarm = 0;  // Flaga sygnału pożaru
int msgid;  // Pobierz identyfikator kolejki


typedef struct {
    int id; // ID kasy
    int liczba_klientow; // Liczba klientów w kolejce
    int kolejka[MAX_QUEUE_SIZE]; // Kolejka klientów
    pthread_mutex_t mutex; // Mutex dla kolejki
    pthread_cond_t cond; // Zmienna warunkowa dla klientów
    int czynna; // Flaga oznaczająca, czy kasa jest czynna
	int zajeta; // Flaga oznaczajaca czy kasa jest zajeta
} Kasa;

Kasa kasy[MAX_CASHIERS];

// Funkcja obsługi sygnału SIGUSR1
void handle_sigusr1(int sig) {
    fire_alarm = 1;  // Ustaw flagę pożaru
}

void* cashier_thread(void *arg) {
	Kasa *kasa = (Kasa *)arg;

    while (1) {
        pthread_mutex_lock(&kasa->mutex);

        // Czekaj na klienta, jeśli kolejka jest pusta
        while (kasa->liczba_klientow == 0) {
            pthread_cond_wait(&kasa->cond, &kasa->mutex);
        }

        // Obsługa pierwszego klienta w kolejce
		kasa->zajeta = 1;
        int klient_id = kasa->kolejka[0];
        for (int i = 0; i < kasa->liczba_klientow - 1; i++) {
            kasa->kolejka[i] = kasa->kolejka[i + 1];
        }
        kasa->liczba_klientow--;
        pthread_mutex_unlock(&kasa->mutex);
		
        // Wysłanie odpowiedzi do klienta
		message_t response;
        response.mtype = (long)klient_id + 1;  // Unikalny typ komunikatu dla klienta
        response.customer_id = klient_id;
        response.cashier_id = kasa->id + 1;  // Numeracja kas od 1
        if (msgsnd(msgid, &response, sizeof(response), 0) == -1) {
			perror("msgsnd");
			exit(1);
        }
        
		while(kasa->zajeta) {
			sleep(1);
		}

    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Użycie: %s <msgid>\n", argv[0]);
        exit(1);
    }

	msgid = atoi(argv[1]);
    int active_cashiers = 0;    // Zawsze min. 2 kasy
    int customer_count = 0;
    int cashier_queues[MAX_CASHIERS] = {0};  // Liczba klientów w kolejce do każdej kasy

	pthread_t watek_kasjerow[MAX_CASHIERS];
	
    // Inicjalizacja kas
    for (int i = 0; i < MAX_CASHIERS; i++) {
        kasy[i].id = i;
        kasy[i].liczba_klientow = 0;
        kasy[i].czynna = (i < 2); // Na początku działają 2 kasy
        pthread_mutex_init(&kasy[i].mutex, NULL);
        pthread_cond_init(&kasy[i].cond, NULL);
		kasy[i].zajeta = 0;
        if (i < 2) active_cashiers++;
        pthread_create(&watek_kasjerow[i], NULL, cashier_thread, &kasy[i]);
    }


    // Tworzenie klucza dla pamięci dzielonej
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
    void *shm_addr = shmat(shmid, NULL, 0);
    if (shm_addr == (void *)-1) {
        perror("shmat");
        exit(1);
    }	
	int *shm_ptr = (int *)shm_addr;  // wskaźnik na zmienną int
	
    // Rejestracja obsługi sygnału SIGUSR1
    signal(SIGUSR1, handle_sigusr1);

    while (!fire_alarm) {  // Działaj, dopóki nie ma pożaru
		//aktualizacja liczby klientow
		customer_count = *shm_ptr;
        // Odbieranie komunikatów od klientów
        message_t msg;

      if (msgrcv(msgid, &msg, sizeof(msg), 0, IPC_NOWAIT) != -1) {

            if (msg.mtype == 1) {  // Klient ustawia się w kolejce

                // Przydzielenie klienta do kasy z najmniejszą kolejką
                int selected_cashier = 0;
                for (int i = 0; i < active_cashiers; i++) {
                    if (kasy[i].liczba_klientow < kasy[selected_cashier].liczba_klientow) {
                        selected_cashier = i;
                    }
                }
				//	Wysylanie informacji o braku miejsca w kolejce
				if (kasy[selected_cashier].liczba_klientow >= MAX_QUEUE_SIZE ) {
					// Wysłanie odpowiedzi do klienta
					message_t response;
					response.mtype = (long)(msg.customer_id + 1);  // Unikalny typ komunikatu dla klienta
					response.customer_id = msg.customer_id;
					response.cashier_id = -1;  // Brak miejsca w kolejce
					if (msgsnd(msgid, &response, sizeof(response), 0) == -1) {
						perror("msgsnd");
						exit(1);
					}
				}
				else {
					pthread_mutex_lock(&kasy[selected_cashier].mutex);
					kasy[selected_cashier].liczba_klientow++;
					kasy[selected_cashier].kolejka[kasy[selected_cashier].liczba_klientow-1] = msg.customer_id;
					pthread_cond_signal(&kasy[selected_cashier].cond); // Powiadom kasjera
					pthread_mutex_unlock(&kasy[selected_cashier].mutex);
				
					printf("Klient %d ustawil sie w kolejce do kasy %d\n", msg.customer_id, selected_cashier + 1);		
				}

			} else if (msg.mtype == 2) {  // Klient opuszcza kasę
                int cashier_id = msg.cashier_id - 1;  // Numeracja kas od 0
                kasy[cashier_id].zajeta = 0;
            }
        }

        // Zarządzanie kasami
        int required_cashiers = customer_count / K + 1;
        if (required_cashiers < 2) required_cashiers = 2;  // Zawsze min. 2 kasy
		else if (required_cashiers > MAX_CASHIERS) required_cashiers = MAX_CASHIERS;

        if (required_cashiers > active_cashiers) {
            // Otwarcie nowej kasy
            active_cashiers++;
            printf("Nowa kasa otwarta. Aktywne kasy: %d\n", active_cashiers);
        } else if (required_cashiers < active_cashiers && customer_count < K * (active_cashiers - 1)) {
            // Zamknięcie kasy
            active_cashiers--;
           printf("Kasa zamknięta. Aktywne kasy: %d\n", active_cashiers);
        }

    }

    // Pożar: zamknięcie kas i zakończenie działania
    printf("POŻAR! Zamykanie kas...\n");
    active_cashiers = 0;
    printf("Wszystkie kasy zamknięte. Kierownik kończy działanie.\n");
	
	
	// Odłączenie segmentu pamięci dzielonej
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}
