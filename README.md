# Linux_C_Bankomat_TCP_IP

Aplikacje serwera i klienta symulujące bankomat. Do komunikacji między aplikacjami wykorzystano gniazda TCP/IP.

### Funkcjonalność:
- Serwer pracuje autonomicznie (na porcie 8888), sporządzając log w pliku log.txt.
- Klienci dołączają się do serwera za pomocą gniazd obsługujących stos protokołów TCP/IP. Każdy z nich obsługiwany jest oddzielnym wątkiem.
- Klienci weryfikowani są za pomocą loginu i hasła, które zebrane są w "quasi bazie danych" - strukturze globalnej. Login i hasło są szyfrowane autorskim, prymitywnym szyfrem.
- Każdy klient może zalogować się z wielu miejsc jednocześnie.
- System serwerowy nalicza co pewien czas odsetki. Umożliwa również wpłaty i wypłaty (nie pozwalając na debet).
- Po zalogowaniu, system serwerowy przesyła klientowi jego stan konta.
- ? czy jest polecenie help ?
- ? coś jeszcze ? 
- ? sprawdzić całą funkcjonalność programów?

### Zrzuty ekranu z działania aplikacji klienta:
![](./Klient.jpg?raw=true "Praca klienta")

### Plik loga:
![](./Log.jpg?raw=true "Praca klienta")
