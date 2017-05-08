# terminus - Projet PNL

Oskar Viljasaar [3000989] - Saalik Hatia [3000442]

## Introduction

Projet de l'UE PNL - 4l402 dont l'objectif est de réaliser un outils en ligne de commande.

Commandes a implémenter:

list
fg <id>
kill <signal> <pid>
wait <pid> [<pid>...]
meminfo
modinfo <name>

L'application a deux parties:
- Client
- Module

## Mode d'emploi

Extraire le dossier dans le même répertoire ou se situe les sources de Linux 3.4.2

Dans le dossier extrait:
make

Importer les fichiers dans la machine virtuelle:
insmod terminusmod.ko

Puis ./terminus pour lancer l'outil

La liste des commandes disponible est visible grace a la commande help

## Traces d'execution


## Ce qui a été fait

Toutes les commandes ont été implémenter 

## What's left:


