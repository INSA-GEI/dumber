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

/**
 * Redefinition of cv::Mat type
 */
typedef cv::Mat ImageMat;

/**
 * Declaration of Jpg type
 */
typedef vector<unsigned char> Jpg;

/**
 * Class position, used for store robot coordinates
 * 
 * @brief Class position, used for store robot coordinates
 */
class Position {
public:
    int robotId;
    float angle;
    cv::Point2f center; 
    cv::Point2f direction;
    
    /**
     * Constructor of Position object
     */
    Position() {
        robotId = -1;
        angle = 0.0;
        center=cv::Point2f(0.0,0.0);
        direction=cv::Point2f(0.0,0.0);
    }
    
    /**
     * Build a string representation of the object
     * @return String containing object information
     */
    string ToString() {
        return "Id=" + to_string(this->robotId) + ", Angle=" + to_string(this->angle) + ", center=(" + to_string(this->center.x) + ";" + to_string(this->center.y) + ")";
    }
};

/**
 * Class arena, used for holding outline of arena on image and cropping image to only usefull area
 * 
 * @brief Class arena, used for holding outline of arena on image and cropping image to only usefull area
 */
class Arena {
public:
    /**
     * Coordinate of arena, empty if no arena found
     */
    cv::Rect arena;
    
    /**
     * Constructor of Arena object
     */
    Arena() {}
    
    /**
     * Tell if arena is empty (not found) or not
     * @return true if no arena found, false otherwise
     */
    bool IsEmpty();
    
    /**
     * Build a string representation of the object
     * @return String containing object information
     */
    string ToString() {
        if (IsEmpty()) return "Arena empty";
        else
        return "Arena: (x;y)=(" + to_string(this->arena.x) + ";" + to_string(this->arena.x) + " (w;h)=(" + to_string(this->arena.width) + ";" + to_string(this->arena.height) + ")";
    }
};

/**
 * Class for image storage and treatment
 * 
 * @brief Class for image storage and treatment
 */
class Img {
public:
    /**
     * Image data
     */
    ImageMat img;
    
    /**
     * Create new Img object based on image data
     * 
     * @param imgMatrice Image data to be stored (raw data)
     */
    Img(ImageMat imgMatrice);
    
    /**
     * Convert object to a string representation
     * 
     * @return String containing information on contained image (size and number of channel) 
     */
    string ToString();
    
    /**
     * Create a copy of current object
     * 
     * @return New Img object, copy of current 
     */
    Img* Copy();
    
    /**
     * Compress current image to JPEG
     * @return Image compressed as JPEG
     */
    Jpg ToJpg();
    
    /**
     * Search arena outline in current image
     * @return Arena object with coordinate of outline, empty if no arena found
     */
    Arena SearchArena();
    
    /**
     * Draw an oriented arrow at robot position
     * @param robot Position of robot
     */
    void DrawRobot(Position robot);
    
    /**
     * Draw an oriented arrow for each position provided
     * @param robots List of robot positions
     * @return Number of position drawn
     */
    int DrawAllRobots(std::list<Position> robots);
    
    /**
     * Draw arena outline
     * @param arenaToDraw Arena position
     */
    void DrawArena(Arena arenaToDraw);
    
    /**
     * Search available robots in an image
     * @param arena Arena position for cropping image
     * @return list of position, empty if no robot found
     */
    std::list<Position> SearchRobot(Arena arena);
       
    Img Resize();
#ifdef __WITH_ARUCO__    
    /**
     * Dictionary to be used for aruco recognition
     */
    cv::Ptr<cv::aruco::Dictionary> dictionary;
#endif // __WITH_ARUCO__
private:
#ifdef __WITH_ARUCO__
    /**
     * Find center point of given aruco
     * @param aruco Aruco coordinates
     * @return Center point coordinate
     */
    cv::Point2f FindArucoCenter(std::vector<cv::Point2f> aruco);
    
    /**
     * Find direction of given aruco
     * @param aruco Aruco coordinates
     * @return Orientation of aruco
     */
    cv::Point2f FindArucoDirection(std::vector<cv::Point2f> aruco);
#endif // __WITH_ARUCO__
    
    /**
     * Function for computing angle 
     * @param robots Position of robot
     * @return Angle
     */
    float CalculAngle(Position robots);
    
    /**
     * Function for computing angle 
     * @param pt1 ???
     * @param pt2 ???
     * @return Angle
     */
    float CalculAngle2(cv::Point2f pt1, cv::Point2f pt2);
    
    /**
     * Used for computing distance
     * @param p ???
     * @param q ???
     * @return Distance
     */
    float EuclideanDistance(cv::Point2f p, cv::Point2f q);
    
    /**
     * Crop image around detected arena
     * @param arena Coordinate of arena
     * @return Reduced image, focused on arena
     */
    ImageMat CropArena(Arena arena);
};

#endif //__IMG_H__
