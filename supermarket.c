
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include "common.h"



#include <dirent.h>
#include <ctype.h>
#include <string.h>

#define SHM_SIZE sizeof(int) + sizeof(pthread_mutex_t) // pamięć dla int + mutex

int msgid; //id kolejki komunikatow
void *shm_addr; //wskaznik do przylaczonego segmentu pamieci dzielonej
int shmid;

// Funkcja obsługi sygnału SIGUSR1
void handle_sigusr1(int sig) {
	printf("POZAR!!! Zamkniecie sklepu!!!\n");
	
	// Oczekiwanie na zakończenie procesów
    for (int i = 0; i < MAX_CUSTOMERS + 2; i++) {
        wait(NULL);
    }
	
    // Usunięcie kolejki komunikatów
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl (usuniecie kolejki)");
        exit(1);
    }
	
	// Odłączenie segmentu pamięci dzielonej
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(1);
    }
	
	// Usuwanie pamięci dzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        exit(1);
    }
	
	printf("\nSygnał SIGUSR1 odebrany! Proces zakończony.\n");
    exit(0);  // Zakończenie programu
}

// Funkcja obsługi sygnału SIGINT
void handle_sigint(int sig) {
	
	// Oczekiwanie na zakończenie procesów
    for (int i = 0; i < MAX_CUSTOMERS + 2; i++) {
        wait(NULL);
    }
	
    // Usunięcie kolejki komunikatów
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl (usuniecie kolejki)");
        exit(1);
    }
	
	// Odłączenie segmentu pamięci dzielonej
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(1);
    }
	
	// Usuwanie pamięci dzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        exit(1);
    }
	
	printf("\nSygnał SIGINT odebrany! Proces zakończony.\n");
    exit(0);  // Zakończenie programu
}

int main() {

	srand(time(NULL));
	
	// Rejestracja funkcji obsługi sygnału SIGINT
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("Nie udało się zarejestrować obsługi sygnału");
        exit(1);
    }
	// Rejestracja funkcji obsługi sygnału SIGUSR1
    if (signal(SIGUSR1, handle_sigusr1) == SIG_ERR) {
        perror("Nie udało się zarejestrować obsługi sygnału");
        exit(1);
    }
	
	
    // Inicjalizacja kolejki komunikatów
    msgid = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }
	
	// Tworzenie pamięci dzielonej
	key_t key = ftok("shmfile", 'A');  // Tworzenie unikalnego klucza
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0600);

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

    // Inicjalizacja pamięci dzielonej
    int *shm_ptr = (int *)shm_addr;  // wskaźnik na zmienną int
    pthread_mutex_t *mutex = (pthread_mutex_t *)(shm_ptr + 1);  // wskaźnik na mutex

    // Inicjalizacja mutexa w pamięci dzielonej
    if (pthread_mutex_init(mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(1);
    }

    // Ustawienie początkowej wartości w pamięci dzielonej
    *shm_ptr = 0;
	

    // Tworzenie procesu kierownika
    pid_t manager_pid = fork();
    if (manager_pid == 0) {
        char msgid_str[20];
        snprintf(msgid_str, 20, "%d", msgid);
        execl("./manager", "manager", msgid_str, NULL);
        perror("execl manager");
        exit(1);
    }

    // Tworzenie procesu strażaka
    pid_t firefighter_pid = fork();
    if (firefighter_pid == 0) {
        char msgid_str[20];
        snprintf(msgid_str, 20, "%d", msgid);
        execl("./firefighter", "firefighter", msgid_str, NULL);
        perror("execl firefighter");
        exit(1);
    }

    // Tworzenie procesów klientów
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pid_t customer_pid = fork();
        if (customer_pid == 0) {
            char customer_id[10];
            char msgid_str[20];
            snprintf(customer_id, 10, "%d", i);
            snprintf(msgid_str, 20, "%d", msgid);
            execl("./customer", "customer", customer_id, msgid_str, NULL);
            perror("execl customer");
            exit(1);
        }
        sleep(rand() % 3 + 1);  // Opóźnienie między tworzeniem klientów (miedzy 1 a 5 sekund)
    }

    // Oczekiwanie na zakończenie procesów
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        wait(NULL);
    }

    // Usunięcie kolejki komunikatów
    msgctl(msgid, IPC_RMID, NULL);
	
	
	// Odłączenie segmentu pamięci dzielonej
    if (shmdt(shm_addr) == -1) {
        perror("shmdt");
        exit(1);
    }
	
	// Usuwanie pamięci dzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        exit(1);
    }

   printf("Supermarket zamknięty.\n");
	
	
	if (kill(firefighter_pid, SIGTERM) == 0) {  // Wysyłanie sygnału SIGTERM
		printf("Wysłano SIGTERM do procesu potomnego o PID: %d\n", firefighter_pid);
	} else {
		perror("Błąd podczas wysyłania sygnału");
	}
		
	if (kill(manager_pid, SIGTERM) == 0) {  // Wysyłanie sygnału SIGTERM
		printf("Wysłano SIGTERM do procesu potomnego o PID: %d\n", manager_pid);
	} else {
		perror("Błąd podczas wysyłania sygnału");
	}

    return 0;
}
