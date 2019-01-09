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

#ifndef __IMG_H__
#define __IMG_H__

#include <iostream>
#include <list>
#include <string>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>

#ifdef __WITH_ARUCO__
#include <opencv2/aruco/dictionary.hpp>
#include <opencv2/aruco/charuco.hpp>
//#include <opencv2/aruco.hpp>
#include <opencv2/core/mat.hpp>

#endif // __WITH_ARUCO__

#define ARENA_NOT_DETECTED -1

using namespace std;

typedef cv::Mat ImageMat;

typedef vector<unsigned char> Jpg;

typedef struct {
    cv::Point2f center;
    cv::Point2f direction;
    float angle;
    int robotId;
} Position;

class Arena {
public:
    Arena() {}
    
    cv::Rect arena;
    bool IsEmpty();
};

class Img {
public:
    ImageMat img;
    
    Img(ImageMat imgMatrice);
    
    string ToString();
    Img* Copy();
    
    Jpg ToJpg();
    Arena SearchArena();

    int DrawRobot(Position robot);
    int DrawAllRobots(std::list<Position> robots);
    int DrawArena(Arena arenaToDraw);
    std::list<Position> SearchRobot(Arena arena);
        
#ifdef __WITH_ARUCO__    
    list<Position> SearchAruco(Arena arena);
    cv::Ptr<cv::aruco::Dictionary> dictionary;
#endif // __WITH_ARUCO__
private:
#ifdef __WITH_ARUCO__
    cv::Point2f FindArucoCenter(std::vector<cv::Point2f> aruco);
    cv::Point2f FindArucoDirection(std::vector<cv::Point2f> aruco);
#endif // __WITH_ARUCO__
    
    float CalculAngle(Position robots);
    float CalculAngle2(cv::Point2f pt1, cv::Point2f pt2);
    float EuclideanDistance(cv::Point2f p, cv::Point2f q);
    ImageMat CropArena(Arena arena);
};

#endif //__IMG_H__
