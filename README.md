# ♟ Serveur d'échecs multijoueur en C

Projet pédagogique de programmation système et réseau : **serveur d'échecs en C11**, communication **TCP**, parties à deux joueurs et **threads POSIX**. Le serveur peut gérer jusqu'à **cinq parties simultanées**.

> **Sécurité :** protocole TCP non chiffré et sans authentification. Pour une démonstration locale ou sur un réseau de confiance uniquement ; ne pas l'exposer directement à Internet.

## Fonctionnalités

- Échiquier 8 × 8 en terminal et coups au format **e2e4**.
- Attribution automatique des blancs et des noirs.
- Alternance des tours, validation des coups, détection de l'échec.
- **Échec et mat, pat, roque, prise en passant et promotion** (par exemple **e7e8q**).
- Gestion des déconnexions et des sessions concurrentes.
- Tests unitaires de règles et test d'intégration réseau.

## Prérequis

Linux / environnement POSIX, GCC ou Clang avec support C11, make, pthread ; Python 3 pour le test d'intégration.

## Démarrage rapide

Depuis la racine du dépôt, compiler :

~~~bash
make
~~~

Terminal 1 — lancer le serveur :

~~~bash
./server
~~~

Terminal 2 — premier joueur (blancs) :

~~~bash
./client
~~~

Terminal 3 — deuxième joueur (noirs) :

~~~bash
./client
~~~

Le premier client attend la connexion du deuxième. Les joueurs saisissent ensuite chacun leur tour un coup en coordonnées (par exemple **e2e4**), ou **quit** pour quitter.

Par défaut, le serveur écoute sur **127.0.0.1:9999** (accès local uniquement).

### Deux machines sur le réseau local

Sur la machine serveur (adapter l'adresse IP à son interface locale) :

~~~bash
./server 9999 192.168.1.20
~~~

Sur chacune des machines clientes :

~~~bash
./client 192.168.1.20 9999
~~~

**Arguments :** serveur = port, adresse IPv4 d'écoute ; client = adresse IPv4 du serveur, port. Ouvrir uniquement les accès réseau nécessaires sur un réseau autorisé.

Le script **SRC/setup.sh** n'active plus UFW ni ne modifie les règles de pare-feu ; il compile uniquement le projet. **SRC/compilation.sh** fait de même.

## Structure

~~~text
.
├── Makefile
├── README.md
├── Report.pdf
├── Slides.pdf
├── SRC/
│   ├── chess.h           # API du moteur
│   ├── chess.c           # Règles et statut
│   ├── server.c          # Serveur TCP multi-parties
│   ├── client.c          # Interface console
│   ├── compilation.sh
│   ├── setup.sh
│   └── README.txt
├── tests/
│   ├── test_chess.c      # Tests de règles
│   └── test_protocol.py  # Test d'une partie TCP
└── .github/workflows/ci.yml
~~~

## Tests

~~~bash
make test
make clean
~~~

**make test** vérifie notamment les mouvements interdits, un échec et mat, le roque, la prise en passant, la promotion et une session réseau complète. Le workflow GitHub Actions exécute la même commande.

## Protocole réseau

Chaque message est une ligne terminée par un retour à la ligne.

| Message | Sens | Usage |
| --- | --- | --- |
| HELLO WHITE / HELLO BLACK | Serveur → client | Couleur attribuée |
| BOARD [64 cases] [W/B] | Serveur → clients | Position actuelle et tour |
| YOUR_TURN | Serveur → client | Invitation à jouer |
| MOVE e2e4 | Client → serveur | Proposition de coup |
| ERROR [raison] | Serveur → client | Rejet d'un coup |
| INFO [message] | Serveur → client | État / échec |
| GAMEOVER [raison] | Serveur → clients | Fin de partie |
| QUIT | Client → serveur | Quitter la partie |

La validation des déplacements est effectuée côté serveur. Le client n'envoie que des commandes bornées et reçoit un échiquier mis à jour après chaque coup accepté.

## Limites connues

- Pas de gestion de la **nulle par répétition**, de la règle des **cinquante coups**, ni des **positions avec matériel insuffisant**.
- Pas de reconnexion, d'horloge, de classement, de PGN ou de sauvegarde.
- Pas de TLS, d'identification ou de protection avancée contre des clients malveillants.
- La suite de tests ne remplace pas un audit exhaustif du moteur d'échecs.

## Documentation

- [Rapport technique](Report.pdf)
- [Slides de présentation](Slides.pdf)

**Stack :** C11 · sockets TCP/IPv4 · pthread · Make · Python tests · GitHub Actions.
