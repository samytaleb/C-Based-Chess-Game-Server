# ♟ Serveur de jeu d’échecs en C

Projet de programmation réseau proposant un **serveur d’échecs multijoueur en terminal**, développé en **C** sous Linux. Le serveur utilise des **sockets TCP** pour communiquer avec les clients et des **threads POSIX (pthreads)** pour gérer les parties.

> **État du projet : prototype pédagogique.** L’architecture réseau et l’échiquier sont implémentés, mais le code comporte des limitations qui empêchent de garantir une partie complète et conforme aux règles des échecs. Voir [Limitations connues](#limitations-connues).

## Fonctionnalités présentes dans le code

- Serveur TCP écoutant sur le **port 9999**.
- Appariement de deux connexions client par partie, avec un thread serveur dédié.
- Échiquier **8 × 8** affiché dans le terminal, avec couleurs ANSI.
- Saisie des déplacements sous une forme simple : `a2a4`.
- Vérifications partielles des mouvements par type de pièce.
- Menu côté serveur pour démarrer le service, consulter les règles ou instructions et afficher le nombre de clients.

## Structure du dépôt

```text
.
├── README.md
├── Report.pdf               # Rapport technique
├── Slides.pdf               # Présentation
└── SRC/
    ├── server.c             # Serveur, parties et validation des coups
    ├── client.c             # Client et affichage de l'échiquier
    ├── compilation.sh       # Compilation des deux programmes
    ├── setup.sh             # Configuration UFW et compilation
    └── README.txt           # Instructions historiques
```

## Prérequis

- Linux (les scripts utilisent Bash et `setup.sh` est prévu pour une distribution disposant d’`apt` et d’**UFW**).
- **GCC** et les bibliothèques de développement nécessaires à **pthreads**.
- Un terminal compatible avec les séquences ANSI.
- Pour un accès entre machines, le port TCP 9999 doit être joignable sur un réseau autorisé.

## Compilation

Depuis la racine du dépôt :

```bash
cd SRC
bash compilation.sh
```

Ou compiler directement :

```bash
gcc server.c -o server -pthread
gcc client.c -o client
```

Le script `setup.sh` installe et **active UFW**, puis autorise le port 9999. **Ne l’exécutez pas sans vérifier votre configuration réseau**, notamment si vous accédez à la machine par SSH : l’activation d’UFW peut bloquer des connexions existantes.

## Démarrage (expérimental)

Dans un premier terminal, depuis `SRC/` :

```bash
./server
```

Choisir **1 — Démarrer le jeu** dans le menu. Le serveur écoute alors sur le port TCP **9999** et attend deux clients.

Dans deux autres terminaux :

```bash
./client
```

⚠️ **Important :** la version actuelle de `client.c` ne lit aucun argument d’adresse IP et initialise la destination avec `INADDR_ANY`. Les anciennes instructions suggérant `./client <adresse-ip>` ne correspondent donc **pas** à l’implémentation actuelle. La connexion à un serveur distant nécessite de modifier le client pour accepter et utiliser une adresse IPv4. Le fonctionnement des parties doit également être corrigé avant de considérer cette procédure comme validée.

## Format des coups

Les déplacements sont exprimés par les coordonnées de départ puis d’arrivée, sans espace :

```text
a2a4
g1f3
```

Les lettres correspondent aux colonnes (`a` à `h`) et les chiffres aux rangées (`1` à `8`).

## Limitations connues

Il s’agit d’un **prototype**, pas d’un moteur d’échecs complet :

1. La logique de validation ne couvre pas correctement toutes les règles (par exemple l’échec, l’échec et mat, le roque, la promotion, la prise en passant ou l’ensemble des déplacements légaux).
2. La gestion des tours et des messages échangés entre le client et le serveur est incomplète : le serveur envoie des messages textuels alors que le client attend des données d’échiquier, ce qui peut perturber l’affichage et le déroulement d’une partie.
3. Le client ne permet pas de préciser correctement l’adresse d’un serveur distant.
4. Les entrées utilisateur et les messages réseau nécessitent une validation plus robuste. La saisie actuelle d’un coup avec `scanf("%s", buffer)` n’impose pas de limite de taille.
5. Les sockets, threads et cas de déconnexion nécessitent des améliorations pour un usage fiable à plusieurs joueurs.

**Ne pas exposer ce prototype directement à Internet** sans corriger ces problèmes et réaliser des tests de sécurité.

## Pistes d’amélioration

- Définir un protocole réseau explicite (type de message, taille, joueur, état).
- Ajouter les paramètres `--host` et `--port` au client et au serveur.
- Corriger la validation des règles d’échecs et l’alternance des tours.
- Ajouter des tests unitaires des mouvements et des tests d’intégration réseau.
- Fournir un `Makefile` et une CI pour compiler automatiquement.
- Ajouter une capture d’écran du terminal et une démonstration de partie.

## Documentation

- [Rapport du projet](Report.pdf)
- [Présentation](Slides.pdf)

Ce dépôt illustre une approche pédagogique de la **programmation système en C**, de la **communication client–serveur TCP** et de la **gestion concurrente de sessions**.
