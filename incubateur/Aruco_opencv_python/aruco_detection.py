import cv2 as cv
import cv2.aruco as aruco
from calibrate import calculareCameraCalibration

cap = cv.VideoCapture(0)
ret, mtx, dist, rvecs, tvecs = calculareCameraCalibration()
parameters = aruco.DetectorParameters_create()
aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)
drawAxis = True

while (True):

    ret, frame = cap.read()
    gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

    corners, ids, rejectedImgPoints = aruco.detectMarkers(gray, aruco_dict, parameters=parameters)

    frame = aruco.drawDetectedMarkers(frame, corners, ids)
    rvecs, tvecs, _ = aruco.estimatePoseSingleMarkers(corners, 0.05, mtx, dist)
    if ids is not None:
        for idx, marker in enumerate(ids):
            print(corners[idx][0]) #Affiche les coordonée tout les coins Top Left
            try:
                if drawAxis is True:
                    aruco.drawAxis(frame, mtx,dist,rvecs[idx], tvecs[idx], 0.1)
            except:
                print(" Erreur calcul des axes")

    cv.imshow('frame', frame)
    if cv.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv.destroyAllWindows()