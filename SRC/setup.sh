#!/bin/bash

# Mettre à jour les paquets
sudo apt update

# Installer UFW
sudo apt install -y ufw

# Activer UFW
sudo ufw enable

# Autoriser le port 9999 pour TCP
sudo ufw allow 9999/tcp

# Vérifier le statut de UFW
sudo ufw status

# Compilation du serveur
gcc server.c -o server -lpthread

# Compilation du client
gcc client.c -o client

