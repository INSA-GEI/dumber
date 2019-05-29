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

#ifndef __FOR_PC__
#include "raspicam/raspicam_cv.h"
#endif /* __FOR_PC__ */
#include "img.h"

/**
 * Enumerate for picture size
 */
enum captureSize {xs, sm, md, lg};

/**
 * Class for camera (image grab)
 * 
 * @brief Class for camera (image grab)
 * How to grab an image and send it to the monitor:
 *  1. Grab an image, for example:
 *          Img * img = new Img(cam->Grab());
 *          Img * img = cam->Grab().Copy();
 *  2. Instanciate the message to send the image:
 *          MessageImg *msgImg = new MessageImg(MESSAGE_CAM_IMAGE, img);
 */
class Camera {
public:
    Camera();
    
    /**
     * Create an object for accessing camera
     * @param size Size of picture to grab (@see captureSize)
     * @param fps speed of sampling
     */
    Camera(int size, int fps);

    /**
     * Open camera
     * @return True if camera is open, false otherwise
     */
    bool Open();
    
    /**
     * Close and release camera
     */
    void Close();
    
    /**
     * Get width of sampled image
     * @return Width of sampled picture
     */
    int GetWidth() const;
    
    /**
     * Get height of sampled image
     * @return height of sampled picture
     */
    int GetHeight() const;

    /**
     * Get opening status for camera
     * @return true if camera is open, false otherwise
     */
    bool IsOpen();
    
    /**
     * Define size for sampled picture
     * @param size Size of picture (@see captureSize)
     */
    void SetSize(int size);

    /**
     * Grab next image from camera
     * @return Image taken from camera
     */
    Img Grab();
    
private:
#ifdef __FOR_PC__
    /**
     * Camera descriptor
     */
    cv::VideoCapture cap;
#else
    raspicam::RaspiCam_Cv cap;
#endif /*  __FOR_PC__ */   
   
    /**
     * Size for image (default= small)
     */
    int size = sm;
    
    /**
     * Width of image
     */
    int width;
    
    /**
     * Height of image
     */
    int height;
};

#endif //__CAMERA_H__
