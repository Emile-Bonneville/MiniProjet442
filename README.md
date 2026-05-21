# Jeu de Dames en Réseau (FreeRTOS & UDP)

## Description du projet
L'objectif de ce projet est de mettre en œuvre une communication réseau UDP sur une carte STM32F7. Cela permet à deux joueurs de s'affronter au jeu de dames en connectant deux cartes sur un même routeur. 

Le jeu de dames a initialement été développé par Térence Marchi et est disponible sur [ce dépôt GitHub](https://github.com/terencemarchi/projet-442-jeux).

## Architecture FreeRTOS

L'intégration d'un système d'exploitation temps réel (RTOS) a permis de découpler l'interface utilisateur, la logique du jeu et la pile de communication en plusieurs tâches indépendantes :

*   **Tâche Affichage (`Affichage_Jeu`) :** Gère l'interface graphique sur l'écran LCD, la logique de navigation dans les menus et la détection des appuis sur l'écran.
*   **Tâche Logique (`Logique_jeu`) :** Gère les règles du jeu. Elle convertit les coups locaux en chaînes de caractères prêtes à être transmises et applique les coups reçus de l'adversaire.
*   **Tâche Réception UDP (`udpReception`) :** Écoute les paquets entrants sur le réseau et les place dans une file d'attente (`QueueReceptionUDP`) à destination de la logique du jeu.
*   **Tâche Émission UDP (`udpEmission`) :** Reste en attente de données dans la file d'attente (`QueueEmissionUDP`) et transmet les coups locaux à l'adresse IP de la seconde carte dès qu'ils sont disponibles.
*   **Tâche Système (`defaultTask`) :** Assure l'initialisation de la pile logicielle LwIP au démarrage du système.

---

## Limitations actuelles et évolutions possibles

*   **Gestion des adresses IP :** L'adresse IP de la carte distante est actuellement codée en dur dans le logiciel.
*   **Absence de DHCP :** Le protocole DHCP est désactivé, ce qui demande d'adapter le code si une des cartes change ou si le système d'adresse IP du routeur n'est pas le même.
*   **Risque de conflit :** L'affectation statique présente un risque de conflit d'adresses IP sur le réseau local si une autre machine utilise déjà la même adresse.
*   **Évolution :** Intégrer un client DHCP ou implémenter un mécanisme de récupération des adresses IP des cartes au démarrage.
