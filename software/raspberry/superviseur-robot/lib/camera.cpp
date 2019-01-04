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

#include "camera.h"
#include "img.h"

using namespace cv;

Camera::Camera(int size) {
    this->SetSize(size);
    this->cap.set(CV_CAP_PROP_FORMAT, CV_8UC3);
    this->cap.set(CV_CAP_PROP_FRAME_WIDTH,width);
    this->cap.set(CV_CAP_PROP_FRAME_HEIGHT,height);
}

bool Camera::Open() {
    this->cap.open(0);
}

void Camera::Close() {
    this->cap.release();
}

void Camera::SetSize(int size) {
    this->size = size;
    
    switch (size){
        case xs:
            this->width = 480;
            this->height = 360;
            break;
        case sm:
            this->width = 640;
            this->height = 480;
            break;
        case md:
            this->width = 1024;
            this->height = 768;
            break;
        case lg:
            this->width = 1280;
            this->height = 960;
            break;
        default:
            this->width = 480;
            this->height = 360;

    }
}

Img Camera::Grab() {
    ImageMat frame;
    
    cap >> frame;
    Img capture = Img(frame);
    
    return capture;
}

bool Camera::IsOpen() {
    return cap.isOpened();
}

int Camera::GetWidth() const {
    return width;
}

int Camera::GetHeight() const {
    return height;
}


