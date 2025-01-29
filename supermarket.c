#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_KLIENTOW 100
#define MAX_KASY 10
#define KLIENTOW_NA_KASE 5 // K - liczba klientów przypadająca na kasę

typedef struct {
    int id; // ID klienta
    int czas_obslugi; // Czas obsługi klienta w sekundach
} Klient;

typedef struct {
    int id; // ID kasy
    int liczba_klientow; // Liczba klientów w kolejce
    Klient *kolejka[MAX_KLIENTOW]; // Kolejka klientów
    pthread_mutex_t mutex; // Mutex dla kolejki
    pthread_cond_t cond; // Zmienna warunkowa dla klientów
    int czynna; // Flaga oznaczająca, czy kasa jest czynna
} Kasa;

Kasa kasy[MAX_KASY];
int liczba_klientow_w_sklepie = 0; // Liczba klientów w sklepie
int pożar = 0; // Flaga oznaczająca, czy jest pożar
int liczba_otwartych_kas = 0; // Globalna liczba otwartych kas (zawsze >= 2)

pthread_mutex_t mutex_pożar = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_klienci = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_kasy = PTHREAD_MUTEX_INITIALIZER;

// Funkcja obsługi klienta
void *klient(void *arg) {
    Klient *klient = (Klient *)arg;

    pthread_mutex_lock(&mutex_klienci);
    liczba_klientow_w_sklepie++;
    pthread_mutex_unlock(&mutex_klienci);

    printf("Klient %d wchodzi do sklepu.\n", klient->id);
	//Klient chodzi po sklepie
	sleep(rand() % 10 + 1);
	
    // Klient wybiera kasę z najmniejszą liczbą klientów
    int wybrana_kasa = -1;
    while (!pożar && wybrana_kasa == -1) {
        int min_klientow = MAX_KLIENTOW + 1;
        for (int i = 0; i < MAX_KASY; i++) {
            pthread_mutex_lock(&kasy[i].mutex);
            if (kasy[i].czynna && kasy[i].liczba_klientow < min_klientow) {
                wybrana_kasa = i;
                min_klientow = kasy[i].liczba_klientow;
            }
            pthread_mutex_unlock(&kasy[i].mutex);
        }
        if (wybrana_kasa == -1) {
            sleep(1); // Czekaj i spróbuj ponownie
        }
    }

    if (pożar) {
        printf("Klient %d opuszcza sklep z powodu pożaru.\n", klient->id);
        pthread_mutex_lock(&mutex_klienci);
        liczba_klientow_w_sklepie--;
        pthread_mutex_unlock(&mutex_klienci);
        free(klient);
        return NULL;
    }

    // Dodanie klienta do kolejki wybranej kasy
    pthread_mutex_lock(&kasy[wybrana_kasa].mutex);
    kasy[wybrana_kasa].kolejka[kasy[wybrana_kasa].liczba_klientow++] = klient;
    printf("Klient %d ustawia się w kolejce do kasy %d.\n", klient->id, wybrana_kasa);
    pthread_cond_signal(&kasy[wybrana_kasa].cond); // Powiadom kasjera
    pthread_mutex_unlock(&kasy[wybrana_kasa].mutex);

    return NULL;
}

// Funkcja obsługi kasy
void *kasjer(void *arg) {
    Kasa *kasa = (Kasa *)arg;

    while (1) {
        pthread_mutex_lock(&kasa->mutex);

        // Sprawdzaj, czy jest pożar
        if (pożar) {
            printf("Kasa %d zamyka się z powodu pożaru.\n", kasa->id);
            kasa->czynna = 0;
            pthread_mutex_unlock(&kasa->mutex);
            return NULL;
        }

        // Czekaj na klienta, jeśli kolejka jest pusta
        while (!pożar && kasa->liczba_klientow == 0) {
            pthread_cond_wait(&kasa->cond, &kasa->mutex);
        }

        if (pożar) {
            printf("Kasa %d zamyka się z powodu pożaru.\n", kasa->id);
            kasa->czynna = 0;
            pthread_mutex_unlock(&kasa->mutex);
            return NULL;
        }

        // Obsługa pierwszego klienta w kolejce
        Klient *klient = kasa->kolejka[0];
        for (int i = 0; i < kasa->liczba_klientow - 1; i++) {
            kasa->kolejka[i] = kasa->kolejka[i + 1];
        }
        kasa->liczba_klientow--;

        pthread_mutex_unlock(&kasa->mutex);

        printf("Kasa %d obsługuje klienta %d przez %d sekund.\n", kasa->id, klient->id, klient->czas_obslugi);
        sleep(klient->czas_obslugi); // Symulacja czasu obsługi klienta
        printf("Kasa %d zakończyła obsługę klienta %d.\n", kasa->id, klient->id);
		liczba_klientow_w_sklepie--;
        free(klient);
    }

    return NULL;
}

// Funkcja kierownika
void *kierownik(void *arg) {
    int poprzednia_liczba_kas = liczba_otwartych_kas;

    while (!pożar) {
        pthread_mutex_lock(&mutex_klienci);
        pthread_mutex_lock(&mutex_kasy);

        int wymagana_liczba_kas = (liczba_klientow_w_sklepie + KLIENTOW_NA_KASE - 1) / KLIENTOW_NA_KASE; // Zaokrąglanie w górę
        if (wymagana_liczba_kas < 2) wymagana_liczba_kas = 2;

        // Sprawdź, czy liczba kas musi się zmienić
        if (wymagana_liczba_kas != poprzednia_liczba_kas) {
            printf("Kierownik: zmiana liczby otwartych kas na %d.\n", wymagana_liczba_kas);

            // Otwieranie dodatkowych kas
            for (int i = 0; i < MAX_KASY && liczba_otwartych_kas < wymagana_liczba_kas; i++) {
                if (!kasy[i].czynna) {
                    kasy[i].czynna = 1;
                    liczba_otwartych_kas++;
                    printf("Kierownik: Otwieram kasę %d.\n", kasy[i].id);
                }
            }

            // Zamykanie nadmiarowych kas
            for (int i = MAX_KASY - 1; i >= 0 && liczba_otwartych_kas > wymagana_liczba_kas; i--) {
                pthread_mutex_lock(&kasy[i].mutex);
                if (kasy[i].czynna && kasy[i].liczba_klientow == 0) {
                    kasy[i].czynna = 0;
                    liczba_otwartych_kas--;
                    printf("Kierownik: Zamykam kasę %d.\n", kasy[i].id);
                }
                pthread_mutex_unlock(&kasy[i].mutex);
            }

            poprzednia_liczba_kas = wymagana_liczba_kas;
        }

        pthread_mutex_unlock(&mutex_kasy);
        pthread_mutex_unlock(&mutex_klienci);

        sleep(1); // Sprawdzanie co sekundę
    }
    return NULL;
}

// Funkcja strażaka
void *strazak(void *arg) {
    sleep(1000); // Pożar po 10 sekundach
    printf("Strażak: Sygnał o pożarze! Wszyscy klienci opuszczają sklep.\n");

    pthread_mutex_lock(&mutex_pożar);
    pożar = 1;
    pthread_mutex_unlock(&mutex_pożar);

    // Zamykanie wszystkich kas
    for (int i = 0; i < MAX_KASY; i++) {
        pthread_mutex_lock(&kasy[i].mutex);
        pthread_cond_broadcast(&kasy[i].cond); // Powiadom wszystkie kasy
        pthread_mutex_unlock(&kasy[i].mutex);
    }

    return NULL;
}

int main() {
    pthread_t watek_kasjerow[MAX_KASY];
    pthread_t watek_strazak, watek_kierownik;

    // Inicjalizacja kas
    for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].id = i;
        kasy[i].liczba_klientow = 0;
        kasy[i].czynna = (i < 2); // Na początku działają 2 kasy
        pthread_mutex_init(&kasy[i].mutex, NULL);
        pthread_cond_init(&kasy[i].cond, NULL);
        if (i < 2) liczba_otwartych_kas++;
        pthread_create(&watek_kasjerow[i], NULL, kasjer, &kasy[i]);
    }

    // Uruchomienie strażaka i kierownika
    pthread_create(&watek_strazak, NULL, strazak, NULL);
    pthread_create(&watek_kierownik, NULL, kierownik, NULL);

    // Generowanie klientów
    int id_klienta = 0;
    while (!pożar) {
        sleep(rand() % 3 + 1); // Nowy klient co 1-3 sekundy

        // Tworzenie nowego klienta
        Klient *nowy_klient = (Klient *)malloc(sizeof(Klient));
        nowy_klient->id = id_klienta++;
        nowy_klient->czas_obslugi = rand() % 10 + 1;

        // Tworzenie wątku dla klienta
        pthread_t klient_watek;
        pthread_create(&klient_watek, NULL, klient, nowy_klient);
        pthread_detach(klient_watek);
    }

    // Czekaj na zakończenie strażaka
    pthread_join(watek_strazak, NULL);

    // Oczekiwanie na zamknięcie wszystkich kas
    for (int i = 0; i < MAX_KASY; i++) {
        pthread_join(watek_kasjerow[i], NULL);
    }

    // Czekaj na zakończenie kierownika
    pthread_join(watek_kierownik, NULL);

    printf("Sklep został zamknięty.\n");
    return 0;
}
