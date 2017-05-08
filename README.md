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

<pre><code>
root@vm-nmv client]# ./terminus
> modinfo &
> modinfo &
> meminfo &
> list
 2 MODINFO 3 MODINFO 4 MODINFO 5 MEMINFO
> fg 2
sending fg
[ 1108.559857] we should not be here, handler w/id 3
[ 1108.560871] we should not be here, handler w/id 4
[ 1108.561934] we should not be here, handler w/id 5
Name	�k
Version
Core	(nil)
0 arguments
> fg 3
sending fg
[ 1114.175909] we should not be here, handler w/id 4
[ 1114.176632] we should not be here, handler w/id 5
Name	
Version
Core	(nil)
0 arguments
> meminfo &
> list
 4 MODINFO 5 MEMINFO 9 MEMINFO
> fg 5
sending fg
[ 1172.117414] we should not be here, handler w/id 4
[ 1172.118043] we should not be here, handler w/id 9
TotalRam	512554 pages
SharedRam	2106 pages
FreeRam		488081 pages
BufferRam	2427 pages
TotalHigh	0 pages
FreeHigh	0 pages
Memory unit	4096 bytes
> fg 9
sending fg
[ 1174.969562] we should not be here, handler w/id 4
TotalRam	512554 pages
SharedRam	2106 pages
FreeRam		488078 pages
BufferRam	2428 pages
TotalHigh	0 pages
FreeHigh	0 pages
Memory unit	4096 bytes
> fg 4
sending fg
Name	
Version
Core	(nil)
0 arguments
>
</code></pre>

## Ce qui a été fait

Toutes les commandes ont été implémenter

## What's left:
