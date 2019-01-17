# Software

Dans ce repertoire se trouvent les logiciels pour les differentes parties du projet de TP Temps reel (dumber)
- robot: Contient le code source du firmware executé par le robot (STM32 / TrueStudio)
- chargeur: Contient le code executé dans le boitier du chargeur de batterie du robot (STM32 / TrueStudio)
- raspberry: Contient le code du superviseur temps reel s'executant sur la raspberry et en charge du controle de la camera, et du pilotage du robot (via la liaison serie)
- monitor: client C# pour la communication avec le superviseur
- install-opencv.sh: Script pour installer et deployer opencv avec support des aruco et bibliotheque raspicam
