import cv2 as cv 
from matplotlib import pyplot as plt 
import numpy as np 

imread = cv.imread
fig = plt.figure 
subplot = plt.subplot 
imshow = plt.imshow 
def cla():
    return plt.close("all")
#%%

cla()
I = imread("C.jpg",0).astype("float")
I1 = I + 100
fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(I1 , cmap = "gray")
subplot(1,3,3) , imshow(I1 , cmap = "gray" , vmin = 0, vmax = 125)

#%%

import cv2 as cv 
from matplotlib import pyplot as plt 
import numpy as np 

imread = cv.imread
fig = plt.figure 
subplot = plt.subplot 
imshow = plt.imshow 
def cla():
    return plt.close("all")
#%%

cla()
I = imread("C.jpg",0).astype("float")
I1 = I - I.mean() 
fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(I1 , cmap = "gray")
subplot(1,3,3) , imshow(I1 , cmap = "gray" , vmin = 0, vmax = 255)

#%%

cla()
I = imread("C.jpg",0).astype("float")
I1 = I*10
fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(I1 , cmap = "gray")
subplot(1,3,3) , imshow(I1 , cmap = "gray" , vmin = 0, vmax = 255)

#%% exp

I = imread("C.jpg",0).astype("float")
a = 6
I1 = 255*(I/255 )**a 

fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(I1 , cmap = "gray")
subplot(1,3,3) , imshow(I1 , cmap = "gray" , vmin = 0, vmax = 255)

#%% log
cla()
I = imread("C.jpg",0).astype("float")
 
I1 = np.log(I+1) 
I1 /= I1.max()
I1 *= 255

fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(I1 , cmap = "gray")
subplot(1,3,3) , imshow(I1 , cmap = "gray" , vmin = 0, vmax = 255)

#%% belli piksel grubu
cla()
I = imread("C.jpg",0).astype("float")

pix_alt = 120
pix_ust = 220

pix_mask = (I>pix_alt) & (I<pix_ust)

Iy = np.zeros_like(I)

Iy[pix_mask] = I[pix_mask]

fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(Iy , cmap = "gray")
subplot(1,3,3) , imshow(Iy , cmap = "gray" , vmin = 0, vmax = 255)
#%%
Iy = np.zeros_like(I)
for i in range(I.shape[0]):
    for j in range(I.shape[1]):
        
        tmp_pix = I[i,j]
        if (tmp_pix<pix_ust) and (tmp_pix>pix_alt):
            Iy[i,j] = tmp_pix


fig()
subplot(1,3,1) , imshow(I   , cmap = "gray")
subplot(1,3,2) , imshow(Iy , cmap = "gray")
subplot(1,3,3) , imshow(Iy , cmap = "gray" , vmin = 0, vmax = 255)


