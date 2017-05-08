# terminus - Projet PNL

Oskar Viljasaar [3000989] - Saalik Hatia [3000442]

## Introduction

Projet de l'UE PNL - 4l402 dont l'objectif est de réaliser un outil en ligne de commande.

Commandes a implémenter:

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

Il est possible de lancer les commandes de manière asynchrone en utilisant le caractère _&_


Commandes disponibles: 

<pre><code>_list_</code></pre>

Permet de lister les commandes en court d'execution

<pre><code>_fg id_</code></pre>

Permet de récupérer les resultat des commandes lancées en asynchrone. Elle bloque jusqu'a que la commande donnée se termine

<pre><code>_kill signal pid_</code></pre>

Permet d'envoyé le signal *signal* au pid *pid* 

<pre><code>_wait \<pid\> \[\<pid\>...\]_</code></pre>

Cette commande se bloque jusqu'a que l'un des processus passés en paramètre se termine

<pre><code>_meminfo_</code></pre>

Obtenir les informations concernant l'état de la mémoire

<pre><code>_modinfo \<name\>_</code></pre>

Renvoie les informations du module

## Traces d'execution


## Ce qui a été fait

Toutes les commandes ont été implémenter 

## What's left:


