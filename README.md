# Dumber

## 2017-2018 :
Problème : 
- Watchdog du robot ne fonctionne pas
- Interface graphique qui a des etats mal gérée (je voulais le reprendre en utilisant les 'stores' de vuex pour éviter ça).
- Il faudrait une execution de l'interface (srv nodejs) par défaut au démarage de la RPI.
 	L'exectution devrait se faire dans l'user space de linux dans le cas d'utilisation d'un noyau xenomai (co-kernel).
	=> Potentiellement passer d'un noyau preempt_rt (actuel) à un noyau xenomai

## Repertoires
- hardware : contient les plans pour la partie mecanique du robot et de son chargeur
- software: rassemble les parties logicielles du robot, du chargeur, les bibliotheques et superviseur coté raspberry et l'interface Web
- pcb: contient les plans de conception des PCB du robot, du chargeur, de l'adaptateur Xbee pour la raspberry  et les plans des CAP du robot
- incubateur: projet en incubation. Actuellement, contient des essais sur les aruco (pour la detection des robots) ou le portage du firmware du robot sous freertos.
