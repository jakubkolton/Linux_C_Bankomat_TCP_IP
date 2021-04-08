#include <stdio.h>
#include <stdlib.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <netdb.h>
#include <netinet/in.h>

#define PORT 8888
#define IP_SERV "192.168.158.129"

// "szyfruje" podane login lub haslo - zmienia kolejnosc znakow i przesuwa je o 3 pozycje ASCII do przodu
void encryptFunction(char *str) {
	int length = strlen(str);
	char temp [length];
	for (int i = 0; i < length; i++) {
		*(temp + i) = *(str + i) + 3;
	}
	for (int i = 0; i < length; i++) {
		*(str + i) = *(temp + length - i - 1);
	}
}

int main(int argc, char *argv[]) {
        int socket_desc;
        struct sockaddr_in serv_addr;

        char txBuffer[2000];
        char rxBuffer[2000];

        // Utowrzenie socketu
        socket_desc = socket(AF_INET, SOCK_STREAM, 0);

        if (socket_desc < 0) {
                perror("Blad tworzenia socketu");
                exit(1);
        }

        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        serv_addr.sin_addr.s_addr = inet_addr(IP_SERV);

        // Dolaczenie sie do serwera
        if (connect(socket_desc, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("Blad dolaczania do serwera");
                exit(1);
        }

        printf("WITAJ W BANKU!\n");

		// logowanie
		printf("\nLOGOWANIE:\n");
		int stateLogin = 1;
		while (stateLogin) {
			printf("Podaj login: \n");
			scanf("%s", txBuffer);
			encryptFunction(txBuffer); // szyfrowanie loginu
			if (write(socket_desc, txBuffer, strlen(txBuffer)) < 0) {
				perror("Blad wysylania");
				exit(1);
			} else {
				bzero(rxBuffer, 2000);
				if (recv(socket_desc, rxBuffer, 2000, 0) < 0) {
					perror("ERROR reading from socket");
					exit(1);
				} else {
					if (!strcmp(rxBuffer, "OK")) {
						printf("Dobry login\n");
						// haslo
						int statePass = 1;
						while (statePass) {
							printf("Podaj haslo: \n");
							bzero(txBuffer, sizeof(txBuffer));
							scanf("%s", txBuffer);
							encryptFunction(txBuffer); // szyfrowanie hasla
							if (write(socket_desc, txBuffer, strlen(txBuffer)) < 0) {
								perror("Blad wysylania");
								exit(1);
							} else {
								bzero(rxBuffer, 2000);
								if (recv(socket_desc, rxBuffer, 2000, 0) < 0) {
									perror("ERROR reading from socket");
									exit(1);
								} else {
									if (!strcmp(rxBuffer, "OK")) {
										printf("Zalogowano uzytkownika\n");
										statePass = 0; // koniec petli while do wpisywania hasla
										stateLogin = 0; // koniec petli while do logowania
									} else {
										printf("Zle haslo!\n");
									}
								}
							}
						}
					} else {
						printf("Zly login!\n");
					}
				}
			}
		}

		// wyswietlenie danych uzytkownika i stanu konta:
		if (write(socket_desc, "info", strlen("info")) < 0) {
			perror("Blad wysylania");
			exit(1);
		} else {
			bzero(rxBuffer, 2000);
			if (recv(socket_desc, rxBuffer, 2000, 0) < 0) {
				perror("ERROR reading from socket");
				exit(1);
			} else {
				printf("%s\n", rxBuffer); // wyswietla cala informacje o uzytkowniku (otrzymana od serwera)
			}
		}

        int state = 1; // czy klient jest zalogowany

        // obsluga zalogowanego klienta
        while(state) {
        	printf("\nMozliwe dzialania: \n\t1 - Pokaz stan konta \n\t2 - Wplata srodkow \n\t3 - Wyplata srodkow \n\t4 - Wyloguj\n");
			// Pobranie wiadomosci od uzytkownika
			printf("Podaj wiadomosc: ");
			bzero(txBuffer, 2000); // czysci bufor - wypelnia zerami
			scanf("%s", txBuffer);

			if (!strcmp(txBuffer, "1")) { // stan konta
				if (write(socket_desc, txBuffer, strlen(txBuffer)) < 0) {
					perror("Blad wysylania");
					exit(1);
				} else {
					bzero(rxBuffer, 2000);
					if (recv(socket_desc, rxBuffer, 2000, 0) < 0) {
						perror("ERROR reading from socket");
						exit(1);
					} else {
						printf("%s\n", rxBuffer); // wyswietlenie calego komunikatu z serwera
					}
				}
			} else if (!strcmp(txBuffer, "2")) { // wplata
				printf("Ile chcesz wplacic? ");
				char cash[10];
				scanf("%s", cash);
				strcat(txBuffer, cash);
				if (write(socket_desc, txBuffer, strlen(txBuffer)) < 0) {
					perror("Blad wysylania");
					exit(1);
				} else {
					bzero(rxBuffer, 2000);
					if (recv(socket_desc, rxBuffer, 2000, 0) < 0) {
						perror("ERROR reading from socket");
						exit(1);
					} else {
						printf("%s\n", rxBuffer); // wyswietlenie calego komunikatu z serwera
					}
				}
			} else if (!strcmp(txBuffer, "3")) { // wyplata
				printf("Ile chcesz wyplacic? ");
				char cash[10];
				scanf("%s", cash);
				strcat(txBuffer, cash);
				if (write(socket_desc, txBuffer, strlen(txBuffer)) < 0) {
					perror("Blad wysylania");
					exit(1);
				} else {
					bzero(rxBuffer, 2000);
					if (recv(socket_desc, rxBuffer, 2000, 0) < 0) {
						perror("ERROR reading from socket");
						exit(1);
					} else {
						printf("%s\n", rxBuffer); // wyswietlenie calego komunikatu z serwera
					}
				}
			} else if (!strcmp(txBuffer, "4")) { // wylogowanie
				if (write(socket_desc, txBuffer, strlen(txBuffer)) < 0) {
					perror("Blad wysylania");
					exit(1);
				} else {
					printf("Wylogowano!\n");
					state = 0; // zatrzymuje obrot petli while()
				}
			} else {
				printf("Niepoprawna komenda\n");
			}
        }

        /* Now read server response */
        return 0;
}
