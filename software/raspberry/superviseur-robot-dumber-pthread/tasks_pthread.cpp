/*
 * Copyright (C) 2024 dimercur
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

ComMonitor Tasks::monitor;
ComRobot Tasks::robot;

bool Tasks::sendImage;
bool Tasks::sendPosition;

int Tasks::counter;
bool Tasks::flag;

bool Tasks::showArena;

pthread_t Tasks::threadServer;
pthread_t Tasks::threadSendToMon;
pthread_t Tasks::threadTimer;

void Tasks::Init() {
    int status;

    Tasks::showArena =false;
    Tasks::sendImage =false;
    Tasks::sendPosition =false;

    Tasks::counter=0;
    Tasks::flag =false;


    /* Open com port with STM32 */
    cout << "Open serial com (";
    status = Tasks::robot.Open();
    cout << status;
    cout << ")" << endl;

    if (status >= 0) {
        // Open server

        status = Tasks::monitor.Open(SERVER_PORT);
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
    if (pthread_create(&threadTimer, NULL, (void *(*)(void*)) &Tasks::TimerTask, NULL) != 0) {
    perror("pthread_create() error");
    exit(1);
    }

    if (pthread_create(&threadServer, NULL, (void *(*)(void*)) &Tasks::ServerTask, NULL) != 0) {
    perror("pthread_create() error");
    exit(1);
    }
#pragma GCC diagnostic pop

    cout << "Tasks launched" << endl << flush;
}

void Tasks::Stop() {
    Tasks::monitor.Close();
    Tasks::robot.Close();
}

void *Tasks::ServerTask(void *arg) {
    Message *msgRcv;
    Message *msgSend;
    bool isActive = true;
    void *ret=NULL;

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    while (isActive) {
        msgRcv = NULL;
        msgSend = NULL;

        msgRcv = Tasks::monitor.Read();
        cout << "Rcv <= " << msgRcv->ToString() << endl << flush;

        if (msgRcv->CompareID(MESSAGE_ROBOT_COM_OPEN)) {
            msgSend = new Message(MESSAGE_ANSWER_ACK);

            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_ROBOT_COM_CLOSE)) {
            msgSend= new Message(MESSAGE_ROBOT_RESET);
            msgSend = Tasks::robot.Write(msgSend);
            delete (msgSend); // must be deleted manually, no consumer

            msgSend = new Message(MESSAGE_ANSWER_ACK);
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITH_WD)) {
            msgSend = Tasks::robot.Write(msgRcv);
            cout << "Start with wd answer: " << msgSend->ToString() << endl << flush;
        }
        else if (msgRcv->CompareID(MESSAGE_ROBOT_START_WITHOUT_WD)) {
            msgSend = Tasks::robot.Write(msgRcv);
            cout << "Start without wd answer: " << msgSend->ToString() << endl << flush;
        }
        else if (msgRcv->CompareID(MESSAGE_ROBOT_RESET)) {
            msgSend = Tasks::robot.Write(msgRcv);
            cout << "Reset answer: " << msgSend->ToString() << endl << flush;
        }
        else if (msgRcv->CompareID(MESSAGE_ROBOT_GO_FORWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_BACKWARD) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_LEFT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_GO_RIGHT) ||
                msgRcv->CompareID(MESSAGE_ROBOT_STOP)) {
            msgSend = Tasks::robot.Write(msgRcv);

            cout << "Movement answer: " << msgSend->ToString() << endl << flush;
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_OPEN)) {
            Tasks::sendImage = true;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            cout << "Start sending images" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_CLOSE)) {
            Tasks::sendImage = false;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            cout << "Stop sending images" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_START)) {
            Tasks::sendPosition = true;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            cout << "Start computing positions based on AruCo" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_POSITION_COMPUTE_STOP)) {
            Tasks::sendPosition = false;
            msgSend = new Message(MESSAGE_ANSWER_ACK);
            cout << "Stop computing positions based on AruCo" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_ROBOT_BATTERY_GET)) {
            msgSend = new MessageBattery(MESSAGE_ROBOT_BATTERY_LEVEL, BATTERY_FULL);

            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_ASK_ARENA)) {
            Tasks::showArena = true;
            cout << "Ask for arena boundaries" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_ARENA_CONFIRM)) {
            Tasks::showArena = false;
            cout << "Arena boundaries OK" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        else if (msgRcv->CompareID(MESSAGE_CAM_ARENA_INFIRM)) {
            Tasks::showArena = false;
            cout << "Arena boundaries rejected" << endl << flush;
            
            delete(msgRcv); // must be deleted manually, no consumer
        }
        
        if (msgSend != NULL) Tasks::monitor.Write(msgSend);
    }

    return ret;
}

void *Tasks::TimerTask(void* arg) {
    struct timespec tim, tim2;
    Message *msgSend;
    int counter;
    int cntFrame = 0;
    Position pos;
    Arena arena;
    int counter_img;
    void *ret=NULL;

    tim.tv_sec = 0;
    tim.tv_nsec = 50000000; // 50ms (20fps)

    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    Camera camera = Camera(sm, 15);
    cout << "Try opening camera" << endl << flush;
    if (camera.Open()) {
        cout << "Camera opened successfully" << endl ;
        cout << "Image size: " << camera.GetWidth() << "x"<< camera.GetHeight() << " @ " << camera.GetFPS() <<endl<<flush;
    } else {
        cout << "Failed to open camera" << endl << flush;

        exit(0);
    }

    pos.angle = 0.0;
    pos.robotId = -1;
    pos.center = cv::Point2f(0, 0);
    pos.direction = cv::Point2f(0, 0);

    counter_img = 0;

    while (1) {
        Img image = camera.Grab(); // 15fps
        
        image.dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(3));

        std::list<Position> poses = image.SearchRobot(arena);
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
            pos.center = cv::Point2f(0, 0);
            pos.direction = cv::Point2f(0, 0);
        }
        
        if (Tasks::sendPosition == true) {
            counter++;

            if (counter >= 15) { // div =15/15 => 1 update per second
                counter = 0;
                
                //if (!arena.IsEmpty()) {
                
                cout << "Nbr of pos detected: " << to_string(poses.size()) << endl << flush;

                MessagePosition *msgp = new MessagePosition(MESSAGE_CAM_POSITION, pos);
                Tasks::monitor.Write(msgp);
                //cout << "Position sent" << endl << flush;
            }
        }

        if (Tasks::sendImage == true) {
            counter_img++;

            if (counter_img >= 1) { // Div = 15/1 => 15 images per sec
                counter_img = 0;

                if (Tasks::showArena) {
                    arena = image.SearchArena();

                    if (!arena.IsEmpty()) image.DrawArena(arena);
                    else cout << "Arena not found" << endl << flush;
                }

                if (Tasks::sendPosition == true) {
                    image.DrawRobot(pos);
                }

                if (!arena.IsEmpty()) image.DrawArena(arena);
                
                Img resizedImage = image.Resize();
                MessageImg *msg = new MessageImg(MESSAGE_CAM_IMAGE, &resizedImage);

                Tasks::monitor.Write(msg);
                //cout << "Image sent" <<  endl << flush;
            }
        }
    }

    return ret;
}

void Tasks::SendToMonTask(void* arg) {
    cout << "Start " << __PRETTY_FUNCTION__ << endl << flush;

    while (1) {

    }
}

#endif //__WITH_PTHREAD__