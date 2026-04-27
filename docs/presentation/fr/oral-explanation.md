# Explication Orale

## But Du Projet

Ce projet controle un bras robotique a 6 degres de liberte avec une Arduino Uno et un Wii Nunchuck.

Les objectifs sont :

- faire bouger six articulations de maniere fiable
- garder un cablage d'alimentation sur
- proposer une commande simple pour les demonstrations
- conserver un mode diagnostic via le port serie USB

## Architecture

### Entrees

- Wii Nunchuck en `I2C`
- joystick pour la base et l'epaule ou le coude
- accelerometre pour le roulis et le tangage du poignet
- boutons `C` et `Z` pour la pince et les changements de mode

### Controleur

- l'Arduino Uno execute le firmware principal dans `arduino/robotic_arm/robotic_arm.ino`
- le programme calcule des positions cibles
- le lissage evite les mouvements trop brusques
- les limites d'angle protegent le bras

### Sorties

- six sorties PWM pour les servos
- base sur `D3`
- epaule sur `D5`
- coude sur `D6`
- rotation poignet sur `D9`
- inclinaison poignet sur `D10`
- pince sur `D11`

## Modes De Controle

### Mode Nunchuck

- mode normal de controle en direct
- le joystick et l'inclinaison pilotent le bras
- `C + Z` permet d'activer la demo ou de revenir en position home

### Mode Serie

- utile pour le diagnostic et les tests manuels
- permet d'envoyer des commandes comme `set shoulder 140`
- pratique pour calibrer une articulation a la fois

### Mode Demo

- sequence automatique pour la presentation
- utile pour montrer le projet sans pilotage manuel permanent

### Mode Replay

- rejoue une courte sequence enregistree depuis le mode manuel ou serie
- utile pour repeter un mouvement simple sans reecrire la demo

## Securite

### Securite Electrique

- les servos utilisent une alimentation externe `5V-6V`
- la broche `5V` de l'Arduino ne doit pas alimenter les six servos
- toutes les masses doivent etre reliees ensemble
- le Nunchuck est alimente en `3.3V`

### Securite Mecanique

- chaque articulation a des limites min et max en logiciel
- le bras possede une position home
- le lissage reduit les mouvements brusques
- le firmware peut revenir a une position sure au lieu de laisser les servos dans une position aleatoire

## Pourquoi Cette Architecture

- l'Arduino Uno est simple, peu couteuse, et facile a presenter
- le Wii Nunchuck regroupe joystick, boutons, et capteur de mouvement
- les commandes serie rendent le debogage plus rapide
- le code reste assez modulaire pour ajouter plus tard des outils Rust ou d'autres interfaces

## Plan Court Pour La Presentation

1. Presenter les blocs materiels : controleur, alimentation, entrees, actionneurs.
2. Expliquer les principaux modes de controle.
3. Montrer les choix de securite : alimentation externe, masse commune, limites d'angle, position home.
4. Faire une demonstration manuelle puis automatique.
5. Terminer avec les ameliorations possibles : replay, outils desktop, ou essais en Rust embarque.

## Questions Probables

### Pourquoi ne pas alimenter les servos depuis l'Arduino ?

Parce que six servos peuvent consommer bien plus de courant que ce que le regulateur de l'Arduino peut fournir en securite.

### Pourquoi utiliser des limites logicielles ?

Parce qu'elles reduisent le risque de taper une butee mecanique ou d'endommager le bras.

### Pourquoi utiliser un Nunchuck ?

Parce qu'il combine joystick, boutons, et capteurs de mouvement dans une seule manette compacte.

### Pourquoi garder aussi des commandes serie ?

Parce que le port serie reste le moyen le plus simple pour tester, calibrer et deboguer chaque articulation independamment.
