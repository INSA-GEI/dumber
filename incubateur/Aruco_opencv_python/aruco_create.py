import cv2 as cv
import cv2.aruco as aruco

'''
    drawMarker(...)
        drawMarker(dictionary, id, sidePixels[, img[, borderBits]]) -> img
'''

aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50) # Un aruco est composé d'une bande noire et d'un code en blanc et noir
                                                     #4x4 indique la partie ou sera généré le code (blanc et noir donc).
print(aruco_dict)

for aruco_id in range(1,12):
    img = aruco.drawMarker(aruco_dict, aruco_id, 200)
    cv.imwrite("./img_aruco/aruco_"+str(aruco_id)+".jpg", img)
