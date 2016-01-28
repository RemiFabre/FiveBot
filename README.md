# FiveBot

Le FiveBot est un robot holonome possédant des roues Mecanum.
Au-dela du kit initial, il est équipé en sus d'un Raspberry Pi sur lequel se branche un LiDAR.

## Composants

Le dépôt est découpé en quatre dossiers :
  - `arduino` contient le code C++ qui tourne sur l'ATmega328p à bord du robot.  
    Le programme gère la lecture des encodeurs et la régulation en vitesse des moteurs.
    Ce code est basé en partie sur le SDK Arduino (la carte s'apparente à un Arduino Duemilanove).
  - `api` contient le code Python s'interfaçant directement avec la carte.  
    La bibliothèque donne accès à l'odométrie et permet une contrôle en position et en vitesse du robot.
    L'interface graphique permet de débugger la carte avec des indicateurs de performance des composants matériels et logiciels.
    La communication avec la carte se fait via une liaison série USB.
  - `raspberry` contient le code Python qui permet au robot d'être utilisable à distance.  
    Ce code est basé sur ROS et expose sous forme de noeuds les fonctionnalités de l'API et les relevés du LiDAR.
  - `pc` contient de code Python qui commande à distance le robot.  
    Il permet de définir des objectifs en position et en vitesse, d'afficher les relevés du LiDAR et de contrôler manuellement le robot via des joysticks.
    Ce code basé sur ROS peut communiquer en Wifi avec le Raspberry Pi à bord du robot.

## Arduino

Le code qui s'exécute sur le microprocesseur du robot gère le relevé des encodeurs via des interruptions ainsi qu'un calcul d'odométrie à intervalle régulier via un timer. Le calcul de l'odométrie réinitialise le buffer de mouvement associé à chaque encodeur incrémental. La carte n'étant pas assez puissante pour effectuer des calculs trigonométriques, il est nécessaire de transmettre les valeurs de position et angle de la voiture assez rapidement au Raspberry Pi qui va pouvoir calculer la position absolue de la voiture. Cette transmission cause en conséquence une remise à zéro de l'odométrie calculée sur la carte.

 1. Lecture des encodeurs
 2. Calcul de l'évolution de position et d'angle
 3. Calcul trigonométrique de la position absolue du robot

(2) remet à zéro le buffer utilisé en (1) et (3) remet à zéro le buffer utilisé en (2).

Quand le processeur n'est pas en train de gérer une interruption pour (1) et (2), il transmet via la liaison série les données permettant au Raspberry Pi d'effectuer (3), et gère les éventuelles commandes arrivant dans l'autre sens.

## API

La bilbiothèque receptionne les données transmises régulièrement par la carte via un protocole binaire simple. Chaque trame dans un sens comme dans l'autre est composée d'un octet de start, d'un octet désignant la commande, de données échappées si besoin et d'un octet de stop. Elle traite certaines données brutes relevées par la carte et les convertit pour une utilisation externe. Elle permet aussi de contrôler le robot en vitesse et en position.

Une interface graphique permet de visualiser toutes les informations transmises par la voiture en temps réel à des fins de débuggage.
