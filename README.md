# Projet PNL - terminus

Oskar Viljasaar [3000989] - Saalik Hatia [3000442]

## Introduction

Projet de l'UE PNL - 4l402 dont l'objectif est de réaliser un outil en ligne de commande.

L'application a deux parties:
- Client
- Module Noyau

## Mode d'emploi

Extraire le dossier dans le répertoire où se situent les sources de Linux 3.4.2

Dans le dossier extrait:
<pre><code>make</code></pre>

Importer les fichiers dans la machine virtuelle puis:
<pre><code>insmod terminusmod.ko</code></pre>

Pour lancer l'outil:
<pre><code>./terminus</code></pre>

La liste des commandes disponibles est visible grace à la commande:
<pre><code>help</code></pre>

Il est possible de lancer les commandes meminfo, modinfo, kill de manière asynchrone en utilisant le caractères _'&'_

**Commandes disponibles:**

<pre><code>list</code></pre>

Permet de lister les commandes en cours d'éxecution

<pre><code>fg id</code></pre>

Permet de récupérer les résultats des commandes lancées en asynchrone. Bloque jusqu'à ce que la commande donnée se termine

<pre><code>kill signal pid</code></pre>

Permet d'envoyer le signal *signal* au pid *pid*

<pre><code>wait pid [pid...]</code></pre>

Cette commande se bloque jusqu'à ce que l'un des processus passés en paramètre se termine

<pre><code>waitall pid [pid...]</code></pre>

**Commande supplementaire** Cette commande se bloque jusqu'a que **tous** les processus passés en paramètre se terminent

<pre><code>meminfo</code></pre>

Obtenir des informations concernant l'état de la mémoire

<pre><code>modinfo name</code></pre>

Renvoie les informations du module

## Traces d'éxecution

```
[root@vm-nmv client]# ./terminus
> list
> meminfo &
> modinfo terminusmod &
> modinfo terminusmod
Name	terminusmod
Version	(null)
Core	0xffffffffa000f000
0 arguments
> list
 1 MEMINFO
 2 MODINFO
 > fg 1
sending fg
TotalRam	512554 pages
SharedRam	2106 pages
FreeRam		488003 pages
BufferRam	2438 pages
TotalHigh	0 pages
FreeHigh	0 pages
Memory unit	4096 bytes
> fg 2
sending fg
Name	terminusmod
Version	(null)
Core	0xffffffffa000f000
0 arguments
>
```
## Ce qui a été fait

Ce projet est constitué de deux parties majeures:

### Le module noyau

Le module noyau crée un périphérique /dev/terminus qui est utilisé afin de communiquer avec le programme utilisateur.
Une fois ce périphérique créé, il reçoit par le biais d'ioctl des commandes à traiter du programme utilisateur.
Le handler d'ioctl se charge de recevoir et utilise des waitqueues ou workqueues, selon si le traitement est synchrone ou asynchrone,
et renvoie le résultat de l'opération au moment voulu.

### Le client

Le client est un simple intermédiaire entre l'utilisateur et le module. C'est à lui que revient la tâche d'afficher les informations
communiquées par le module. Pour ce faire, il se présente sous la forme d'une invite de commande. Lorsque qu'une commande est rentrée
par l'utilisateur, il se charge de préparer l'ensemble des paramètres néccessaires pour que le module puisse effectuer l'opération.

### Le cahier des charges

A été rempli dans son ensemble.

## Précisions

Notre module passe le checkpatch.pl sans erreurs:
```
../../linux-4.2.3/scripts/checkpatch.pl -f terminusmod.c
total: 0 errors, 0 warnings, 486 lines checked

terminusmod.c has no obvious style problems and is ready for submission.

../../linux-4.2.3/scripts/checkpatch.pl -f terminus.h 
total: 0 errors, 0 warnings, 107 lines checked

terminus.h has no obvious style problems and is ready for submission.

```
