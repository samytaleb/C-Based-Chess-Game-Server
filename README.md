Bonjour,
Voici les instructions à suivre afin de pouvoir lancer convenablement notre projet de création d'un serveur de jeu d'échecs:

1. Une fois le fichier TAR téléchargé, il faut se placer dans le répértoire du dossier extrait pour y compiler les différents fichiers (server.c et client.c) en configurant le port. On utilisera la commande suivante: ./setup.sh
Cette commande permet à la fois de configurer le port qui sera utilisé et de compiler les fichiers.

2. Si vous souhaitez compiler à nouveau les fichiers par la suite, on utilisera la commande de compilation basique suivante: ./compilation.sh

3. Une fois les fichiers compilé, il faut récupérer l'IP de la machine qui éxécutera le fichier serveur. On utilisera la commande suivante: ip a

4. L'IP étant récupéré, on peut éxécuter le fichier serveur (server) dans un premier terminal avec la commande suivante : ./server
Une fois éxécuter, on peut choisir l'option 1 du menu pour créer une partie.

5. On ouvrira par la suite deux terminaux qui serviront d'interface client pour y éxécuter le fichier client (client) avec la commande suivante: ./client adresseip , où adresseip est l'adresse IP récupéré à l'étape 3. Exemple: ./client 19.0.2.17

6. Vous pouvez maintenant apprécier votre partie de jeu d'échecs !
