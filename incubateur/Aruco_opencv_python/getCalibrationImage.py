import cv2


cap = cv2.VideoCapture(0)
nbrImage = 0
while nbrImage < 15:
    ret, img = cap.read()
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Find the chess board corners
    ret, corners = cv2.findChessboardCorners(gray, (7,6),None)

    # If found, add object points, image points (after refining them)
    if ret == True:
        nbrImage = nbrImage+1
        cv2.imwrite("./img_calibrate/calibration_" + str(nbrImage) + ".jpg", img)
        cv2.putText(img, 'Image ' + str(nbrImage) + ' /15', (5, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2, cv2.LINE_AA)
        cv2.imshow('img',img)
        cv2.waitKey(1000)

cv2.destroyAllWindows()
