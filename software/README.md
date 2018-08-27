# Software

Dans ce repertoire se trouvent les logiciels pour les differentes parties du projet dumber
- robot: Contient le code source du firmware executé par le robot (STM32 / Keil)
- chargeur: Contient le code executé dans le boitier du chargeur de batterie du robot (STM32 / Keil)
- raspberry: Contient le code du superviseur temps reel s'executant sur la raspberry et en charge du controle de la camera, et du pilotage du robot (via la liaison serie)
- UserinterfaceWeb: interface Web pour la surveillance du systeme: connexion au superviseur RT, control et affichage de l'etat du robot, retour d'image.
- android: Application android pour le pilotage du robot via un module bluetooth à la place du module XBEE de base sur le robot.
