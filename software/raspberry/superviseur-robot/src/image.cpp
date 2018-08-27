/**
 * \file      imagerie.cpp
 * \author    L.Senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Fonctions de traitement d'image utilisable pour la détection du robot.
 *
 * \details   Ce fichier utilise la libraire openCV2 pour faciliter le traitement d'image dans le projet Destijl.
 *            Il permet de faciliter la détection de l'arène et la détection du robot.
 *			  /!\ Attention Bien que celui-ci soit un .cpp la structure du code n'est pas sous forme d'objet.
 */

#include "image.h"
#include <iostream>

using namespace cv;
#ifndef __STUB__
using namespace raspicam;
#else
Image stubImg;
#endif
using namespace std;

float calculAngle(Position * positionRobot);
int cropArena(Image *imgInput, Image *imgOutput, Arene *AreneInput);
float euclideanDist(Point& p, Point& q);

void draw_arena(Image *imgInput, Image *imgOutput, Arene *monArene)
{
    if(imgInput!=imgOutput)
        *imgOutput=imgInput->clone();
    rectangle(*imgOutput,monArene->tl(),monArene->br(),Scalar(0,0,125),2,8,0);
}

int open_camera(Camera  *camera)
{
#ifndef __STUB__
    camera->set(CV_CAP_PROP_FORMAT, CV_8UC3);
    camera->set(CV_CAP_PROP_FRAME_WIDTH,WIDTH);
    camera->set(CV_CAP_PROP_FRAME_HEIGHT,HEIGHT);

    printf("Opening Camera...\n");
    if (!(camera->open())) {
        perror("Can't open Camera\n") ;
        return -1;
    }
    else
    {
        printf("Camera warmup 2sec\n");
        sleep(2);
        printf("Start capture\n");
        return 0;
    }   
#endif
}

void get_image(Camera *camera, Image * monImage, const char  * fichier) // getImg(Camera, Image img);
{
#ifndef __STUB__
    camera->grab();
    camera->retrieve(*monImage);
    cvtColor(*monImage,*monImage,CV_BGR2RGB);
#else
    stubImg = imread(fichier, CV_LOAD_IMAGE_COLOR);
    stubImg.copyTo(*monImage);
#endif
}

void close_camera(Camera *camera) // closeCam(Camera) : camera Entrer
{
#ifndef __STUB__
    camera->release();
#endif
}


int detect_arena(Image *monImage, Arene *rectangle) // Image en entrée // rectangle en sortie
{
    vector<vector<Point> > contours;
    vector<Point> approx;
    vector<Vec4i> hierarchy;

    Image imageTrt;
    cvtColor(*monImage,imageTrt,CV_RGB2GRAY); // conversion en niveau de gris
    threshold(imageTrt,imageTrt,128,255,CV_THRESH_BINARY); // Threshold les éléments les plus clair
    Canny(imageTrt, imageTrt, 100,200,3); // detection d'angle

    findContours(imageTrt, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    for(unsigned int i = 0; i < contours.size();i++)
	{
        approxPolyDP(Image(contours[i]), approx, arcLength(Image(contours[i]), true)*0.1, true);
        if(approx.size()==4 && fabs(cv::contourArea(contours[i])) > 100000)
        {
            *rectangle = boundingRect(Image(contours[i]));
            return 0;
        }
    }
    return -1;
}

int cropArena(Image *imgInput, Image *imgOutput, Arene *areneInput) // image // rectangle // image2
{
    Image img;
    img=imgInput->clone();

    *imgOutput = img(*areneInput);
    return 0;
}

float euclideanDist(Point& p, Point& q) {
    Point diff = p - q;
    return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

void compress_image(Image *imgInput, Jpg *imageCompress) // image entrée // imageEncodé en sortie
{
    imencode(".jpg",*imgInput,*imageCompress);
}


int detect_position(Image *imgInput, Position *posTriangle, Arene * monArene) // entree : image  / sortie tab pos
{
    vector<vector<Point> > contours;
    vector<Point> approx;
    vector<Vec4i> hierarchy;

    Image imgTraitment;

    if(monArene==NULL)
        imgTraitment=imgInput->clone();
    else
        cropArena(imgInput,&imgTraitment, monArene);

    cvtColor(imgTraitment,imgTraitment,CV_RGB2GRAY);
    threshold(imgTraitment,imgTraitment,128,255,CV_THRESH_BINARY);
    findContours(imgTraitment, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    int nbrTriangle = 0;
    for(unsigned int i = 0;i < contours.size();i++)
	{
        approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.17, true);
        if(approx.size() == 3 && fabs(contourArea(contours[i])) > 200 && fabs(contourArea(contours[i])) < 700)
        {

            Point a,b,c;
            Point center;


            a = approx[0];
            b = approx[1];
            c = approx[2];


            if(monArene !=NULL) // ajout de l'offset de l'arène
            {
               a.x += monArene->x;
               a.y += monArene->y;
               b.x += monArene->x;
               b.y += monArene->y;
               c.x += monArene->x;
               c.y += monArene->y;		    
            }
		
            center.x = (a.x + b.x + c.x)/3;
            center.y = (a.y + b.y + c.y)/3;

            posTriangle[nbrTriangle].center=center;

            if(euclideanDist(center,b) > euclideanDist(center,a) && euclideanDist(center,b) > euclideanDist(center,c) )
            {

                posTriangle[nbrTriangle].direction=b;
                //line(img,center,b,Scalar(0,125,0),2,8,0);
            }
            else if(euclideanDist(center,a) > euclideanDist(center,c))
            {
                posTriangle[nbrTriangle].direction=a;
                //line(img,center,a,Scalar(0,125,0),2,8,0);

            }
            else
            {
                posTriangle[nbrTriangle].direction=c;
                //line(img,center,c,Scalar(0,125,0),2,8,0);
            }
            posTriangle[nbrTriangle].angle=calculAngle(&posTriangle[nbrTriangle]);

            nbrTriangle++;

        }
    }
    return nbrTriangle;
}

void draw_position(Image *imgInput, Image *imgOutput, Position *positionRobot) // img E/S   pos : E
{
    if(imgInput!=imgOutput)
    {
        *imgOutput=imgInput->clone();
    }
    line(*imgOutput,positionRobot->center,positionRobot->direction,Scalar(0,125,0),2,8,0);
}

float calculAngle(Position * positionRobot) // position en entree
{
    float a = positionRobot->direction.x - positionRobot->center.x;
    float b = positionRobot->direction.y - positionRobot->center.y ;
    float angle = atan2(b,a);
    return angle * 180.f/M_PI;

}
