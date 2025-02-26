"""
2324B-BİL490 Sayısal Görüntü İşleme
Hafta-3
"""

import numpy as np 
import cv2   as cv 
from   matplotlib import pyplot as plt 

fig     = plt.figure
subplot = plt.subplot 
imshow  = plt.imshow 
imread = cv.imread 
plot = plt.plot

def cla():
    return plt.close('all') 
#%%



I = imread("I2.png",0).astype("float")

fig()
imshow(I,cmap = "gray")


sg = 3

x = np.arange(-10,10,0.01 )

def exp_img(I_var , cnt , sgm):
    
    return np.exp(-(I_var-cnt)**2/(2*sgm**2))


Iy = exp_img(I , 50 , 15)

fig()
imshow(Iy , cmap= "gray"   )

#%%

import time as tm

I = imread("A.jpg",0) 


str_time = tm.time()
hist_vect = np.zeros(256)
M,N = I.shape
for i in range(M):
    for j in range(N):
        tmp_pix = I[i,j]
        
        hist_vect[  tmp_pix ] += 1
stp_time = tm.time()

print(stp_time-str_time)
fig()

plot(hist_vect)


#%%
str_time = tm.time()
hist_vect = np.zeros(256)

for i in range(256):
    hist_vect[i] = np.sum(I == i)
stp_time = tm.time()

print(stp_time-str_time)

str_time = tm.time()
for i in range(200):
    cv_hist = cv.calcHist([I] , [0] , None,[256], [0,256])
stp_time = tm.time()
print(stp_time-str_time)

print(stp_time-str_time)
#%%

cla()
I = imread("A.jpg" , 0)

fig()

imshow(I,cmap = "gray" , vmin = 0 , vmax = 255)

cv_hist = cv.calcHist([I] , [0] , None,[256], [0,256])

fig()
plot(cv_hist)

I0 = I.astype("float")

# I0 = I0-I0.mean()

I0 -= I0.mean()

I0 /= np.max(np.abs(I0))

I0 *= 128

I0 += 128

I0-=1

fig()
subplot(1,2,1) , imshow(I,cmap = "gray" , vmin = 0 , vmax = 255)
subplot(1,2,2) , imshow(I0,cmap = "gray" , vmin = 0 , vmax = 255)










