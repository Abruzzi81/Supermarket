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
