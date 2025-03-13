import numpy as np
from scipy.fft import fft2 , fftshift, ifft2
from  matplotlib import pyplot as plt
import cv2 as cv

pi  = np.pi
cos = np.cos
sin = np.sin 
exp = np.exp 

imread      = cv.imread
fig         = plt.figure
subplot     = plt.subplot 
plot        = plt.plot
imshow      = plt.imshow
sumf        = np.sum
shape       = np.shape
size        = np.size
ary         = np.array 

def cla():
    return plt.close('all')

#%%

N = 128

grid_range = np.arange(0,N+1)

[nx , my] = np.meshgrid( grid_range , grid_range )


Nx0 = 64; Fx0 = 1/Nx0
Ny0 = 8; Fy0 = 1/Ny0



I = sin(2*pi*(Fx0*nx + Fy0*my))

fig()
imshow(I , cmap = "gray" )
#%%


F_I = abs(fftshift(fft2(I)))

fig()
subplot(1,2,1) , imshow(I , cmap = "gray" )
subplot(1,2,2) , imshow(F_I  , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )

#%%

cla()
Nx0 = np.array([64 , 3 , 3])
Ny0 = np.array([3 , 3 , 64])


Nz = zip(Nx0,Ny0)

I_tmp = np.zeros_like(I)
for nx0 , ny0 in Nz:
    I_tmp += sin(2*pi/nx0 * nx +  2*pi/ny0 * my)

# fig()
# imshow(I_tmp , cmap = "gray" )

F_I = abs(fftshift(fft2(I_tmp)))

fig()
subplot(1,2,1) , imshow(I_tmp , cmap = "gray" )
subplot(1,2,2) , imshow(F_I  , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )


#%%


cla()
I   = imread('I3.png' , 0)
F_I = fftshift(np.abs(fft2( I , (2048, 2048) )))


# plt.close('all')

 
fig( )
subplot(1,2,1) , imshow(I, cmap = 'gray' )
subplot(1,2,2) , imshow(np.log(F_I+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])   # np.log
 


#%%

Nt = 128

cla()
I = np.zeros( (Nt,Nt) )

I[ int(Nt/2) , int(Nt/2)] = 1
 
F_I = fftshift(np.abs(fft2(I)))

fig('Impuls isareti')
subplot(1,2,1) , imshow(I, cmap = 'gray' )
subplot(1,2,2) , imshow(F_I , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])









