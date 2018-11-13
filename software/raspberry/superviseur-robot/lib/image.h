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

/**
 * \file      image.h
 * \author    L.Senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Functions for image treatment.
 *
 * \details   This file use openCV2 library for picture processing. This allow to detect arena and robot.
 */

#ifndef _IMAGE_H_
#define _IMAGE_H_

#ifndef __STUB__
#ifndef __FOR_PC__
#include <raspicam/raspicam_cv.h>
#else
#include <opencv2/highgui/highgui.hpp>
#endif /* __FOR_PC__ */
#else
#include <opencv2/highgui/highgui.hpp>
#endif
#include "opencv2/imgproc/imgproc.hpp"
#include <unistd.h>
#include <math.h>

#define WIDTH 480 //1280 1024 640 480
#define HEIGHT 360 // 960 768 480 360

using namespace std;
using namespace cv;
#ifndef __STUB__
#ifndef __FOR_PC__
using namespace raspicam;
#endif /* __FOR_PC__ */
#endif

typedef Mat Image;
#ifndef __STUB__
#ifndef __FOR_PC__
typedef RaspiCam_Cv Camera;
#else
typedef int Camera;
#endif /* __FOR_PC__ */
#else
typedef int Camera;
#endif

typedef Rect Arene;
typedef vector<unsigned char> Jpg;

struct Position {
    Point center;
    Point direction;
    float angle;
};

/**
 * \brief       Ouvre une camera.
 * \details     Met à jour le descripteur de fichier passé en paramètre pour correspondre à la camera ouverte
 *             
 * \param       camera  Pointeur d'un file descriptor d'une camera ouverte
 * \return      Retourne 0 si la camera a été ouverte correctement et -1 si une erreur survient.
 */
int open_camera(Camera *camera);

/**
 * \brief       Ferme la camera passé en paramètre
 *             
 * \param       camera  Pointeur sur la camera à fermer
 */
void close_camera(Camera *camera);

/**
 * \brief       Capture une image avec la camera passée en entrée. En cas de test sans camera, la fonction charge une image 
 * \details     La camera doit préalablement être ouverte via \a openCamera(...)
 *             
 * \param       camera      Pointeur sur la camera passée en entrée.
 * \param       monImage    Pointeur sur une image capturée. 
 * \param       fichier     Chemin du fichier d'image
 * \return      Retourne -1 si une erreur survient.
 */
void get_image(Camera *camera, Image * monImage, const char *fichier = NULL);

/**
 * \brief       Détecte une arène dans une image fournis en paramètre.
 *             
 * \param       monImage    Pointeur sur l'image d'entrée
 * \param       rectangle   Pointeur sur les coordonnées du rectangles trouvé. 
 * \return      Retourne -1 si aucune arène n'est détectée. Sinon retourne 0
 */
int detect_arena(Image *monImage, Arene *rectangle);

/**
 * \brief       Dessine le plus petit rectangle contenant l'arène
 
 * \param       imgInput    Pointeur sur l'image d'entrée.
 * \param       imgOutput   Pointeur sur l'image de sortie (image d'entrée + arène marquée)
 * \param       monArene    Pointeur de type Arène contenant les information à dessiner
 */
void draw_arena(Image *imgInput, Image *imgOutput, Arene *monArene);

/**
 * \brief       Détecte la position d'un robot.
 * \details     Détecte la position de triangles blanc sur une image /a imgInput passé en paramètre d'entrer.
 *             
 * \param       imgInput    Pointeur sur l'image sur laquelle chercher la position du des robots.
 * \param       posTriangle Pointeur sur un tableau de position ou seront stocké les positions des triangles détectés.
 * \param       monArene    Pointeur de type Arène si nécessaire d'affiner la recherche (optionnel) 
 * \return      Le nombre de triangles détectés.
 */
int detect_position(Image *imgInput, Position *posTriangle, Arene * monArene = NULL);

/**
 * \brief       Dessine sur une image en entrée la position d'un robot et sa direction.
 * \details     Sauvegarde l'image des coordonnées passées par positionRobot superposée à l'image d'entrée sur imgOutput.
 *             
 * \param       imgInput         Pointeur sur l'image d'entrée
 * \param       imgOutput        Pointeur sur l'image de sortie ( image d'entrée + dessin de la position)
 * \param       positionRobot    Pointeur sur la structure position d'un robot.
 */
void draw_position(Image *imgInput, Image *imgOutput, Position *positionRobot);

/**
 * \brief       Détecte la position d'un robot.
 * \details     Détecte la position de triangles blanc sur une image /a imgInput passé en paramètre d'entrer.
 *             
 * \param       imgInput      	Pointeur sur l'image à sauvegarder en mémoire sous format jpg.
 * \param       imageCompress	Pointeur sur une image .jpg.
 */
void compress_image(Image *imgInput, Jpg *imageCompress);

#endif // _IMAGE_H_
