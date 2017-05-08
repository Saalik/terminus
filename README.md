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

La liste des commandes disponible est visible grace a la commande:
<pre><code>help</code></pre>

Il est possible de lancer les commandes meminfo, modinfo, kill de manière asynchrone en utilisant le caractère _'&'_

**Commandes disponibles:**

<pre><code>list</code></pre>

Permet de lister les commandes en court d'execution

<pre><code>fg id</code></pre>

Permet de récupérer les resultat des commandes lancées en asynchrone. Bloque jusqu'a que la commande donnée se termine

<pre><code>kill signal pid</code></pre>

Permet d'envoyer le signal *signal* au pid *pid*

<pre><code>wait pid [pid...]</code></pre>

Cette commande se bloque jusqu'a que l'un des processus passés en paramètre se termine

<pre><code>waitall pid [pid...]</code></pre>

**Commande supplementaire** Cette commande se bloque jusqu'a que **tous** les processus passés en paramètre se termine

<pre><code>meminfo</code></pre>

Obtenir des informations concernant l'état de la mémoire

<pre><code>modinfo name</code></pre>

Renvoie les informations du module

## Traces d'execution

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

Ce projet est continuer de deux parties majeures:

### Le module noyau

Le module noyau créer un périphérique dev/terminus qui est utilisé afin de communiquer avec le programme utilisateur.
Une fois ce périphérique créer il reçoit par le biais d'ioctl des commandes a traiter du programme utilisateur. 
Le handler d'ioctl se charge de recevoir et utilise des waitqueues ou workqueues, selon si le traitement est synchrone ou asynchrone,
et renvoie le résultat de l'opérations au moment voulut.

### Le client 

Le client est un simple intermédiaire entre l'utilisateurs et le module. C'est a lui que reviens la tache d'afficher les informations 
communiqués par le module. Pour ce faire il se présente sous la forme d'une invite de commande. Lorsque qu'une commande est rentrée 
par l'utilisateur il se charge de préparé l'ensemble des paramètres néccessaire pour le module puisse effectué l'opération.

### Le cachier des charges

Il a été rempli dans son ensemble

## Précisions

Notre module passe le checkpatch.pl sans erreurs:
```
../../linux-4.2.3/scripts/checkpatch.pl -f terminusmod.c
total: 0 errors, 0 warnings, 487 lines checked

terminusmod.c has no obvious style problems and is ready for submission.
```
