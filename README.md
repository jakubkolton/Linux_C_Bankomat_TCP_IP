# Linux_C_Bankomat_TCP_IP

Aplikacje serwera i klienta symulujące bankomat. Do komunikacji między aplikacjami wykorzystano gniazda TCP/IP.

### Funkcjonalność:
- Serwer pracuje autonomicznie (na porcie 8888), sporządzając log w pliku log.txt. Klienci oczekują go pod adresem lokalnym IP 192.168.158.129.
- Klienci dołączają się do serwera za pomocą gniazd obsługujących stos protokołów TCP/IP. Każdy z nich obsługiwany jest oddzielnym wątkiem.
- Klienci weryfikowani są za pomocą loginu i hasła, które zebrane są w "quasi bazie danych" - globalnej tablicy struktur. Login i hasło są szyfrowane autorskim, prymitywnym szyfrem.
- Każdy klient może zalogować się z wielu miejsc jednocześnie.
- System serwerowy nalicza co pewien czas odsetki (służy do tego oddzielny wątek). 
- System umożliwa wpłaty i wypłaty (nie pozwalając na debet).
- Po zalogowaniu, system serwerowy przesyła klientowi jego stan konta. Klient wyświetla też listę dostępnych operacji.


### Zrzuty ekranu z działania aplikacji klienta:
![](./Klient.JPG?raw=true "Praca klienta")

### Plik loga:
![](./Log.JPG?raw=true "Praca klienta")


## Historia zmian
#### v 1.0:
- Pierwsza stabilna, poprawnie działająca wersja programu.
- Planowane zmiany:
	- Poprawa całej struktury programu - wydzielenie szeregu oddzielnych funkcji, poprawa przejrzystości kodu.
	- Przejęcie przez serwer funkcji przekazywania poleceń użytkownikowi.
	- Dołączenie bazy danych w miejsce roboczego rozwiązania (globalnej tablicy struktur).
	- Użycie rzeczywistej bilbioteki kryptograficznej.
