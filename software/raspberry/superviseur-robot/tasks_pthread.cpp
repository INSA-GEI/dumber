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

#include "tasks_pthread.h"
#include <time.h>

#ifdef __WITH_PTHREAD__

// Déclaration des priorités des taches
#define PRIORITY_TSERVER 30
#define PRIORITY_TOPENCOMROBOT 20
#define PRIORITY_TMOVE 10
#define PRIORITY_TSENDTOMON 25
#define PRIORITY_TRECEIVEFROMMON 22
#define PRIORITY_TSTARTROBOT 20

/*
 * Some remarks:
 * 1- This program is mostly a template. It shows you how to create tasks, semaphore
 *   message queues, mutex ... and how to use them
 * 
 * 2- semDumber is, as name say, useless. Its goal is only to show you how to use semaphore
 * 
 * 3- Data flow is probably not optimal
 * 
 * 4- Take into account that ComRobot::Write will block your task when serial buffer is full,
 *   time for internal buffer to flush
 * 
 * 5- Same behavior existe for ComMonitor::Write !
 * 
 * 6- When you want to write something in terminal, use cout and terminate with endl and flush
 * 
 * 7- Good luck !
 */

void Tasks::Init() {
    int status;

    /* Open com port with STM32 */
    cout << "Open serial com (";
    status = robot.Open();
    cout << status;
    cout << ")" << endl;

    if (status >= 0) {
        // Open server

        status = monitor.Open(SERVER_PORT);
        cout << "Open server on port " << (SERVER_PORT) << " (" << status << ")" << endl;

        if (status < 0) throw std::runtime_error {
            "Unable to start server on port " + std::to_string(SERVER_PORT)
        };
    } else
        throw std::runtime_error {
        "Unable to open serial port /dev/ttyS0 "
    };
}

void Tasks::Run() {
    threadTimer = new thread((void (*)(void*)) & Tasks::TimerTask, this);
    threadServer = new thread((void (*)(void*)) & Tasks::ServerTask, this);

    //    threadSendToMon=new thread((void (*)(void*)) &Tasks::SendToMonTask,this);

    //
    //    Camera camera=Camera(sm);
    //    cout << "Try opening camera"<<endl<<flush;
    //    if (camera.Open()) cout<<"Camera opened successfully"<<endl<<flush;
    //    else cout<<"Failed to open camera"<<endl<<flush;
    //    
    //    counter = 0;
    //    while (1) {
    //        Img image=camera.Grab();
    //        
    //        counter++;
    //        
    //        if (flag == true) {
    //            cout<< "Image info: "<<image.ToString()<<endl<<flush;
    //            cout << "FPS = "<<to_string(counter)<<endl<<flush;
    //            flag=false;
    //            counter=0;
    //        }
    //    }


    //    msgSend = ComRobot::Ping();
    //    cout << "Send => " << msgSend->ToString() << endl << flush;
    //    msgRcv = robot.SendCommand(msgSend, MESSAGE_ANSWER_ACK, 3);
    //    cout << "Rcv <= " << msgRcv->ToString() << endl << flush;
    //
    //    delete(msgRcv);
    //
    //    msgSend = ComRobot::StartWithoutWD();
    //    cout << "Send => " << msgSend->ToString() << endl << flush;
    //    msgRcv = robot.SendCommand(msgSend, MESSAGE_ANSWER_ACK, 3);
    //    cout << "Rcv <= " << msgRcv->ToString() << endl << flush;
    //
    //    delete(msgRcv);
    //
    //    msgSend = ComRobot::Move(1000);
    //    cout << "Send => " << msgSend->ToString() << endl << flush;
    //    msgRcv = robot.SendCommand(msgSend, MESSAGE_ANSWER_ACK, 3);
    //    cout << "Rcv <= " << msgRcv->ToString() << endl << flush;
    //
    //    delete(msgRcv);
    //
    //    msgSend = ComRobot::GetBattery();
    //    cout << "Send => " << msgSend->ToString() << endl << flush;
    //    msgRcv = robot.SendCommand(msgSend, MESSAGE_ROBOT_BATTERY_LEVEL, 3);
    //    cout << "Rcv <= " << msgRcv->ToString() << endl << flush;
    //
    //    delete(msgRcv);
    cout << "Tasks launched" << endl << flush;
}

void Tasks::Stop() {
    monitor.Close();
    robot.Close();
}

void Tasks::ServerTask(void *arg) {
    Message *msgRcv;
    Message *msgSend;
    bool isActive = true;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    while (isActive) {
        msgRcv = NULL;
        msgSend = NULL;

        msgRcv = monitor.Read();
        cout << "Rcv <= " << msgRcv->ToString() << endl << flush;

        if (msgRcv->CompareID(MESSAGE_ROBOT_COM_OPEN)) msgSend = new Message(MESSAGE_ANSWER_ACK);
        if (msgRcv->CompareID(MESSAGE_ROBOT_COM_CLOSE)) msgSend = new Message(MESSAGE_ANSWER_ACK);

        if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITH_WD)) msgSend = new Message(MESSAGE_ANSWER_ACK);
        if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITHOUT_WD)) msgSend = new Message(MESSAGE_ANSWER_ACK);

        if (msgRcv->CompareID(MESSAGE_ROBOT_COM_CLOSE)) isActive = false;

        if (msgRcv->CompareID(MESSAGE_CAM_OPEN)) {
            sendImage = true;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
        }

        if (msgRcv->CompareID(MESSAGE_CAM_CLOSE)) {
            sendImage = false;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
        }

        if (msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_START)) {
            sendPosition = true;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
        }

        if (msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_STOP)) {
            sendPosition = false;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
        }

        if (msgRcv->CompareID(MESSAGE_ROBOT_BATTERY_GET)) msgSend = new MessageBattery(MESSAGE_ROBOT_BATTERY_LEVEL, BATTERY_FULL);

        if (msgRcv->CompareID(MESSAGE_CAM_ASK_ARENA)) showArena = true;
        if (msgRcv->CompareID(MESSAGE_CAM_ARENA_CONFIRM)) showArena = false;
        if (msgRcv->CompareID(MESSAGE_CAM_ARENA_INFIRM)) showArena = false;

        if (msgSend != NULL) monitor.Write(msgSend);
        delete(msgRcv);
    }
}

void Tasks::TimerTask(void* arg) {
    struct timespec tim, tim2;
    Message *msgSend;
    int counter;
    int cntFrame = 0;
    Position pos;
    Arena arena;
    
    tim.tv_sec = 0;
    tim.tv_nsec = 50000000; // 50ms (20fps)

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    Camera camera = Camera(sm, 20);
    cout << "Try opening camera" << endl << flush;
    if (camera.Open()) cout << "Camera opened successfully" << endl << flush;
    else {
        cout << "Failed to open camera" << endl << flush;
        
        exit(0);
    }

    pos.angle = 0.0;
    pos.robotId = -1;
    pos.center = cv::Point2f(0, 0);
    pos.direction = cv::Point2f(0, 0);
    
    while (1) {
        //std::this_thread::sleep_for(std::chrono::seconds )
        //sleep(1);
        //        if (nanosleep(&tim, &tim2) < 0) {
        //            printf("Nano sleep system call failed \n");
        //            return;
        //        }

        //        counter++;
        //        if (counter>=10) {
        //            flag=true;
        //            counter=0;
        //        }
        //mutexTimer.unlock();
        Img image = camera.Grab(); // 15fps

        if (sendPosition == true) {
            counter++;

            if (counter >= 1) { // div =15
                counter = 0;
                
                //if (!arena.IsEmpty()) {
                image.dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(3));
                         
                std::list<Position> poses = image.SearchRobot(arena);
                cout << "Nbr of pos detected: " << to_string(poses.size()) << endl << flush;

                if (poses.size() > 0) {
                    Position firstPos = poses.front();

                    pos.angle = firstPos.angle;
                    pos.robotId = firstPos.robotId;
                    pos.center = firstPos.center;
                    pos.direction = firstPos.direction;
                } else {
                    // Nothing found
                    pos.angle = 0.0;
                    pos.robotId = -1;
                    pos.center = cv::Point2f(0,0);
                    pos.direction = cv::Point2f(0,0);
                }

                MessagePosition *msgp = new MessagePosition(MESSAGE_CAM_POSITION, pos);
                monitor.Write(msgp);
                cout << "Position sent" << endl << flush;
            }
        }
        
        if (sendImage == true) {
            if (showArena) {
                arena = image.SearchArena();

                if (!arena.IsEmpty()) image.DrawArena(arena);
                else cout << "Arena not found" << endl << flush;
            }

            if (sendPosition == true) {
                image.DrawRobot(pos);
            }
             
            if (!arena.IsEmpty()) image.DrawArena(arena);
            
            MessageImg *msg = new MessageImg(MESSAGE_CAM_IMAGE, &image);

            monitor.Write(msg);
            cntFrame++;
            cout << "cnt: " << to_string(cntFrame) << endl << flush;
        }
    }
}

void Tasks::SendToMonTask(void* arg) {
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    while (1) {

    }
}

//void Tasks::f_sendToMon(void * arg) {
//    int err;
//    MessageToMon msg;
//
//    /* INIT */
//    RT_TASK_INFO info;
//    rt_task_inquire(NULL, &info);
//    printf("Init %s\n", info.name);
//    rt_sem_p(&sem_barrier, TM_INFINITE);
//
//#ifdef _WITH_TRACE_
//    printf("%s : waiting for sem_serverOk\n", info.name);
//#endif
//    rt_sem_p(&sem_serverOk, TM_INFINITE);
//    while (1) {
//
//#ifdef _WITH_TRACE_
//        printf("%s : waiting for a message in queue\n", info.name);
//#endif
//        if (rt_queue_read(&q_messageToMon, &msg, sizeof (MessageToRobot), TM_INFINITE) >= 0) {
//#ifdef _WITH_TRACE_
//            printf("%s : message {%s,%s} in queue\n", info.name, msg.header, (char*)msg.data);
//#endif
//
//            send_message_to_monitor(msg.header, msg.data);
//            free_msgToMon_data(&msg);
//            rt_queue_free(&q_messageToMon, &msg);
//        } else {
//            printf("Error msg queue write: %s\n", strerror(-err));
//        }
//    }
//}
//
//void Tasks::f_receiveFromMon(void *arg) {
//    MessageFromMon msg;
//    int err;
//
//    /* INIT */
//    RT_TASK_INFO info;
//    rt_task_inquire(NULL, &info);
//    printf("Init %s\n", info.name);
//    rt_sem_p(&sem_barrier, TM_INFINITE);
//
//#ifdef _WITH_TRACE_
//    printf("%s : waiting for sem_serverOk\n", info.name);
//#endif
//    rt_sem_p(&sem_serverOk, TM_INFINITE);
//    do {
//#ifdef _WITH_TRACE_
//        printf("%s : waiting for a message from monitor\n", info.name);
//#endif
//        err = receive_message_from_monitor(msg.header, msg.data);
//#ifdef _WITH_TRACE_
//        printf("%s: msg {header:%s,data=%s} received from UI\n", info.name, msg.header, msg.data);
//#endif
//        if (strcmp(msg.header, HEADER_MTS_COM_DMB) == 0) {
//            if (msg.data[0] == OPEN_COM_DMB) { // Open communication supervisor-robot
//#ifdef _WITH_TRACE_
//                printf("%s: message open Xbee communication\n", info.name);
//#endif
//                rt_sem_v(&sem_openComRobot);
//            }
//        } else if (strcmp(msg.header, HEADER_MTS_DMB_ORDER) == 0) {
//            if (msg.data[0] == DMB_START_WITHOUT_WD) { // Start robot
//#ifdef _WITH_TRACE_
//                printf("%s: message start robot\n", info.name);
//#endif 
//                rt_sem_v(&sem_startRobot);
//
//            } else if ((msg.data[0] == DMB_GO_BACK)
//                    || (msg.data[0] == DMB_GO_FORWARD)
//                    || (msg.data[0] == DMB_GO_LEFT)
//                    || (msg.data[0] == DMB_GO_RIGHT)
//                    || (msg.data[0] == DMB_STOP_MOVE)) {
//
//                rt_mutex_acquire(&mutex_move, TM_INFINITE);
//                robotMove = msg.data[0];
//                rt_mutex_release(&mutex_move);
//#ifdef _WITH_TRACE_
//                printf("%s: message update movement with %c\n", info.name, robotMove);
//#endif
//
//            }
//        }
//    } while (err > 0);
//
//}
//
//void Tasks::f_openComRobot(void * arg) {
//    int err;
//
//    /* INIT */
//    RT_TASK_INFO info;
//    rt_task_inquire(NULL, &info);
//    printf("Init %s\n", info.name);
//    rt_sem_p(&sem_barrier, TM_INFINITE);
//
//    while (1) {
//#ifdef _WITH_TRACE_
//        printf("%s : Wait sem_openComRobot\n", info.name);
//#endif
//        rt_sem_p(&sem_openComRobot, TM_INFINITE);
//#ifdef _WITH_TRACE_
//        printf("%s : sem_openComRobot arrived => open communication robot\n", info.name);
//#endif
//        err = open_communication_robot();
//        if (err == 0) {
//#ifdef _WITH_TRACE_
//            printf("%s : the communication is opened\n", info.name);
//#endif
//            MessageToMon msg;
//            set_msgToMon_header(&msg, (char*)HEADER_STM_ACK);
//            write_in_queue(&q_messageToMon, msg);
//        } else {
//            MessageToMon msg;
//            set_msgToMon_header(&msg, (char*)HEADER_STM_NO_ACK);
//            write_in_queue(&q_messageToMon, msg);
//        }
//    }
//}
//
//void Tasks::f_startRobot(void * arg) {
//    int err;
//
//    /* INIT */
//    RT_TASK_INFO info;
//    rt_task_inquire(NULL, &info);
//    printf("Init %s\n", info.name);
//    rt_sem_p(&sem_barrier, TM_INFINITE);
//
//    while (1) {
//#ifdef _WITH_TRACE_
//        printf("%s : Wait sem_startRobot\n", info.name);
//#endif
//        rt_sem_p(&sem_startRobot, TM_INFINITE);
//#ifdef _WITH_TRACE_
//        printf("%s : sem_startRobot arrived => Start robot\n", info.name);
//#endif
//        err = send_command_to_robot(DMB_START_WITHOUT_WD);
//        if (err == 0) {
//#ifdef _WITH_TRACE_
//            printf("%s : the robot is started\n", info.name);
//#endif
//            rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
//            robotStarted = 1;
//            rt_mutex_release(&mutex_robotStarted);
//            MessageToMon msg;
//            set_msgToMon_header(&msg, (char*)HEADER_STM_ACK);
//            write_in_queue(&q_messageToMon, msg);
//        } else {
//            MessageToMon msg;
//            set_msgToMon_header(&msg, (char*)HEADER_STM_NO_ACK);
//            write_in_queue(&q_messageToMon, msg);
//        }
//    }
//}
//
//void Tasks::f_move(void *arg) {
//    /* INIT */
//    RT_TASK_INFO info;
//    rt_task_inquire(NULL, &info);
//    printf("Init %s\n", info.name);
//    rt_sem_p(&sem_barrier, TM_INFINITE);
//
//    /* PERIODIC START */
//#ifdef _WITH_PERIODIC_TRACE_
//    printf("%s: start period\n", info.name);
//#endif
//    rt_task_set_periodic(NULL, TM_NOW, 100000000);
//    while (1) {
//#ifdef _WITH_PERIODIC_TRACE_
//        printf("%s: Wait period \n", info.name);
//#endif
//        rt_task_wait_period(NULL);
//#ifdef _WITH_PERIODIC_TRACE_
//        printf("%s: Periodic activation\n", info.name);
//        printf("%s: move equals %c\n", info.name, robotMove);
//#endif
//        rt_mutex_acquire(&mutex_robotStarted, TM_INFINITE);
//        if (robotStarted) {
//            rt_mutex_acquire(&mutex_move, TM_INFINITE);
//            send_command_to_robot(robotMove);
//            rt_mutex_release(&mutex_move);
//#ifdef _WITH_TRACE_
//            printf("%s: the movement %c was sent\n", info.name, robotMove);
//#endif            
//        }
//        rt_mutex_release(&mutex_robotStarted);
//    }
//}
//
//void write_in_queue(RT_QUEUE *queue, MessageToMon msg) {
//    void *buff;
//    buff = rt_queue_alloc(&q_messageToMon, sizeof (MessageToMon));
//    memcpy(buff, &msg, sizeof (MessageToMon));
//    rt_queue_send(&q_messageToMon, buff, sizeof (MessageToMon), Q_NORMAL);
//}

#endif //__WITH_PTHREAD__