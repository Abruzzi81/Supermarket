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

### 1. Tworzenie i obsługa procesów i wątków

**Procesy:** Program wykorzystuje procesy do symulacji klientów, kierownika i strażaka. Każdy klient, kierownik i strażak są tworzeni jako osobne procesy za pomocą funkcji `fork()`. Procesy te działają niezależnie od siebie.

**Wątki:** W programie kierownika (`manager.c`) wykorzystywane są wątki do obsługi kasjerów. Każda kasa ma swój własny wątek, który obsługuje klientów w kolejce. Wątki są tworzone za pomocą `pthread_create`.

### 2. Zastosowanie systemowych mechanizmów synchronizacji procesów i wątków

**Mutexy:** W programie użyto mutexów (`pthread_mutex_t`) do synchronizacji dostępu do współdzielonych zasobów, takich jak liczba klientów w sklepie oraz kolejki przy kasach. Mutexy są używane zarówno w pamięci dzielonej, jak i w strukturach kas.

**Zmienne warunkowe:** W programie kierownika (`manager.c`) zastosowano zmienne warunkowe (`pthread_cond_t`) do synchronizacji wątków kasjerów. Kasjerzy czekają na klientów, a gdy pojawi się klient w kolejce, są budzeni przez sygnał zmiennej warunkowej.

### 3. Zastosowanie mechanizmów komunikacji między procesami

**Kolejka komunikatów:** Do komunikacji między klientami a kierownikiem użyto kolejki komunikatów (`msgget`, `msgsnd`, `msgrcv`). Klienci wysyłają komunikaty do kierownika, aby poinformować go o ustawieniu się w kolejce lub opuszczeniu kasy.

**Pamięć dzielona:** Do przechowywania liczby klientów w sklepie użyto pamięci dzielonej (`shmget`, `shmat`). Pamięć dzielona jest również używana do przechowywania mutexu, który synchronizuje dostęp do tej liczby.

### 4. Obsługa sygnałów

**SIGUSR1:** Sygnał `SIGUSR1` jest używany przez strażaka do poinformowania wszystkich procesów o pożarze. Kierownik i klienci reagują na ten sygnał, kończąc swoje działanie.

**SIGINT:** Sygnał `SIGINT` jest używany do bezpiecznego zakończenia programu. Gdy użytkownik naciśnie `Ctrl+C`, program zamyka wszystkie procesy, usuwa kolejkę komunikatów i pamięć dzieloną.

### 5. Wyjątki/obsługa błędów

**Walidacja danych:** Program sprawdza poprawność argumentów przekazywanych do procesów klientów (`customer.c`) oraz kierownika (`manager.c`). Jeśli argumenty są nieprawidłowe, program kończy działanie z odpowiednim komunikatem błędu.

**Obsługa błędów:** Każda operacja systemowa (np. tworzenie pamięci dzielonej, wysyłanie komunikatów) jest sprawdzana pod kątem błędów. W przypadku błędu program wyświetla odpowiedni komunikat i kończy działanie.

### 6. Własne moduły

**Plik `common.h`:** Zawiera definicje stałych, struktur oraz deklaracje funkcji używanych w programie. Jest to wspólny plik nagłówkowy dla wszystkich modułów.

**Plik `customer.c`:** Zawiera logikę działania klientów. Klienci przychodzą do sklepu, ustawiają się w kolejce, są obsługiwani przez kasjerów i opuszczają sklep.

**Plik `manager.c`:** Zawiera logikę działania kierownika. Kierownik zarządza kasami, otwierając i zamykając je w zależności od liczby klientów.

**Plik `firefighter.c`:** Zawiera logikę działania strażaka. Strażak wykrywa pożar i wysyła sygnał do wszystkich procesów.

**Plik `supermarket.c`:** Główny plik programu, który tworzy procesy klientów, kierownika i strażaka oraz zarządza pamięcią dzieloną i kolejką komunikatów.


## Testy

### Sprawdzenie czasu działania programu dla różnej ilości klientów

Liczba klientów jest zmieniana za pomocą zmiany wartości `MAX_CUSTOMERS` w pliku `common.c`

• 10 klientów

   Czas trwania symulacji: 40.380885 s
   
• 100 klientów

   Czas trwania symulacji: 232.415921 s   =  3 min 52 s

• 1000 klientów

   Czas trwania symulacji: 2077.483760 s  =  34 min 37 s

### Test alarmu pożarowego

Czas alermu został ustawiony na 5s.
Komunikaty wyśweietlone w terminalu:
`Klient 0 wszedł do sklepu.`

`Klient 1 wszedł do sklepu.`

`POŻAR! Zamykanie kas...`

`Wszystkie kasy zamknięte. Kierownik kończy działanie.`

`User defined signal 1`


Alarm pożarowy działa zgodnie z założeniami. Pożar powoduje zamknięcie kas i zakończenie programu.

### Test sygnału SIGINT

Za pomocą kombinacji klawiszy Ctrl + C wywołany został sygnał SIGINT
Wynik w terminalu:

`Klient 0 wszedł do sklepu.`

`Klient 1 wszedł do sklepu.`

`^C`

`Sygnał SIGINT odebrany! Proces zakończony.`

Informacja o wysłaniu sygnału została odebrana a program zakończył swoje działanie.


## Opis działania poszczególnych modułów

### supermarket.c

**1. Inicjalizacja i obsługa sygnałów**

Program rejestruje funkcję `handle_sigint` jako obsługę sygnału SIGINT (np. gdy użytkownik naciśnie Ctrl+C). Ta funkcja:
- Oczekuje na zakończenie wszystkich procesów potomnych.
- Usuwa kolejkę komunikatów.
- Odłącza i usuwa segment pamięci dzielonej.
- Kończy działanie programu.

**2. Inicjalizacja kolejki komunikatów**
- Tworzona jest kolejka komunikatów za pomocą `msgget`. 
- Kolejka ta będzie używana do komunikacji między procesami.

**3. Inicjalizacja pamięci dzielonej**
- Tworzony jest segment pamięci dzielonej za pomocą `shmget`. 
- Pamięć ta będzie przechowywać:
  - Liczbę całkowitą (`int`), która może być używana np. do zliczania klientów.
  - Mutex (`pthread_mutex_t`), który będzie synchronizować dostęp do pamięci dzielonej.
- Pamięć dzielona jest inicjalizowana, a mutex jest inicjalizowany za pomocą `pthread_mutex_init`.

**4. Tworzenie procesów**
- **Proces kierownika (`manager`)**:  
  Tworzony jest proces potomny, który uruchamia program `manager` za pomocą `execl`.

- **Proces strażaka (`firefighter`)**:  
  Tworzony jest kolejny proces potomny, który uruchamia program `firefighter` za pomocą `execl`. 

- **Procesy klientów (`customer`)**:  
  Tworzonych jest `MAX_CUSTOMERS` procesów potomnych, z których każdy uruchamia program `customer` za pomocą `execl`.  
  Każdy proces klienta otrzymuje unikalny identyfikator (`customer_id`) oraz ID kolejki komunikatów.  
  Procesy klientów są tworzone z losowym opóźnieniem (1-5 sekund).

**5. Oczekiwanie na zakończenie procesów**
- Program główny oczekuje na zakończenie wszystkich procesów klientów za pomocą `wait`.

**6. Zamykanie supermarketu**
Po zakończeniu wszystkich procesów klientów, program:
- Usuwa kolejkę komunikatów za pomocą `msgctl`.
- Odłącza segment pamięci dzielonej za pomocą `shmdt`.
- Usuwa segment pamięci dzielonej za pomocą `shmctl`.
- Wysyła sygnał `SIGTERM` do procesów kierownika i strażaka, aby je zakończyć.

### customer.c

**1. Inicjalizacja pamięci dzielonej i muteksu**

- Tworzy klucz `ftok`, pobiera segment pamięci `shmget`, dołącza go `shmat`.
- Inicjalizuje wskaźniki do licznika klientów i muteksu.

**2. Modyfikacja licznika klientów**

- Blokuje muteks, zwiększa licznik klientów, zwalnia muteks.

**3. Wejście klienta do sklepu**

- Pobiera `customer_id` i `msgid`, drukuje komunikat, czeka losowy czas (1–20s).

**4. Komunikacja z kierownikiem**

- Wysyła wiadomość `msgsnd` (ustawienie w kolejce).
- Odbiera wiadomość `msgrcv` (przydział do kasy), drukuje informację.

**5. Obsługa przy kasie**

- Symuluje obsługę (1–10s), wysyła wiadomość o opuszczeniu kasy.

**6. Zmniejszenie licznika klientów**

- Blokuje muteks, zmniejsza licznik, zwalnia muteks.

**7. Zakończenie pracy**

- Odłącza pamięć dzieloną i kończy działanie.

### manager.c

**1. Inicjalizacja i konfiguracja**  

- Program odbiera argument `msgid` – identyfikator kolejki komunikatów.  
- Tworzy i inicjalizuje kasy (`Kasa`) – każda posiada ID, kolejkę klientów, mutex, zmienną warunkową oraz flagi aktywności i zajętości.  
- Domyślnie otwarte są 2 kasy.  
- Tworzy wątki kasjerów (`cashier_thread`), które obsługują klientów.  

**2. Obsługa pamięci dzielonej**  

- Tworzy klucz `ftok`, pobiera segment pamięci dzielonej (`shmget`).  
- Dołącza pamięć dzieloną (`shmat`) i uzyskuje wskaźnik do licznika klientów.  

**3. Obsługa sygnału SIGUSR1 (pożar)**  

- Rejestruje funkcję `handle_sigusr1`, która ustawia flagę `fire_alarm = 1`.  

**4. Główna pętla programu (obsługa klientów)**  

- Sprawdza liczbę klientów w sklepie (`customer_count = *shm_ptr`).  
- Obsługuje komunikaty z kolejki (`msgrcv`) od klientów:  
  - Jeśli `mtype == 1`:  
    - Klient ustawia się w kolejce do kasy z najmniejszą liczbą klientów.  
    - Powiadamia kasjera (`pthread_cond_signal`).  
  - Jeśli `mtype == 2`:  
    - Klient opuszcza kasę, oznaczając ją jako wolną (`zajeta = 0`).  

**5. Zarządzanie liczbą kas**  

- Oblicza liczbę wymaganych kas (`required_cashiers = customer_count / K + 1`).  
- Zawsze otwarte są min. 2 kasy.  
- Jeśli klientów jest dużo – otwiera nową kasę.  
- Jeśli klientów jest mało – zamyka kasę.  

**6. Obsługa klientów przez kasjera (wątek cashier_thread)**  

- Czeka na klientów w kolejce (`pthread_cond_wait`).  
- Obsługuje pierwszego klienta i wysyła mu komunikat (`msgsnd`).  
- Czeka, aż klient opuści kasę (`zajeta = 0`).  
- Powtarza procedurę.  

**7. Reakcja na pożar (SIGUSR1)**  

- Gdy `fire_alarm = 1`, program:  
  - Wypisuje komunikat `"POŻAR! Zamykanie kas..."`.  
  - Zamknie wszystkie kasy i zakończy działanie.  

**8. Zakończenie programu**  

- Odłącza pamięć dzieloną (`shmdt`).  
- Kończy działanie.  


### firefighter.c

**1. Inicjalizacja**  

- Program inicjalizuje generator liczb losowych (`srand(time(NULL))`).  
- Pobiera `msgid` z argumentów wywołania, czyli identyfikator kolejki komunikatów.  

**2. Symulacja pożaru**  

- Program czeka losowy czas od 20 do 120 sekund (`sleep(rand() % 100 + 20)`).  
- Po upływie tego czasu generuje sygnał pożaru (`SIGUSR1`).  

**3. Wysłanie sygnału SIGUSR1**  

- Funkcja `kill(0, SIGUSR1)` wysyła sygnał `SIGUSR1` do wszystkich procesów w grupie.  
- Procesy odbierające ten sygnał (np. kasjerzy i kierownik) podejmują odpowiednie działania – zamykają kasy i kończą działanie.  

**4. Komunikat o pożarze**  

- Program wypisuje `"POŻAR! Wszyscy klienci opuszczają sklep!"`, informując o ewakuacji.  
- Następnie kończy działanie.

## Poprawa błędu SIGUSR1
Po wysłaniu sygnału przez strazaka wszystkie zasoby zostają wyczyszczone (pamięć współdzielona i kolejka komunikatów)

