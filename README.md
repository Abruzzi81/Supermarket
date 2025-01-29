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









