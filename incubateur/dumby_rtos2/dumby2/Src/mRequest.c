/*
 * manage_request.c
 *
 *  Created on: 18 avr. 2018
 *      Author: senaneuc
 */
#include "mRequest.h"
#include "common.h"
#include "sharedData.h"
#include "Motor.h"

Consigne consigneRotation;
Consigne consigneForward;
Consigne nouvelleConsigne;


char manageMessage(char * mes){




	int cptWd = -1;
	switch(mes[8]){
		case MES_PING:
					return MES_VALID;
		case MES_STOP:
				if(getDumberState() == RUN){
					setDumberState(IDLE);
					if(getDumberWdActive() == WD_ACTIVE){
						setDumberWdActive(WD_INACTIVE);
						setDumberCptWd(0);
					}
					return MES_VALID;
				}
				else{
					return MES_ERROR_PARAM;
				}
		case MES_START_WD:
			if(getDumberState() == IDLE){
				setDumberState(RUN);
				setDumberWdActive(WD_ACTIVE);
				return MES_VALID;
			}
			else
				return MES_UNKNOW_COM;

		case MES_RESET_WD:
			cptWd = getDumberCptWd();
			if((cptWd%WD_BASE_TIME) > WD_WINDOW_LOW && (cptWd%WD_BASE_TIME) < WD_WINDOW_HIGH){ // Condition de remise à zero du compteur de wd ( valeur dans une fenetre)
				setDumberCptWd(0);
				return MES_VALID;
			}
			else{
				return MES_ERROR_PARAM;
			}

		case MES_GET_VERS:
				return VERSION_FIRMWARE;

		case MES_START_WWD:
			if(getDumberState() == IDLE){
				setDumberState(RUN);
				return MES_VALID;
			}
			else{
				return MES_ERROR_PARAM;
			}

		case MES_MOVE:
			if(getDumberState() == RUN){
				distanceR = 0;
				distanceL = 0;
				consigneForward.motorLSpeed = DEFAULT_SPEED_STRAIGHT;
				consigneForward.motorRSpeed = DEFAULT_SPEED_STRAIGHT;
				consigneForward.motorRDistance = (char)mes[9];
				consigneForward.motorLDistance = (char)mes[9];
				setDumberConsigne(&consigneForward);
				return MES_VALID;
			}else{
				return MES_ERROR_PARAM;
			}
			break;

		case MES_ROTATE:
			if(getDumberState() == RUN){
				distanceL = 0;
				distanceR = 0;
				consigneRotation.motorLSpeed = DEFAULT_SPEED_ROTATION;
				consigneRotation.motorRSpeed = DEFAULT_SPEED_ROTATION;
				consigneRotation.motorRDistance = (int)mes[9];
				consigneRotation.motorLDistance = -(int)mes[9];
				setDumberConsigne(&consigneRotation);
				return MES_VALID;
			}
			else{
				return MES_ERROR_PARAM;
			}
			break;

		case MES_TURN_RIGHT:
			if(getDumberState() == RUN){
				nouvelleConsigne = getDumberConsigne();
				nouvelleConsigne.motorRSpeed += (char)mes[9];
				setDumberConsigne(&nouvelleConsigne);
				return MES_VALID;
			}else{
				return MES_ERROR_PARAM;
			}
			break;

		case MES_TURN_LEFT:
			if(getDumberState() == RUN){
				nouvelleConsigne = getDumberConsigne();
				nouvelleConsigne.motorLSpeed += (char)mes[9];
				setDumberConsigne(&nouvelleConsigne);
				return MES_VALID;
			}else{
				return MES_ERROR_PARAM;
			}

			break;

		case MES_BUSY:
			return getDumberActivity();
			break;

		case MES_GET_VOLTAGE:
				return getDumberBattery();

		default:
			return MES_UNKNOW_COM;
	}
	return 0;
}
