/*
 * common.h
 *
 *  Created on: 18 avr. 2018
 *      Author: senaneuc
 */

#ifndef OWN_LIB_COMMON_H_
#define OWN_LIB_COMMON_H_

/*DUMBY*/
/*STATE SYSTEM*/
#define IDLE								0x06
#define RUN									0x07
#define DISABLED							0x08


/*WatchDog*/
#define WD_ACTIVE							0x55
#define WD_INACTIVE 						0x66

//Valeur doivent être un multiple de 5ms
#define WD_WINDOW_LOW						90
#define WD_WINDOW_HIGH 						150
#define WD_BASE_TIME						1000
#define WD_TRYING_NUMBER 					3

//Timeout
#define TIME_OUT 							180000 // 3 min

/*MOTOR*/
#define MOTOR_RIGHT							0x10
#define MOTOR_LEFT							0x20

/*STATE*/
#define MOTOR_FORWARD 						 0x01
#define MOTOR_BACKWARD 						 0x02
#define MOTOR_BREAK							 0x03
/*MOTOR_R*/
/*SPEED*/
#define SPEED_DEFAULT_R						 50
#define KiR									 0 // Integration coef
#define KpR									 0 // Proportional coef
/*MOTOR_L*/
/*SPEED*/
#define SPEED_DEFAULT_L						 50
#define KiL									 0 // Integration coef
#define KpL									 0 // Proportional coef



/*LED*/
/*LED COLOR*/
#define LED_RED 							0x01
#define LED_GREEN  							0x02
#define LED_ORANGE							0x03
#define LED_OFF 							0x04

/*LED DUTY*/
#define LED_DUTY_IDLE 						10
#define LED_DUTY_RUN						123
#define LED_DUTY_BLOCK						123


/*BATTERY*/
#define VOLTAGE_BUFFER_SIZE 				25
#define BATTERY_LOW							20
#define BATTERY_HIGH						100
#define BATTERY_MEDIUM						50

/*UART*/
#define EoT	 								'\r'



/*Config*/
#define CHECKSUM_ENABLE 					FALSE
#define TRAME_PDU							3											//802.15.4 protocol xbee
#define TRAME_SIZE_RECEIPT					12										// 3 byte of PDU and 9 byte used by 802.15.4
#define TRAME_SIZE_SEND						11
#define VERSION_FIRMWARE					2

/* Activity */
#define BUSY								0x1
#define NOT_BUSY							0x0

/*XBEE*/
/*SUCCESS*/
#define XBEE_SUCCESS						0
/*ERROR*/
#define XBEE_RECEIVE_INCOMPLETE 			-1
#define XBEE_INVALID_FRAME					-2
#define XBEE_SEND_FAILED					-3
#define XBEE_INVALID_ADDRESS				-4


/*MESSAGE*/
#define MES_PING							'p'
#define MES_STOP							'r'
#define MES_START_WD						'W'
#define MES_RESET_WD						'w'
#define MES_GET_VERS						'V'
#define MES_START_WWD						'u'
#define	MES_MOVE							'M'
#define	MES_ROTATE							'T'
#define	MES_BUSY							'b'
#define	MES_GET_VOLTAGE						'v'
#define MES_TURN_LEFT						'L'
#define MES_TURN_RIGHT						'R'

#define MES_UNKNOW_COM  					'C'
#define MES_ERROR_PARAM 					'E'
#define MES_VALID							'O'


/*WHEEL SPEED - POSITION*/
#define WHEEL_DIAMETER						32 //mm
#define IMPULSE_BY_LAPS						80
#define IMPULSE_BY_CM						7

#define SPEED_MAX							1100
#define SPEED_MIN							10000

#define DEFAULT_SPEED_STRAIGHT				50
#define DEFAULT_SPEED_ROTATION				50



#define Kp_position_R						0.2
#define Kp_position_L						0.2
#define Ki_position_R						0
#define Ki_position_L						0
#define Kd_position_R						0
#define Kd_position_L						0




#endif /* OWN_LIB_COMMON_H_ */
