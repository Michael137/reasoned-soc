import numpy as np
import cv2
import os
import sys
import numpy as np

directory = sys.argv[1]

if len(sys.argv) > 2:
    silent = sys.argv[2] == "-q"
else:
    silent = False

for filename in os.listdir(directory):
    if filename.endswith(".jpg"):
        img = os.path.join(directory, filename)
        if not silent:
            print(img)
        frame = cv2.imread(img)
        # Resize frame with Required image size
        frame_resized = cv2.resize(frame,(513,513))
        # Adding Mean & Multiplying with 0.7843
        blob = cv2.dnn.blobFromImage(frame_resized, 0.007843, (513, 513), (127.5, 127.5, 127.5), swapRB=True)
        # Making numpy array of required shape
        blob = np.reshape(blob, (1,513,513,3))
        # Storing to the file
        newfilename = filename.split(".jpg")[0] + '.raw'
        np.ndarray.tofile(blob, open(newfilename,'w') )
    else:
        continue
