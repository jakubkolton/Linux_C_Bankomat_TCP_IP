#include <stdio.h>
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <sys/fcntl.h> //fcntl
#include <errno.h> //
#include <pthread.h> // obsluga watkow

#define PORT 8888

struct client {
	uint id;
	char *login;
	char *password;
	float cash;
	float interestRate; // stopa oprocentowania (np. 2% -> 0.02)
};

// Baza klientow
struct client clients[] = { {0, "user1", "pass1", 1000.0, 0.02},
							{1, "user2", "pass2", 420.0, 0.01},
};

// formatuje podanego stringa tak, by zawieral aktualna date i godzine
void current_time(char *strTime) {
	time_t rawtime; // zmienna na czas
	struct tm *timeinfo; // struktura na date i godzina
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(strTime, "[%d.%d.%d %d:%d:%d]  --  ", timeinfo->tm_mday,
			timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour,
			timeinfo->tm_min, timeinfo->tm_sec);
}

// oblicza odsetki
void* interest_handler () {
	int interestInterval = 15; // odsetki naliczane co 15 sekund
	clock_t start = time(0); // moment startu serwera
	char timeBuffer[100]; // bufor na biezaca date i czas - potrzebne do loga

	while(1) {
		sleep(1); // odczekuje sekunde - jest to nie do konca poprawne, ale odciaza system z krecenia petla while()
		clock_t now = time(0); // obecny moment
		clock_t diff = now - start; // czas jaki uplynal od startu serwera

		if (diff%interestInterval == 0) { // jesli minelo 15 sekund lub wielokrotnosc 5 sekund
		for (int i = 0; i < 2; i++) { // kolejni klienci
			float interest = clients[i].cash*clients[i].interestRate; // wysokosc odsetek
			clients[i].cash += interest; // doliczenie odstetek

			FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
			fprintf(log, "%s*AUTOMAT_ODSETKI* Uzytkownik %s otrzymal %.2f PLN odsetek.\n",timeBuffer, clients[i].login, interest);
			fclose(log);
			}
		}
	}

	pthread_exit(NULL); // zamkniecie watku
}

// "deszyfruje" podane login lub haslo - zmienia kolejnosc znakow i przesuwa je o 3 pozycje ASCII do tylu
void decryptFunction(char *str) {
	int length = strlen(str);
	char temp [length];
	for (int i = 0; i < length; i++) {
		*(temp + i) = *(str + i) - 3;
	}
	for (int i = 0; i < length; i++) {
		*(str + i) = *(temp + length - i - 1);
	}
}

void* thread_handler (void* p_socket) {

	int sock = *(int *)p_socket; // socket klienta - identyfikator dolaczonego klienta
	int read_size; // zmienne pomocnicze do wysylania i odbioru
	char txBuffer[2000];
	static char rxBuffer[2000];

	char timeBuffer[100]; // bufor na biezaca date i czas - potrzebne do loga

	// Odbieranie wiadomosci od klienta
	int stage = 0; // etap obslugi klienta: 0 - login, 1 - haslo, 2 - dane uzytkownika i stan konta, 4 - obsluga zadan klienta
	int j = -1; // identyfikator obslugiwanego klienta
	while((read_size = recv(sock , rxBuffer , 2000 , 0)) > 0 ) {
		printf("%s\n", rxBuffer);
		switch (stage) {
			case 0: // login
				// wskazanie, id klienta z bazy
				decryptFunction(rxBuffer); // odszyfrowanie loginu
				for (int i = 0; i < 2; i++) {
					if (!strcmp(rxBuffer, clients[i].login)) {
						j = i; // wybrano klienta o id = i
						write(sock, "OK", strlen("OK"));
						printf("dobry login\n");
						stage = 1;

						FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
						fprintf(log, "%sKlient o ID: %d  --  Podano wlasciwy login: %s\n",timeBuffer, sock, clients[i].login);
						fclose(log);
					} else if (stage == 0 && i == 1) {
						write(sock, "zle", strlen("OK"));
						printf("zly login\n");

						FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
						fprintf(log, "%sKlient o ID: %d  --  Podano zly login.\n",timeBuffer, sock);
						fclose(log);
					}
				}
				break;
			case 1: // haslo
				decryptFunction(rxBuffer); // odszyfrowanie hasla
				if (!strcmp(rxBuffer, clients[j].password)) {
					write(sock, "OK", strlen("OK"));
					printf("dobre haslo\n");
					stage = 2;

					FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
					fprintf(log, "%sKlient o ID: %d  --  Podano wlasciwe haslo.\n",timeBuffer, sock);
					fclose(log);
				} else {
					write(sock, "zle", strlen("OK"));
					printf("zle haslo\n");

					FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
					fprintf(log, "%sKlient o ID: %d  --  Podano zle haslo.\n",timeBuffer, sock);
					fclose(log);
				}
				break;
			case 2: // dane uzytkownika i stan konta
				if (!strcmp(rxBuffer, "info")) {
					strcat(txBuffer, "Zalogowano jako: ");
					strcat(txBuffer, clients[j].login);
					char cash[20];
					sprintf(cash, "%.2f", clients[j].cash); // konwertuje kwote do stringa
					strcat(txBuffer, "\nSaldo: ");
					strcat(txBuffer, cash);
					strcat(txBuffer, " PLN\n");
					write(sock, txBuffer, strlen(txBuffer));
					stage = 3;
					printf("Wyslano dane o kliencie\n");
					printf("%s\n", txBuffer);

					FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
					fprintf(log, "%sKlient o ID: %d  --  Zalogowano. Wyslano dane o kliencie.\n",timeBuffer, sock);
					fclose(log);
				}
				break;
			case 3: // obsluga zadan klienta (stan konta, wplata, wyplata, wyjscie)
				if (!strcmp(rxBuffer, "1")) { // stan konta
					bzero(txBuffer, 2000);
					char cash[20];
					sprintf(cash, "%.2f", clients[j].cash); // konwertuje kwote do stringa
					strcat(txBuffer, "Saldo: ");
					strcat(txBuffer, cash);
					strcat(txBuffer, " PLN\n");
					write(sock, txBuffer, strlen(txBuffer));

					FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
					fprintf(log, "%sKlient o ID: %d  --  Prosba o saldo konta. Wyslano.\n",timeBuffer, sock);
					fclose(log);
				} else if (!strncmp(rxBuffer, "2", 1)) { // wplata srodkow
					printf("odebrano 2 + kwota\n");
					bzero(txBuffer, 2000);
					float cashAdd = 0;
					sscanf(rxBuffer+1, "%float", &cashAdd); // konwertuje stringa na floata
					clients[j].cash += cashAdd;
					bzero(txBuffer, 2000);
					char cash[20];
					sprintf(cash, "%.2f", clients[j].cash); // konwertuje kwote do stringa
					strcat(txBuffer, "Wplata udana.\nSaldo: ");
					strcat(txBuffer, cash);
					strcat(txBuffer, " PLN\n");
					write(sock, txBuffer, strlen(txBuffer));

					FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
					fprintf(log, "%sKlient o ID: %d  --  Prosba o wplate %.2f PLN. Wplacono i wyslano saldo.\n",timeBuffer, sock, cashAdd);
					fclose(log);
				} else if (!strncmp(rxBuffer, "3", 1)) { // wyplata srodkow
					printf("odebrano 3 + kwota\n");
					bzero(txBuffer, 2000);
					float cashSubtract = 0;
					sscanf(rxBuffer + 1, "%f", &cashSubtract); // konwertuje stringa na inta
					if (cashSubtract <= clients[j].cash) { // nieco bledny sposob porownania, bo to floaty
						clients[j].cash -= cashSubtract;
						char cash[20];
						sprintf(cash, "%.2f", clients[j].cash); // konwertuje kwote do stringa
						strcat(txBuffer, "Wyplata udana.\nSaldo: ");
						strcat(txBuffer, cash);
						strcat(txBuffer, " PLN\n");
						write(sock, txBuffer, strlen(txBuffer));

						FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
						fprintf(log, "%sKlient o ID: %d  --  Prosba o wyplate %.2f PLN. Wyplacono i wyslano saldo.\n",timeBuffer, sock, cashSubtract);
						fclose(log);
					} else {
						strcat(txBuffer, "Wyplata nieudana! Nie mozesz zrobic debetu!\n");
						write(sock, txBuffer, strlen(txBuffer));

						FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
						fprintf(log, "%sKlient o ID: %d  --  Prosba o wyplate %.2f PLN. Niedostateczna ilosc srodkow.\n",timeBuffer, sock, cashSubtract);
						fclose(log);
					}
				} else if (!strcmp(rxBuffer, "4")) { // wylogowanie
					printf("Uzykownik chce sie wylogowac\n");
					read_size = 0; // wyrzuca z petli while(), powinien odlaczyc klienta w nastepnym if-ie

					FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
					fprintf(log, "%sKlient o ID: %d  --  Prosba o wylogowanie. Wylogowano.\n",timeBuffer, sock);
					fclose(log);
				}
				break;
			default:
				printf("Niepoprawna komenda!\n");

				FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
				fprintf(log, "%sKlient o ID: %d  --  Niepoprawna komenda.\n",timeBuffer, sock);
				fclose(log);

				break;
		}
		bzero(rxBuffer,2000); // czysci bufor - wypelnia zerami
	}

	if(read_size == 0) {
		printf("Klient odlaczony\n");
		fflush(stdout);
	}
	else if(read_size == -1) {
		perror("Blad odbioru wiadomosci\n");
	}

	close(sock); // zamkniecie socketu
	free(p_socket);
	pthread_exit(NULL); // zamkniecie watku
}



int main(int argc , char *argv[]) {
	// Zmienne pomocnicze do obslugi socketow
	int socket_desc, client_sock, c, *new_socket;
	struct sockaddr_in server, client;

	// Zmienne pomocnicze do obslugi loga
	char timeBuffer[100]; // bufor na biezaca date i czas - potrzebne do loga
	FILE *log = fopen("log.txt", "a"); // otwiera plik log.txt w trybie append - zapisywania NA KONCU PLIKU
	if (log == NULL) {
		perror("Blad tworzenia wskaznika do pliku!\n");
		exit(1);
	}
	current_time(timeBuffer);
	fprintf(log, "\n%s%s", timeBuffer, "Uruchomiono serwer\n"); // wpisuje tekst do pliku
	fclose(log);

	// Utworzenie socketu
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Blad tworzenia socketu\n");
	} else {
		printf("Socket utworzony\n");
	}

	// Struktura server typu sockadrr_in
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	// Bindowanie - rejestracja uslugi w systemie
	if (bind(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("Blad bindowania\n");
		return 1;
	} else {
		printf("Bindowanie udane\n");
	}

	// Nasluchiwanie portu
	listen(socket_desc, 3); // max 3 rownolegle polaczenia TCP

	// Oczekiwanie na nadchodzace polaczenie
	printf("Oczekiwanie na nadejscie polaczenia...\n");
	c = sizeof(struct sockaddr_in);

	// Watek - obsluga odsetek
	pthread_t interest_thread;
	pthread_create(&interest_thread, NULL, &interest_handler, NULL);

	// Petla - Akceptowanie nadchodzacego polaczenia od klienta
	while ((client_sock = accept(socket_desc, (struct sockaddr*) &client, (socklen_t*) &c))) {
		printf("Polaczenie zaakceptowane. ID: %d\n", client_sock);

		FILE *log = fopen("log.txt", "a"); current_time(timeBuffer);
		char clientIP[20];
		inet_ntop(AF_INET, &client.sin_addr, clientIP, sizeof(clientIP));
		fprintf(log, "%sDolaczono nowego klienta (IP: %s). Nadano ID: %d\n", timeBuffer, clientIP, client_sock);
		fclose(log);

		pthread_t server_thread;
		new_socket = malloc(1);
		*new_socket = client_sock;
		pthread_create(&server_thread, NULL, &thread_handler, (void*) new_socket);
	}

	if (client_sock < 0) {
		perror("Blad akceptacji\n");
	}

	return 0;
}
