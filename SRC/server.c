#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 9999
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clients_actuels = 0;

typedef struct {
    int socket;
    struct sockaddr_in adresse;
} client_info;

typedef struct {
    char echiquier[8][8];
    char etat_precedent[8][8]; // Nouvelle variable pour stocker l'état précédent de l'échiquier
    int joueur_blanc_socket;
    int joueur_noir_socket;
    int tour;  // 0 pour les blancs, 1 pour les noirs
} partie_echecs;

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

int valider_mouvement(char echiquier[8][8], int x1, int y1, int x2, int y2) {
    // Vérifier que la case de départ contient une pièce
    if (echiquier[x1][y1] == '.') {
        return 0;
    }

    // Vérifier que la case d'arrivée est vide ou contient une pièce adverse
    char piece_arrivee = echiquier[x2][y2];
    char piece_depart = echiquier[x1][y1];
    if (piece_arrivee != '.' && (piece_arrivee - 32 == piece_depart || piece_arrivee == piece_depart)) {
        return 0;
    }

    // Vérifier que le mouvement est valide pour la pièce
    switch (piece_depart) {
        case 'p': case 'P':
            // Vérifier le mouvement d'un pion
            if ((x2 - x1 == 1 || x2 - x1 == -1) && (y2 - y1 == 0 || y2 - y1 == 1 || y2 - y1 == -1)) {
                return 1;
            }
            break;
        case 't': case 'T':
            // Vérifier le mouvement d'une tour
            if ((x1 == x2 && y1 != y2) || (x1 != x2 && y1 == y2)) {
                int x_min = (x1 < x2) ? x1 : x2;
                int x_max = (x1 > x2) ? x1 : x2;
                int y_min = (y1 < y2) ? y1 : y2;
                int y_max = (y1 > y2) ? y1 : y2;
                for (int i = x_min + 1; i < x_max; i++) {
                    if (echiquier[i][y1] != '.') {
                        return 0;
                    }
                }
                for (int j = y_min + 1; j < y_max; j++) {
                    if (echiquier[x1][j] != '.') {
                        return 0;
                    }
                }
                return 1;
            }
            break;
        case 'c': case 'C':
            // Vérifier le mouvement d'un cavalier
            if ((x2 - x1 == 2 || x2 - x1 == -2) && (y2 - y1 == 1 || y2 - y1 == -1)) {
                return 1;
            }
            if ((x2 - x1 == 1 || x2 - x1 == -1) && (y2 - y1 == 2 || y2 - y1 == -2)) {
                return 1;
            }
            break;
        case 'f': case 'F':
            // Vérifier le mouvement d'un fou
            if ((x2 - x1 == 1 || x2 - x1 == -1) && (y2 - y1 == 1 || y2 - y1 == -1)) {
                return 1;
            }
            if ((x2 - x1 == 2 || x2 - x1 == -2) && (y2 - y1 == 2 || y2 - y1 == -2)) {
                return 1;
            }
            if ((x2 - x1 == 3 || x2 - x1 == -3) && (y2 - y1 == 3 || y2 - y1 == -3)) {
                return 1;
            }
            if ((x2 - x1 == 4 || x2 - x1 == -4) && (y2 - y1 == 4 || y2 - y1 == -4)) {
                return 1;
            }
            break;
        case 'd': case 'D':
            // Vérifier le mouvement d'une dame
            if ((x1 == x2 && y1 != y2) || (x1 != x2 && y1 == y2)) {
                int x_min = (x1 < x2) ? x1 : x2;
                int x_max = (x1 > x2) ? x1 : x2;
                int y_min = (y1 < y2) ? y1 : y2;
                int y_max = (y1 > y2) ? y1 : y2;
                for (int i = x_min + 1; i < x_max; i++) {
                    if (echiquier[i][y1] != '.') {
                        return 0;
                    }
                }
                for (int j = y_min + 1; j < y_max; j++) {
                    if (echiquier[x1][j] != '.') {
                        return 0;
                    }
                }
                return 1;
            }
            if ((x2 - x1 == 1 || x2 - x1 == -1) && (y2 - y1 == 1 || y2 - y1 == -1)) {
                return 1;
            }
            if ((x2 - x1 == 2 || x2 - x1 == -2) && (y2 - y1 == 2 || y2 - y1 == -2)) {
                return 1;
            }
            if ((x2 - x1 == 3 || x2 - x1 == -3) && (y2 - y1 == 3 || y2 - y1 == -3)) {
                return 1;
            }
            if ((x2 - x1 == 4 || x2 - x1 == -4) && (y2 - y1 == 4 || y2 - y1 == -4)) {
                return 1;
            }
            break;
        case 'r': case 'R':
            // Vérifier le mouvement d'un roi
            if ((x2 - x1 == 1 || x2 - x1 == -1) && (y2 - y1 == 0 || y2 - y1 == 1 || y2 - y1 == -1)) {
                return 1;
            }
            break;
        default:
            return 0;
    }

    return 0;
}

void *gerer_partie(void *arg) {
    partie_echecs *partie = (partie_echecs *)arg;
    char buffer[BUFFER_SIZE];
    int quitter = 0;

    // Envoyer les couleurs des pièces aux joueurs
    send(partie->joueur_blanc_socket, "Vous jouez avec les pièces blanches.", 37, 0);
    send(partie->joueur_noir_socket, "Vous jouez avec les pièces noires.", 35, 0);

    while (!quitter) {
        int client_socket = (partie->tour == 0) ? partie->joueur_blanc_socket : partie->joueur_noir_socket;
        int autre_socket = (partie->tour == 0) ? partie->joueur_noir_socket : partie->joueur_blanc_socket;

        memset(buffer, 0, BUFFER_SIZE);

        int bytes_lus = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_lus <= 0) {
            printf("Client déconnecté\n");
            quitter = 1;
            break;
        }

        printf("Reçu du client : %s\n", buffer);

        // Logique pour traiter les mouvements et mettre à jour l'échiquier
        int x1 = 7 - (buffer[1] - '1'); // Adjusted index for the first rank
        int y1 = buffer[0] - 'a';
        int x2 = 7 - (buffer[3] - '1'); // Adjusted index for the second rank
        int y2 = buffer[2] - 'a';

        if (x1 >= 0 && x1 < 8 && y1 >= 0 && y1 < 8 && x2 >= 0 && x2 < 8 && y2 >= 0 && y2 < 8) {
            if (valider_mouvement(partie->echiquier, x1, y1, x2, y2)) {
                // Mettre à jour l'état de l'échiquier
                partie->echiquier[x2][y2] = partie->echiquier[x1][y1];
                partie->echiquier[x1][y1] = '.';

                // Mettre à jour l'état précédent de l'échiquier
                memcpy(partie->etat_precedent, partie->echiquier, sizeof(partie->echiquier));

                // Envoyer l'échiquier mis à jour aux deux joueurs
                send(client_socket, partie->echiquier, sizeof(partie->echiquier), 0);
                send(autre_socket, partie->echiquier, sizeof(partie->echiquier), 0);

                // Basculer le tour
                partie->tour = (partie->tour == 0) ? 1 : 0;

                // Si le dernier coup a été joué par le joueur blanc, envoyer l'état précédent de l'échiquier au client noir
                if (partie->tour == 1) {
                    send(partie->joueur_noir_socket, partie->etat_precedent, sizeof(partie->etat_precedent), 0);
                }

                // Afficher l'échiquier avec l'indication du tour
                afficher_echiquier(partie->echiquier, partie->tour);
            } else {
                printf("Mouvement invalide\n");
                send(client_socket, "Mouvement invalide", 17, 0);
            }
        }
    }

    close(partie->joueur_blanc_socket);
    close(partie->joueur_noir_socket);
    free(partie);

    pthread_mutex_lock(&mutex);
    clients_actuels -= 2;  // Deux clients de moins
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void demarrer_serveur() {
    int serveur_socket, client_socket;
    struct sockaddr_in serveur_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t threads[MAX_CLIENTS / 2];

    serveur_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (serveur_socket == -1) {
        perror("Impossible de créer la socket");
        exit(EXIT_FAILURE);
    }

    serveur_addr.sin_family = AF_INET;
    serveur_addr.sin_addr.s_addr = INADDR_ANY;
    serveur_addr.sin_port = htons(PORT);

    if (bind(serveur_socket, (struct sockaddr *)&serveur_addr, sizeof(serveur_addr)) < 0) {
        perror("Échec du binding");
        close(serveur_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(serveur_socket, 3) < 0) {
        perror("Échec de l'écoute");
        close(serveur_socket);
        exit(EXIT_FAILURE);
    }

    printf("Serveur démarré sur le port %d\n", PORT);

    while (1) {
        if (clients_actuels >= MAX_CLIENTS) {
            printf("Nombre maximum de clients atteint. En attente de connexions disponibles...\n");
            sleep(1);
            continue;
        }

        client_socket = accept(serveur_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Échec de l'acceptation");
            continue;
        }

        pthread_mutex_lock(&mutex);
        clients_actuels++;
        pthread_mutex_unlock(&mutex);

        printf("Client connecté. Clients actuels : %d\n", clients_actuels);

        // Attendre le deuxième joueur pour la partie
        int client_socket_2 = accept(serveur_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket_2 < 0) {
            perror("Échec de l'acceptation");
            close(client_socket);
            continue;
        }

        pthread_mutex_lock(&mutex);
        clients_actuels++;
        pthread_mutex_unlock(&mutex);

        printf("Deuxième client connecté. Clients actuels : %d\n", clients_actuels);

        partie_echecs *partie = malloc(sizeof(partie_echecs));
        initialiser_echiquier(partie->echiquier);
        memcpy(partie->etat_precedent, partie->echiquier, sizeof(partie->echiquier)); // Initialiser l'état précédent de l'échiquier
        partie->joueur_blanc_socket = client_socket;
        partie->joueur_noir_socket = client_socket_2;
        partie->tour = 0;

        if (pthread_create(&threads[(clients_actuels / 2) - 1], NULL, gerer_partie, partie) != 0) {
            perror("Échec de la création du thread");
            close(client_socket);
            close(client_socket_2);
            free(partie);

            pthread_mutex_lock(&mutex);
            clients_actuels -= 2;
            pthread_mutex_unlock(&mutex);
        }
    }

    close(serveur_socket);
}

void afficher_menu() {
    printf("\033[1;34m");  // Couleur bleue pour le titre
    printf("*********************************\n");
    printf("*        Serveur d'échecs       *\n");
    printf("*********************************\n");
    printf("\033[0m");  // Réinitialiser la couleur
    printf("1. \033[1;32mDémarrer le jeu\033[0m\n");  // Vert pour démarrer
    printf("2. \033[1;36mConsulter les règles\033[0m\n");  // Cyan pour consulter les règles
    printf("3. \033[1;36mConsulter les instructions\033[0m\n");  // Cyan pour consulter les instructions
    printf("4. \033[1;36mVoir les joueurs en ligne\033[0m\n");  // Cyan pour voir les joueurs en ligne
    printf("5. \033[1;31mQuitter\033[0m\n");  // Rouge pour quitter
    printf("Entrez votre choix: ");
}

void consulter_regles() {
    printf("\n\033[1;33mRègles des échecs:\033[0m\n");
    printf("- Les pions se déplacent d'une case vers l'avant.\n");
    printf("- Les tours se déplacent en ligne droite.\n");
    printf("- Les cavaliers se déplacent en forme de L.\n");
    printf("- Les fous se déplacent en diagonale.\n");
    printf("- La reine se déplace dans toutes les directions.\n");
    printf("- Le roi se déplace d'une case dans toutes les directions.\n\n");
}

void consulter_instructions() {
    printf("\n\033[1;33mInstructions pour jouer:\033[0m\n");
    printf("Pour déplacer une pièce, entrez les coordonnées de départ et d'arrivée sous la forme:\n");
    printf("xyXY\n");
    printf("où (x, y) sont les coordonnées de départ et (X, Y) sont les coordonnées d'arrivée.\n");
    printf("Par exemple, pour déplacer une pièce de a2 à a4, entrez: a2a4\n\n");
}

void voir_joueurs_en_ligne() {
    printf("\nJoueurs en ligne: %d\n\n", clients_actuels);
}

int main() {
    int choix;
    do {
        afficher_menu();
        scanf("%d", &choix);

        switch (choix) {
            case 1:
                demarrer_serveur();
                break;
            case 2:
                consulter_regles();
                break;
            case 3:
                consulter_instructions();
                break;
            case 4:
                voir_joueurs_en_ligne();
                break;
            case 5:
                printf("Au revoir!\n");
                break;
            default:
                printf("Choix invalide. Veuillez réessayer.\n");
        }
    } while (choix != 5);

    return 0;
}


