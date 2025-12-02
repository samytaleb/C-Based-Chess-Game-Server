#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUFFER_SIZE 1024

void initialiser_echiquier(char echiquier[8][8]) {
    // Initialiser les pions
    for (int i = 0; i < 8; i++) {
        echiquier[1][i] = 'p'; // Pion noir
        echiquier[6][i] = 'P'; // Pion blanc
    }

    // Initialiser les pièces
    char pieces_noires[] = {'t', 'c', 'f', 'd', 'r', 'f', 'c', 't'};
    char pieces_blanches[] = {'T', 'C', 'F', 'D', 'R', 'F', 'C', 'T'};
    for (int i = 0; i < 8; i++) {
        echiquier[0][i] = pieces_noires[i]; // Pièces noires
        echiquier[7][i] = pieces_blanches[i]; // Pièces blanches
    }

    // Initialiser les cases vides
    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            echiquier[i][j] = '.';
        }
    }
}

void afficher_echiquier(char echiquier[8][8], int tour) {
    printf("\033[1;34m");  // Couleur bleue pour l'échiquier
    printf("  a b c d e f g h\n");
    printf(" +-----------------+\n");
    for (int i = 0; i < 8; i++) {
        printf("%d|", 8 - i);
        for (int j = 0; j < 8; j++) {
            switch (echiquier[i][j]) {
                case 'p':
                    printf(" \033[1;30m%c\033[0m", echiquier[i][j]);
                    break;
                case 'P':
                    printf(" \033[1;37m%c\033[0m", echiquier[i][j]);
                    break;
                case 't':
                    printf(" \033[1;30m%c\033[0m", echiquier[i][j]);
                    break;
                case 'T':
                    printf(" \033[1;37m%c\033[0m", echiquier[i][j]);
                    break;
                case 'c':
                    printf(" \033[1;30m%c\033[0m", echiquier[i][j]);
                    break;
                case 'C':
                    printf(" \033[1;37m%c\033[0m", echiquier[i][j]);
                    break;
                case 'f':
                    printf(" \033[1;30m%c\033[0m", echiquier[i][j]);
                    break;
                case 'F':
                    printf(" \033[1;37m%c\033[0m", echiquier[i][j]);
                    break;
                case 'd':
                    printf(" \033[1;30m%c\033[0m", echiquier[i][j]);
                    break;
                case 'D':
                    printf(" \033[1;37m%c\033[0m", echiquier[i][j]);
                    break;
                case 'r':
                    printf(" \033[1;30m%c\033[0m", echiquier[i][j]);
                    break;
                case 'R':
                    printf(" \033[1;37m%c\033[0m", echiquier[i][j]);
                    break;
                default:
                    printf(" %c", echiquier[i][j]);
                    break;
            }
        }
        printf(" |\n");
    }
    printf(" +-----------------+\n");
    printf("\033[0m");  // Réinitialiser la couleur

    // Indiquer à qui est le tour de jouer
    if (tour == 0) {
        printf("C'est le tour des \033[1;37mBlancs\033[0m.\n");  // Blanc
    } else {
        printf("C'est le tour des \033[1;30mNoirs\033[0m.\n");  // Noir
    }
}

int main() {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char echiquier[8][8];
    int tour = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");

    // Initialiser l'échiquier
    initialiser_echiquier(echiquier);

    // Afficher l'échiquier
    afficher_echiquier(echiquier, tour);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Attendre la réception de l'échiquier mis à jour
        n = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (n <= 0) {
            printf("Disconnected from the server\n");
            break;
        }

        // Copier le nouvel échiquier reçu
        memcpy(echiquier, buffer, sizeof(echiquier));

        // Afficher l'échiquier
        afficher_echiquier(echiquier, tour);

        if (tour == 0) {
            // Demander à l'utilisateur d'entrer un mouvement
            printf("Entrez votre mouvement (ex. : a2a4) : ");
            scanf("%s", buffer);

            // Envoyer le mouvement au serveur
            send(sockfd, buffer, strlen(buffer), 0);
        }

        // Mettre à jour le tour
        tour = (tour + 1) % 2;
    }

    close(sockfd);
    return 0;
}

