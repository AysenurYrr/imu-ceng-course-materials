import numpy as np
import cv2   as cv 
from matplotlib import  pyplot as plt
import time as tm

imread      = cv.imread
fig         = plt.figure
subplot     = plt.subplot 
plot        = plt.plot
imshow      = plt.imshow
sumf        = np.sum
shape       = np.shape
size        = np.size

def cla():
    return plt.close("all")
#%%

I = imread("A.jpg" , 0)

fig()
imshow(I , cmap = "gray" , vmin = 0 , vmax = 255)


def myhist(I):
    hist = np.zeros(256)
    for i in range(256):
        hist[i] = sumf(I == i)
    
    return hist

h = myhist(I)

fig()
plot(h)

def stdfun(I):
    
    h = myhist(I)
    
    p = h/sumf(h)
    
    x = np.arange(0,256)
    
    mu = sumf(x*p)
    
    sd = np.sqrt(sumf( (x-mu)**2 * p))
    
    return (mu,sd)

mu , sd = stdfun(I)
#%%

I1 = imread("A.jpg" , 0)
I2 = imread("B.jpg" , 0)
I3 = imread("C.tif" , 0)
I4 = imread("C.jpg" , 0)

_ , sd1 = stdfun(I1)
_ , sd2 = stdfun(I2)
_ , sd3 = stdfun(I3)
_ , sd4 = stdfun(I4)

fig()
subplot(1,4,1) , imshow(I1 , cmap = "gray" , vmin = 0 , vmax = 255)
subplot(1,4,2) , imshow(I2 , cmap = "gray" , vmin = 0 , vmax = 255)
subplot(1,4,3) , imshow(I3 , cmap = "gray" , vmin = 0 , vmax = 255)
subplot(1,4,4) , imshow(I4 , cmap = "gray" , vmin = 0 , vmax = 255)

#%%
cla()
Ix = imread("C.jpg" , 0)

h = myhist(Ix)

p = h/sumf(h)

F = np.cumsum(p)

fig()
plot(p/np.max(p)) , plot(F , "--r")


Iy = np.zeros_like(Ix)

for i in range(256):
    I_mask = Ix == i
    
    Iy[I_mask] = np.uint8(255*F[i])

fig()
subplot(1,2,1) , imshow(Ix , cmap = "gray" , vmin = 0 , vmax = 255)
subplot(1,2,2) , imshow(Iy , cmap = "gray" , vmin = 0 , vmax = 255)

hy = myhist(Iy)
py = hy/sumf(hy)

Fy = np.cumsum(py)

fig()
subplot(2,1,1) , plot(p/np.max(p)) , plot(F , "--r")
subplot(2,1,2) , plot(py/np.max(py)) , plot(Fy , "--r")
#%%


from skimage.exposure import match_histograms


Ir = imread('Ref2.jpg'); Ir = cv.cvtColor(Ir, cv.COLOR_BGR2RGB )
Is = imread('Src.jpg'); Is = cv.cvtColor(Is, cv.COLOR_BGR2RGB )
fig()

subplot(1,2,1) , imshow(Is  , vmin = 0 , vmax = 255)
subplot(1,2,2) , imshow(Ir  , vmin = 0 , vmax = 255)

matched = match_histograms(Is, Ir, 
                           multichannel=True,)   
fig()

subplot(1,3,1) , imshow(Is)
subplot(1,3,2) , imshow(Ir)
subplot(1,3,3) , imshow(matched)










    
    
    
    
    
    
    
    
 

