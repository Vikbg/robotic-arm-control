# Script De Demo

Ce document est un brouillon pour la demonstration orale.

## Trame Sur 3 Minutes

### 0:00 - 0:30 Introduction

"Ce projet est un bras robotique a 6 degres de liberte controle par une Arduino Uno et un Wii Nunchuck. Le bras utilise six servomoteurs et une alimentation externe pour fonctionner en securite."

### 0:30 - 1:10 Commandes

"Le joystick controle la base et un des bras selon la couche active. L'inclinaison du Nunchuck controle le poignet, et les boutons `C` et `Z` servent pour la pince et les gestes de mode."

### 1:10 - 1:40 Securite

"Pour eviter les problemes electriques et mecaniques, les servos sont alimentes par une source externe, toutes les masses sont reliees, et le logiciel impose des limites d'angle ainsi qu'une position home."

### 1:40 - 2:20 Demonstration Manuelle

- faire bouger lentement la base
- faire bouger l'epaule ou le coude
- faire bouger le poignet
- ouvrir et fermer la pince

### 2:20 - 2:45 Demonstration Automatique

"Le bras contient aussi un mode demo automatique et un mode replay pour rejouer un mouvement enregistre."

- lancer le mode demo
- lancer eventuellement le replay si prepare

### 2:45 - 3:00 Conclusion

"Ce projet combine le controle embarque, une alimentation securisee, et une interaction homme-machine. Plus tard, on pourrait ajouter des outils desktop, des essais en Rust embarque, ou une planification de mouvement plus avancee."

## Checklist De Repetition

- [ ] introduction en moins de 30 secondes
- [ ] commandes manuelles fluides
- [ ] mode demo lance correctement
- [ ] retour home fonctionne correctement
- [ ] conclusion dans le temps imparti
