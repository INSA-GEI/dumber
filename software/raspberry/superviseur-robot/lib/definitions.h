/*
 * Copyright (C) 2018 dimercur
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file      definitions.h
 * \author    PE.Hladik
 * \version   1.0
 * \date      06/06/2017
 * \brief     Various constants used in destjil project
 */


#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define OPEN_COM_DMB            'o'
#define CLOSE_COM_DMB           'C'

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

#define ROBOT_TIMED_OUT         -3
#define ROBOT_UKNOWN_CMD        -2
#define ROBOT_ERROR             -1
#define ROBOT_CHECKSUM          -4
#define ROBOT_OK                0

#define CAM_OPEN                    'A'
#define CAM_CLOSE                   'I'
#define CAM_ASK_ARENA               'y'
#define CAM_ARENA_CONFIRM           'x'
#define CAM_ARENA_INFIRM            'z'
#define CAM_COMPUTE_POSITION        'p'
#define CAM_STOP_COMPUTE_POSITION   's'

#define DMB_BAT_LOW             0
#define DMB_BAT_MEDIUM          1
#define DMB_BAT_HIGH            2

#define DMB_BUSY                1
#define DMB_DO_NOTHING          0

#endif /* MSG_H */

