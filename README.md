# Supermarket

## Opis projektu

W pewnym supermarkecie jest łącznie 10 kas. Zasady ich działania przyjęte przez kierownika sklepu 
są następujące:  

• Zawsze działają min. 2 stanowiska kasowe.  
• Na każdych K klientów znajdujących się na terenie supermarketu powinno przypadać min. 1 
czynne stanowisko kasowe.  
• Jeśli liczba klientów jest mniejsza niż K*(N-1), gdzie N oznacza liczbę czynnych kas, to jedna 
z kas zostaje zamknięta.  
• Jeśli w kolejce do kasy czekali klienci (przed ogłoszeniem decyzji o jej zamknięciu) to powinni 
zostać obsłużeni przez tę kasę.  
Klienci przychodzą do supermarketu w losowych momentach czasu i przebywają w nim przez pewien 
określony losowy dla każdego z nich czas. Na sygnał o pożarze – sygnał wysyła strażak - klienci 
natychmiast opuszczają supermarket bez robienia zakupów, a następnie po wyjściu klientów 
zamykane są wszystkie kasy.

## Struktura programu

W projekcie zastosowane zostały 3 główne komponenty:

1. **Strażak:** Wysyła sygnał o pożarze, który powoduje natychmiastowe opuszczenie supermarketu przez wszystkich klientów.
2. **Klienci:** Przybywają do supermarketu w losowych momentach, spędzają w nim losowy czas i dokonują zakupów.
3. **Kierownik kas:** Zarządza kasami, otwierając i zamykając je zgodnie z zasadami opisanymi powyżej, dbając o odpowiednią liczbę kas w zależności od liczby klientów.


## Użyte funkcje systemowe

1. **`pthread_create`**

   Funkcja `pthread_create` jest używana do tworzenia nowych wątków. Jest to funkcja z biblioteki POSIX, która umożliwia tworzenie wątków w aplikacjach wielowątkowych.

   **Zastosowanie w projekcie:**
   Tworzenie wątków dla każdego klienta:
   ```c
   pthread_create(&klient_watek, NULL, klient, nowy_klient);

2. **`pthread_detach`**
  Funkcja `pthread_detach` jest używana do oddzielania wątku, tak aby po jego zakończeniu zasoby były automatycznie zwolnione. Wątek, który jest odłączony (detached), nie wymaga wywołania pthread_join.

    **Zastosowanie w projekcie:**
      Klient jest tworzony w wątku, który jest odłączany, aby nie trzeba było czekać na jego zakończenie (pthread_detach(klient_watek);)
      ```c
      pthread_detach(klient_watek);


3. **`pthread_mutex_lock**` i **`pthread_mutex_unlock`**
  Funkcje pthread_mutex_lock i pthread_mutex_unlock służą do blokowania i odblokowywania mutexów. Mutexy są używane do synchronizacji wątków, zapobiegając jednoczesnemu dostępowi do tych samych zasobów przez różne wątki.

  **Zastosowanie w projekcie:**
  Synchronizacja dostępu do globalnych zmiennych, takich jak liczba_klientow_w_sklepie, pożar, a także do struktury kas, aby zapewnić bezpieczeństwo wątków przy dodawaniu klientów do kolejki i obsłudze ich przez kasjera.
    ```c
    pthread_mutex_lock(&kasy[i].mutex);
    ```

  4. **`pthread_cond_wait`** i **`pthread_cond_signal`**
  Funkcje pthread_cond_wait i pthread_cond_signal są używane do synchronizacji wątków za pomocą zmiennych warunkowych. Zmienna warunkowa pozwala na wstrzymanie wątku do momentu, gdy inny wątek wyśle sygnał, że może on kontynuować działanie.

  **Zastosowanie w projekcie:**
  W kasach, wątki kasjerów czekają na klientów, a kiedy klient się pojawi, wysyłany jest sygnał za pomocą pthread_cond_signal, aby kasjer mógł zacząć obsługiwać klienta:
```c
  pthread_cond_wait(&kasa->cond, &kasa->mutex);
  pthread_cond_signal(&kasa->cond);
```
  5. sleep
  Funkcja sleep powoduje, że wątek zostaje wstrzymany na określoną liczbę sekund. Jest to przydatne do symulacji opóźnień, takich jak czas oczekiwania klientów, czas obsługi przez kasjera, czy symulacja czasu, w którym klienci chodzą po sklepie.

      **Zastosowanie w projekcie:**

      Symulacja czasu oczekiwania klientów:
```c
sleep(rand() % 10 + 1);
```
  Symulacja czasu obsługi klienta przez kasjera:
```c
sleep(klient->czas_obslugi);
```



## Działanie projektu

1. **Struktury**
   
   Klient
   ```c
   typedef struct {
    int id; // ID klienta
    int czas_obslugi; // Czas obsługi klienta w sekundach
   } Klient;
   ```
   Kasa
   ```c
   typedef struct {
    int id; // ID kasy
    int liczba_klientow; // Liczba klientów w kolejce
    Klient *kolejka[MAX_KLIENTOW]; // Kolejka klientów
    pthread_mutex_t mutex; // Mutex dla kolejki
    pthread_cond_t cond; // Zmienna warunkowa dla klientów
    int czynna; // Flaga oznaczająca, czy kasa jest czynna
   } Kasa;
   ```


   
2. **Zasada działania funkcji w projekcie**

   **klient:**
   
   1.Zwiększenie liczby klientow w sklepie
   ```c
   pthread_mutex_lock(&mutex_klienci);
    liczba_klientow_w_sklepie++;
    pthread_mutex_unlock(&mutex_klienci);
   ```

   2.Klient chodzi przez pewien losowy czas po sklepie
   ```c
   sleep(rand() % 10 + 1); //losowy czas między 1 a 10 sekund
   ```

   3.Klient szuka dostępnej kasy z najkrótszą kolejką

   ```c
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
   ```

   4.Klient ustawia się w kolejce do wybranej kasy

   ```c
    pthread_mutex_lock(&kasy[wybrana_kasa].mutex);
    kasy[wybrana_kasa].kolejka[kasy[wybrana_kasa].liczba_klientow++] = klient;
    pthread_cond_signal(&kasy[wybrana_kasa].cond); // Powiadom kasjera
    pthread_mutex_unlock(&kasy[wybrana_kasa].mutex);
   ```


   **kasjer**

   1.Kasjer czeka na sygnał o pojawieniu się klienta (ewentualnie o pożarze)

   ```c
   while (!pożar && kasa->liczba_klientow == 0) {
      pthread_cond_wait(&kasa->cond, &kasa->mutex);
   }
   ```

   2.Kasjer obsługuje pierwszego klienta w kolejce

   ```c
   Klient *klient = kasa->kolejka[0];
        for (int i = 0; i < kasa->liczba_klientow - 1; i++) {
            kasa->kolejka[i] = kasa->kolejka[i + 1];
        }
        kasa->liczba_klientow--;

        pthread_mutex_unlock(&kasa->mutex);

        sleep(klient->czas_obslugi); //Symulacja obsługi
   ```

   3.Kasjer po bosłużeniu klienta aktualizuje liczbe kliwntow w sklepie i zwalnia pamięć po kliencie

   ```c
   liczba_klientow_w_sklepie--;
   free(klient);
   ```

   **kierownik**
   
   Kierownik działa w pętli nieskończonej, przerywanej tylko pożarem, 

   1.Oblicza liczbe potrzebnych kas (zawsze conajmniej dwie kasy otwarte)

   ```c
   int wymagana_liczba_kas = (liczba_klientow_w_sklepie + KLIENTOW_NA_KASE - 1) / KLIENTOW_NA_KASE;
   if (wymagana_liczba_kas < 2) wymagana_liczba_kas = 2;
   ```


   2.W razie potrzeby zmienia liczbę uruchomionych kas

   ```c
    if (wymagana_liczba_kas != poprzednia_liczba_kas) {
            printf("Kierownik: zmiana liczby otwartych kas na %d.\n", wymagana_liczba_kas);

            // Otwieranie dodatkowych kas
            for (int i = 0; i < MAX_KASY && liczba_otwartych_kas < wymagana_liczba_kas; i++) {
                if (!kasy[i].czynna) {
                    kasy[i].czynna = 1;
                    liczba_otwartych_kas++;
                }
            }

            // Zamykanie nadmiarowych kas
            for (int i = MAX_KASY - 1; i >= 0 && liczba_otwartych_kas > wymagana_liczba_kas; i--) {
                pthread_mutex_lock(&kasy[i].mutex);
                if (kasy[i].czynna && kasy[i].liczba_klientow == 0) {
                    kasy[i].czynna = 0;
                    liczba_otwartych_kas--;
                }
                pthread_mutex_unlock(&kasy[i].mutex);
            }
            poprzednia_liczba_kas = wymagana_liczba_kas;
        }
   ```

   **strazak**

   1.Czeka losowy czas do uruchomienia alarmu o pożarze

   ```c
   sleep(rand() % 101 + 20); //losowy czas między 100 a 120 sekund
   ```

   2.Ustawia zmienną informującą o pożarze

   ```c
    pthread_mutex_lock(&mutex_pożar);
    pożar = 1;
    pthread_mutex_unlock(&mutex_pożar);
   ```

   3.Powiadamia kasy o pożarze

   ```c
   for (int i = 0; i < MAX_KASY; i++) {
      pthread_mutex_lock(&kasy[i].mutex);
      pthread_cond_broadcast(&kasy[i].cond); // Powiadom wszystkie kasy
      pthread_mutex_unlock(&kasy[i].mutex);
    }
   ```


   
3. **Funkcja main**

   1.Inicjalizuje kasy

   ```c
   for (int i = 0; i < MAX_KASY; i++) {
        kasy[i].id = i;
        kasy[i].liczba_klientow = 0;
        kasy[i].czynna = (i < 2); // Na początku działają 2 kasy
        pthread_mutex_init(&kasy[i].mutex, NULL);
        pthread_cond_init(&kasy[i].cond, NULL);
        if (i < 2) liczba_otwartych_kas++;
        pthread_create(&watek_kasjerow[i], NULL, kasjer, &kasy[i]);
    }
   ```

   2.Utworzenie wątku strażaka i kierownika

   ```c
   pthread_create(&watek_strazak, NULL, strazak, NULL);
   pthread_create(&watek_kierownik, NULL, kierownik, NULL);
   ```

   3.Generowanie klientów co 1-3 sekundy (przerwanie gdy wybuchnie pożar)

   ```c
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
   ```

   4.Oczekiwanie na zamknięcie wszystkich wątków

   ```c
    pthread_join(watek_strazak, NULL);
    for (int i = 0; i < MAX_KASY; i++) {
        pthread_join(watek_kasjerow[i], NULL);
    }
    pthread_join(watek_kierownik, NULL);
   ```
   
