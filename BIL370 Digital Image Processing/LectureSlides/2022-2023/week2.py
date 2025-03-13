"""
                            22-23 Bahar Dönemi
                      BİL 370 Sayısal Görüntü İşleme
                    
Hafta-2: Görüntü okuma ve noktasal işlemler

"""


import numpy as np
import cv2 as cv
from matplotlib import pyplot as plt


fig     = plt.figure 
subplot = plt.subplot 
imshow  = plt.imshow 
imread  = cv.imread 

def cla():
    return plt.close("all")

#%%


I = imread("C.jpg" , 0)
I = I.astype("float")

# fig()
# imshow(I , cmap = "gray")

I0 = 5*I  

# fig()
# imshow(I0 , cmap = "gray" , vmin = 0 , vmax = 255)


fig()
subplot(1,3,1) , imshow(I , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,2) , imshow(I0 , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,3) , imshow(I0 , cmap = "gray" )

# a = np.uint8(50)
# b = a*6
# c = b.astype("uint8")

#%% exponansiyel fonk.

a = 3
I0 = 255*(I/255)**a

fig()
subplot(1,3,1) , imshow(I , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,2) , imshow(I0 , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,3) , imshow(I0 , cmap = "gray" )

#%% log fonk.

N = 8
K = 2**N

I0 = K/np.log(K) * np.log(I+1) 

fig()
subplot(1,3,1) , imshow(I , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,2) , imshow(I0 , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,3) , imshow(I0 , cmap = "gray" )


#%% 3. örnek

cla()

N1 = 120
N2 = 180

I_mask1 = I>120
I_mask2 = I<180

I_mask3 =  np.logical_and(I_mask1 , I_mask2)  

I0 = I*I_mask3


fig()
subplot(1,3,1) , imshow(I , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,2) , imshow(I0 , cmap = "gray", vmin = 0 , vmax = 255)
subplot(1,3,3) , imshow(I0 , cmap = "gray" )








