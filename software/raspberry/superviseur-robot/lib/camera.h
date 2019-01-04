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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "img.h"

enum captureSize {xs, sm, md, lg};

class Camera {
public:
    Camera(int size);

    bool Open();
    void Close();
    
    int GetWidth() const;
    int GetHeight() const;

    bool IsOpen();
    void SetSize(int size);

    Img Grab();
    
private:
    cv::VideoCapture cap;
    int size = sm;
    int width;
    int height;
};

#endif //__CAMERA_H__
