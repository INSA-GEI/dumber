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

void Camera::setSize(int size) {
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


int Camera::open_camera() {
    this->cap.open(0);
}

Camera::Camera(int size) {
    this->setSize(size);
    this->cap.set(CV_CAP_PROP_FORMAT, CV_8UC3);
    this->cap.set(CV_CAP_PROP_FRAME_WIDTH,width);
    this->cap.set(CV_CAP_PROP_FRAME_HEIGHT,height);
}

int Camera::close_camera() {
    cap.release();
    return 0;
}

Img Camera::grab_image() {
    ImageMat frame;
    cap >> frame;
    Img capture = Img(frame);
    return capture;
}


bool Camera::isOpen() {
    return cap.isOpened();
}

int Camera::getWidth() const {
    return width;
}

int Camera::getHeight() const {
    return height;
}


