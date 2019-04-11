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
#include <unistd.h>

using namespace cv;

/**
 * Create an object for accessing camera with default values (size = sm and 
 * fps = 10)
 */
Camera::Camera():Camera(sm, 10){
}

/**
 * Create an object for accessing camera
 * @param size Size of picture to grab (@see captureSize)
 * @param fps speed of sampling
 */
Camera::Camera(int size, int fps) {
    this->SetSize(size);
#ifndef __FOR_PC__
    this->cap.set(CV_CAP_PROP_FORMAT, CV_8UC3);
    this->cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
    this->cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
    this->cap.set(CV_CAP_PROP_FPS, fps);
#endif /* __FOR_PC__ */
}

/**
 * Open camera
 * @return True if camera is open, false otherwise
 */
bool Camera::Open() {
    bool status = false;

#ifdef __FOR_PC__
    if (this->cap.open(0)) {
        //this->cap.set(CV_CAP_PROP_FORMAT, CV_8UC3);
        this->cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
        this->cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);

        status = true;
    }
#else
    if (this->cap.open()) {
        cout << "Camera warmup 2sec" << endl << flush;
        sleep(2);
        cout << "Start capture" << endl << flush;

        status = true;
    }
#endif /* __FOR_PC__ */

    return status;
}

/**
 * Close and release camera
 */
void Camera::Close() {
    this->cap.release();
}

/**
 * Define size for sampled picture
 * @param size Size of picture (@see captureSize)
 */
void Camera::SetSize(int size) {
    this->size = size;

    switch (size) {
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

/**
 * Grab next image from camera
 * @return Image taken from camera
 */
Img Camera::Grab() {
    ImageMat frame;

#ifdef __FOR_PC__
    cap >> frame;
    Img capture = Img(frame);
#else
    cap.grab();
    cap.retrieve(frame);

#ifdef __INVERSE_COLOR__
    cvtColor(frame, frame, CV_BGR2RGB);
#endif // __INVERSE_COLOR__

    Img capture = Img(frame);
#endif /* __FOR_PC__ */

    return capture;
}

/**
 * Get opening status for camera
 * @return true if camera is open, false otherwise
 */
bool Camera::IsOpen() {
    return cap.isOpened();
}

/**
 * Get width of sampled image
 * @return Width of sampled picture
 */
int Camera::GetWidth() const {
    return width;
}

/**
 * Get height of sampled image
 * @return height of sampled picture
 */
int Camera::GetHeight() const {
    return height;
}
