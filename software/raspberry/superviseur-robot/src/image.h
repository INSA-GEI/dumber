/**
 * \file      imagerie.h
 * \author    L.Senaneuch
 * \version   1.0
 * \date      06/06/2017
 * \brief     Fonctions de traitement d'image utilisable pour la d�tection du robot.
 *
 * \details   Ce fichier utilise la libraire openCV2 pour faciliter le traitement d'image dans le projet Destijl.
 *            Il permet de faciliter la d�tection de l'ar�ne et la d�tection du robot.
 *			  /!\ Attention Bien que celui-ci soit un .cpp la structure du code n'est pas sous forme d'objet.
 */


#ifndef IMAGERIE_H
#define IMAGERIE_H

#ifndef __STUB__
#include <raspicam/raspicam_cv.h>
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
using namespace raspicam;
#endif

typedef Mat Image;
#ifndef __STUB__
typedef RaspiCam_Cv Camera;
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
 * \brief      Ouvre une camera.
 * \details    Met à jour le descripteur de fichier passé en paramètre pour correspondre à la camera ouverte
 *             
 * \param    *Camera      	Pointeur d'un file descriptor d'une camera ouverte
 * \return retourne 0 si la camera a été ouverte correctement et -1 si une erreur survient.
 */
int open_camera(Camera *camera);

/**
 * \brief      Ferme la camera passé en paramètre
 *             
 * \param    *Camera       Pointeur sur la camera à fermer
 */
void close_camera(Camera *camera);

/**
 * \brief      Capture une image avec la camera passée en entrée. En cas de test
 * sans camera, la fonction charge une image 
 * \details    La camera doit préalablement être ouverte via \a openCamera(...)
 *             
 * \param    *Camera       Pointeur sur la camera passée en entrée.
 * \param    *monImage 	   Pointeur sur une image capturée. 
 * \param    *fichier         chemin du fichier d'image
 * \return    Retourne -1 si une erreur survient.
 */
void get_image(Camera *camera, Image * monImage, const char *fichier = NULL);

/**
 * \brief      Détecte une arène dans une image fournis en paramètre.
 *             
 * \param    *monImage       Pointeur sur l'image d'entrée
 * \param    *rectangle 	 Pointeur sur les coordonnées du rectangles trouvé. 
 * \return    Retourne -1 si aucune arène n'est détectée. Sinon retourne 0
 */
int detect_arena(Image *monImage, Arene *rectangle);

/**
 * \brief      Dessine le plus petit rectangle contenant l'arène
 
 * \param    *imgInput       Pointeur sur l'image d'entrée.
 * \param    *imgOutput      Pointeur sur l'image de sortie (image d'entrée + arène marquée)
 * \param    *monArene		 Pointeur de type Arène contenant les information à dessiner
 */
void draw_arena(Image *imgInput, Image *imgOutput, Arene *monArene);

/**
 * \brief      Détecte la position d'un robot.
 * \details    Détecte la position de triangles blanc sur une image /a imgInput passé en paramètre d'entrer.
 *             
 * \param    *imgInput       Pointeur sur l'image sur laquelle chercher la position du des robots.
 * \param    *posTriangle    Pointeur sur un tableau de position ou seront stocké les positions des triangles détectés.
 * \param    *monArene       Pointeur de type Arène si nécessaire d'affiner la recherche (optionnel) 
 * \return    Le nombre de triangles détectés.
 */
int detect_position(Image *imgInput, Position *posTriangle, Arene * monArene = NULL);

/**
 * \brief      Dessine sur une image en entrée la position d'un robot et sa direction.
 * \details    Sauvegarde l'image des coordonnées passées par positionRobot 
 * superposée à l'image d'entrée sur imgOutput.
 *             
 * \param      *imgInput	      	Pointeur sur l'image d'entrée
 * \param      *imgOutput    		Pointeur sur l'image de sortie ( image 
 * d'entrée + dessin de la position)
 * \param      *positionRobot   	Pointeur sur la structure position d'un robot.
 */
void draw_position(Image *imgInput, Image *imgOutput, Position *positionRobot);

/**
 * \brief      Détecte la position d'un robot.
 * \details    Détecte la position de triangles blanc sur une image /a imgInput passé en paramètre d'entrer.
 *             
 * \param    *imgInput      	Pointeur sur l'image à sauvegarder en mémoire sous format jpg.
 * \param    *imageCompress	Pointeur sur une image .jpg.
 */
void compress_image(Image *imgInput, Jpg *imageCompress);

#endif // IMAGERIE_H
