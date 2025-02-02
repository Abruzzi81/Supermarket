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
• 10 klientów

   Czas trwania symulacji: 40.380885 s
   
• 100 klientów

   

• 1000 klientów

   Czas trwania symulacji: 2077.483760 s  =  34 min 37 s


