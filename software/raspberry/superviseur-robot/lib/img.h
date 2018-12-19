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

class Arene {
public:
    Arene();
    
    cv::Rect arene;
    bool empty();
};

class Img {
public:
    Img(ImageMat imgMatrice);
    
    string ToString();
    Img* Copy();
    
    Jpg toJpg();
    string ToBase64();
    Arene search_arena();

    int draw_robot(Position robot);
    int draw_all_robots(std::list<Position> robots);
    int draw_arena(Arene areneToDraw);
    std::list<Position> search_robot(Arene monArene);
    
    
#ifdef __WITH_ARUCO__    
    list<Position> search_aruco(Arene monArene = NULL);
#endif // __WITH_ARUCO__
private:
    ImageMat img;
    
#ifdef __WITH_ARUCO__
    Ptr<std::Dictionary> dictionary;
    cv::Point2f find_aruco_center(std::vector<cv::Point2f> aruco);
    cv::Point2f find_aruco_direction(std::vector<cv::Point2f> aruco);
#endif // __WITH_ARUCO__
    
    float calculAngle(Position robots);
    float calculAngle2(cv::Point2f pt1, cv::Point2f pt2);
    float euclideanDist(cv::Point2f p, cv::Point2f q);
    ImageMat cropArena(Arene arene);
};

#endif //__IMG_H__
