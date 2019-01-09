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

#include "img.h"

bool Arena::IsEmpty() {
    if ((this->arena.height == 0) || (this->arena.width == 0)) return true;
    else return false;
}

Img::Img(ImageMat imgMatrice) {
    this->img = imgMatrice.clone();
}

string Img::ToString() {
    return "Image size: " + to_string(this->img.cols) + "x" + to_string(this->img.rows) + " (dim=" + to_string(this->img.dims) + ")";
}

Img* Img::Copy() {
    return new Img(this->img);
}

float Img::CalculAngle(Position robot) {
    float a = robot.direction.x - robot.center.x;
    float b = robot.direction.y - robot.center.y;
    float angle = atan2(b, a);
    return angle * 180.f / M_PI;
}

float Img::CalculAngle2(cv::Point2f pt1, cv::Point2f pt2) {
    float a = pt1.x - pt2.x;
    float b = pt1.y - pt2.y;
    float angle = atan2(b, a);
    return angle * 180.f / M_PI;
}

#ifdef __WITH_ARUCO__ 

cv::Point2f Img::FindArucoCenter(std::vector<cv::Point2f> aruco) {
    return ((aruco[0] + aruco[2]) / 2);
}

cv::Point2f Img::FindArucoDirection(std::vector<cv::Point2f> aruco) {
    return ((aruco[0] + aruco[1]) / 2);
}

std::list<Position> Img::SearchAruco(Arena arena) {
    ImageMat imgTraitment;
    std::list<Position> positionList;
    cv::Point2f areneCoor;
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f> > corners;
    
    if (arena.IsEmpty())
        imgTraitment = this->img.clone();
    else {
        imgTraitment = CropArena(arena);
        areneCoor.x = arena.arena.x;
        areneCoor.y = arena.arena.y;
    }
    
    cv::aruco::detectMarkers(imgTraitment, dictionary, corners, ids);
    if (ids.size() > 0) {
        for (int i = 0; i < ids.size(); i++) {
            Position newPos;
            newPos.center = FindArucoCenter(corners[i]);
            newPos.direction = FindArucoDirection(corners[i]);
            newPos.robotId = ids[i];
            if (!arena.IsEmpty()) {
                newPos.direction += areneCoor;
                newPos.center += areneCoor;
            }
            newPos.angle = CalculAngle2(newPos.center, newPos.direction);
            positionList.push_back(newPos);
        }
    }
    return positionList;
}
#endif // __WITH_ARUCO__

float Img::EuclideanDistance(cv::Point2f p, cv::Point2f q) {
    cv::Point diff = p - q;
    return cv::sqrt(diff.x * diff.x + diff.y * diff.y);
}

Jpg Img::ToJpg() {
    Jpg imgJpg;
    cv::imencode(".jpg", this->img, imgJpg);
    return imgJpg;
}

std::list<Position> Img::SearchRobot(Arena arena) {

    std::list<Position> robotsFind;
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Point> approx;
    std::vector<cv::Vec4i> hierarchy;

    ImageMat imgTraitment;

    if (arena.IsEmpty()) {
        imgTraitment = this->img.clone();
    } else {
        imgTraitment = this->img(arena.arena);
    }

    cvtColor(imgTraitment, imgTraitment, CV_RGB2GRAY);
    threshold(imgTraitment, imgTraitment, 128, 255, CV_THRESH_BINARY);
    findContours(imgTraitment, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    for (unsigned int i = 0; i < contours.size(); i++) {
        ImageMat m(contours[i]);
        cv::approxPolyDP(m, approx, cv::arcLength(ImageMat(contours[i]), true)*0.17, true);

        if (approx.size() == 3 && fabs(cv::contourArea(contours[i])) > 200 && fabs(cv::contourArea(contours[i])) < 700) {
            cv::Point a, b, c;
            cv::Point center;

            a = approx[0];
            b = approx[1];
            c = approx[2];

            if (!arena.IsEmpty()) { // ajout de l'offset de l'arène
                a.x += arena.arena.x;
                a.y += arena.arena.y;
                b.x += arena.arena.x;
                b.y += arena.arena.y;
                c.x += arena.arena.x;
                c.y += arena.arena.y;
            }

            center.x = (a.x + b.x + c.x) / 3;
            center.y = (a.y + b.y + c.y) / 3;
            Position newPos;
            newPos.center = center;

            if (EuclideanDistance(center, b) > EuclideanDistance(center, a) && EuclideanDistance(center, b) > EuclideanDistance(center, c)) {
                newPos.direction = b;
                //line(img,center,b,Scalar(0,125,0),2,8,0);
            } else if (EuclideanDistance(center, a) > EuclideanDistance(center, c)) {

                newPos.direction = a;
                //line(img,center,a,Scalar(0,125,0),2,8,0);
            } else {
                newPos.direction = c;
                //line(img,center,c,Scalar(0,125,0),2,8,0);
            }

            newPos.angle = CalculAngle(newPos);
            newPos.robotId = -1; // dumb identifier

            robotsFind.push_back(newPos);
        }
    }

    return robotsFind;
}

Arena Img::SearchArena() {
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Point> approx;
    std::vector<cv::Vec4i> hierarchy;

    ImageMat imageTrt;
    cv::cvtColor(this->img, imageTrt, CV_RGB2GRAY); // conversion en niveau de gris
    cv::threshold(imageTrt, imageTrt, 128, 255, CV_THRESH_BINARY); // Threshold les éléments les plus clair
    cv::Canny(imageTrt, imageTrt, 100, 200, 3); // detection d'angle

    findContours(imageTrt, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
    for (unsigned int i = 0; i < contours.size(); i++) {
        approxPolyDP(ImageMat(contours[i]), approx, cv::arcLength(ImageMat(contours[i]), true)*0.1, true);
        if (approx.size() == 4 && fabs(cv::contourArea(contours[i])) > 100000) {
            Arena rectangle;
            rectangle.arena = cv::boundingRect(ImageMat(contours[i]));
            return rectangle;
        }
    }
    return Arena();
}

int Img::DrawRobot(Position robot) {
    cv::arrowedLine(this->img, (cv::Point2f)robot.center, (cv::Point2f)robot.direction, cv::Scalar(0, 0, 255), 3, 8, 0);
    return 0;
}

int Img::DrawAllRobots(std::list<Position> robots) {
    for (Position robot : robots) {
        cv::arrowedLine(this->img, (cv::Point2f)robot.center, (cv::Point2f)robot.direction, cv::Scalar(0, 0, 255), 3, 8, 0);
    }
    return robots.size();
}

int Img::DrawArena(Arena arenaToDraw) {
    cv::rectangle(this->img, arenaToDraw.arena.tl(), arenaToDraw.arena.br(), cv::Scalar(0, 0, 125), 2, 8, 0);
    return 0;
}

ImageMat Img::CropArena(Arena arena) {
    return this->img(arena.arena);
}
