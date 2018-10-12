/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   msg.h
 * Author: pehladik
 *
 * Created on 15 janvier 2018, 20:45
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define OPEN_COM_DMB  'o'
#define CLOSE_COM_DMB 'C'

#define DMB_PING                'p'
#define DMB_IDLE                'r'
#define DMB_START_WITHOUT_WD    'u'
#define DMB_START_WITH_WD       'W'
#define DMB_RELOAD_WD           'w'
#define DMB_GET_VBAT            'v'
#define DMB_IS_BUSY             'b'
#define DMB_MOVE                'M'
#define DMB_TURN                'T'
#define DMB_GO_FORWARD          'F'
#define DMB_GO_BACK             'B'
#define DMB_GO_LEFT             'L'
#define DMB_GO_RIGHT            'R'
#define DMB_STOP_MOVE           'S'

#define ROBOT_TIMED_OUT -3
#define ROBOT_UKNOWN_CMD -2
#define ROBOT_ERROR  -1
#define ROBOT_CHECKSUM  -4
#define ROBOT_OK    0

#define CAM_OPEN                    'A'
#define CAM_CLOSE                   'I'
#define CAM_ASK_ARENA               'y'
#define CAM_ARENA_CONFIRM           'x'
#define CAM_ARENA_INFIRM            'z'
#define CAM_COMPUTE_POSITION        'p'
#define CAM_STOP_COMPUTE_POSITION   's'

#define DMB_BAT_LOW 0
#define DMB_BAT_MEDIUM 1
#define DMB_BAT_HIGHT 2

#define DMB_BUSY 1
#define DMB_DO_NOTHING 0

#endif /* MSG_H */

