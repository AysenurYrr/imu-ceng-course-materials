import numpy as np
import cv2
from  matplotlib import pyplot as plt 
from scipy import signal as sg

imread      = cv2.imread
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

from scipy import signal as sg 

conv2 = sg.convolve2d 

I = np.float32( imread("I2.png" , 0) )


N = 27
h = 1/(N**2) * np.ones((N,N))

Iy = conv2(I,h , mode = "same")

cla()

fig()
subplot(1,2,1) , imshow(I , cmap = "gray")
subplot(1,2,2) , imshow(Iy , cmap = "gray")


Nf = [1,7,11,21]
fig()
for i,K in enumerate(Nf):
    
    N = K
    h = 1/(N**2) * np.ones((N,N))

    Iy = conv2(I,h , mode = "same")
    
    subplot(2,2,i+1), imshow(Iy , cmap = "gray")
    
#%%



exp = np.exp 
sgm = 3

x,y = np.meshgrid(  np.arange(-3*sgm , 3*sgm+1  ) ,
                    np.arange(-3*sgm , 3*sgm+1  ))

# Nf = 21
# x,y = np.meshgrid(  np.linspace(-3*sgm , 3*sgm , Nf) ,
#                     np.linspace(-3*sgm , 3*sgm , Nf))



mu_x , mu_y = 0,0

h = exp(   -( (x-mu_x)**2 + (y - mu_y)**2  ) / (2*sgm**2))


fig()

imshow(h)


def gauss_filt( sgm = 1):
    x,y = np.meshgrid(  np.arange(-3*sgm , 3*sgm+1  ) ,
                        np.arange(-3*sgm , 3*sgm+1  ))
    
    mu_x , mu_y = 0,0

    h = exp(   -( (x-mu_x)**2 + (y - mu_y)**2  ) / (2*sgm**2))
    
    return h


    
#%%


Nf = [1 , 3 , 5 , 7]

fig()
for i , K in enumerate(Nf):
    h = gauss_filt(K)
    
    Iy = conv2(I , h , mode = 'same')
    
    subplot(2,2,i+1), imshow(Iy , cmap = "gray")
    

    
#%% Görüntü keskinleştirme 


Ns = 5
h  = np.ones((Ns,Ns)) * 1/(Ns**2)  
 

I_bl = sg.convolve2d(I , h , mode='same')

I_D = I - I_bl
 
fig()
subplot(1,3,1) , imshow( I , cmap = 'gray' , vmin = 0 , vmax = 255 )
subplot(1,3,2) , imshow(I_D , cmap = 'gray' ) 
subplot(1,3,3) , imshow(I+5*I_D , cmap = 'gray' , vmin = 0 , vmax = 255 ) 



#%% Gürültü Temizleme
 
cla()
I = np.float32(imread('I2.png' , 0) )

I_ns = I + 30*np.random.randn(I.shape[0] , I.shape[1])

fig()
subplot(1,2,1) , imshow( I , cmap = 'gray' , vmin = 0 , vmax = 255 )
subplot(1,2,2) , imshow( I_ns , cmap = 'gray' , vmin = 0 , vmax = 255   )


fig()

plot(I_ns[155,:])

#%% Gürültü Temizleme
Ns = 21 # 3 5 11

h  = np.ones((Ns,Ns)) * 1/(Ns**2) 
I_bl_bx = sg.convolve2d(I_ns , h , mode='same')

fig()
subplot(1,2,1) , imshow( I_ns , cmap = 'gray' , vmin = 0 , vmax = 255 )
subplot(1,2,2) , imshow(I_bl_bx , cmap = 'gray' , vmin = 0 , vmax = 255 ) 


