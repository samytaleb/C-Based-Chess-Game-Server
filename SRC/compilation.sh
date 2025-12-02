#!/bin/bash

# Compilation du serveur
gcc server.c -o server -lpthread

# Compilation du client
gcc client.c -o client

