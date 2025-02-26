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
#%% 2B Yönlü Sinüsoidaller 
Nt             = 256 
grid_range     = np.arange(0 , Nt+1 )
[nx,ny]        = np.meshgrid( grid_range , grid_range )

Nx0 = 32 # x 
Ny0 = 128

fx0 = 1/Nx0
fy0 = 1/Ny0

wx0 = 2*pi*fx0
wy0 = 2*pi*fy0
 
I = sin(wx0*nx + wy0*ny) # + - ve yön

plt.figure( )
plt.imshow(I, cmap = 'gray',
       extent = [grid_range.min() , grid_range.max() , grid_range.min() , grid_range.max() ]  )

#%% 2B Yönlü Sinüsoidaller : 0.5'in önemi ve frekans değerlerinden periyodun elde edilmesi

F_I = fftshift(np.abs(fft2(I)))

plt.figure( )
plt.imshow(F_I , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])

#%% 2B Yönlü Sinüsoidaller : 3 farklı işaretin toplanması


Nx0 = np.array([4,64,64])
Ny0 = np.array([64,64,4])

wx0 = 2*pi/Nx0
wy0 = 2*pi/Ny0

I3 = np.zeros((Nt+1,Nt+1))

for i in range(len(Nx0)):
    
    I3 = I3 + sin(wx0[i]*nx + wy0[i]*ny)
    
    
plt.figure("Figure 3.9(a)")
plt.imshow(I3, cmap = 'gray',
       extent = [grid_range.min() , grid_range.max() , grid_range.min() , grid_range.max() ] )


F_I = fftshift(np.abs(fft2(I3)))

plt.figure('Figure 3.9(b)')
plt.imshow(F_I , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])



#%% Kare (I2 - F2 - I1) (2048, 2048)
# cla()
I   = imread('img/F1.png' , 0)
F_I = fftshift(np.abs(fft2( I , (2048, 2048) )))


# plt.close('all')

 
fig( )
subplot(1,2,1) , imshow(I, cmap = 'gray' )
subplot(1,2,2) , imshow(np.log(F_I+1 ) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])   # np.log
 
# np.log !!
#%% Birim İmpuls

plt.close('all')
I = np.zeros( (Nt,Nt) )

I[ int(Nt/2) , int(Nt/2)] = 1
 
F_I = fftshift(np.abs(fft2(I)))

fig('Impuls isareti')
subplot(1,2,1) , imshow(I, cmap = 'gray' )
subplot(1,2,2) , imshow(F_I , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])


 
#%% Harfler A - E - E2 - O , (2048,2048)

I   = imread('img/O.png' , 0)
F_I = fftshift(np.abs(fft2(I)))


plt.close('all')

 
fig('Kare isareti')
subplot(1,3,1) , imshow(I, cmap = 'gray' )
subplot(1,3,2) , imshow(F_I , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])
subplot(1,3,3) , imshow(np.log(F_I+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])


#%% Görüntülerin FD'si : Ia Ib

# I   = imread('img/I3.png' , 0)
# I   = imread('img/S.png' , 0)
I   = imread('img/MIT.jpg' , 0)
F_I = fftshift(np.abs(fft2(I)))


plt.close('all')

fig( )
subplot(1,2,1) , imshow(I, cmap = 'gray' )
subplot(1,2,2) , plot(I.sum(0))

fig( )
subplot(1,3,1) , imshow(I, cmap = 'gray' )
subplot(1,3,2) , imshow(F_I , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])
subplot(1,3,3) , imshow(np.log(F_I+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ])

#%% Sıkıştırma https://inst.eecs.berkeley.edu/~ee123/sp16/Sections/JPEG_DCT_Demo.html  

from scipy import fftpack

def dct2(a):
    return  fftpack.dct(  fftpack.dct( a, axis=0, norm='ortho' ), axis=1, norm='ortho' )

def idct2(a):
    return  fftpack.idct(  fftpack.idct( a, axis=0 , norm='ortho'), axis=1 , norm='ortho')



I   = imread('img/S.png' , 0)

Ni = I.shape[0]

I_tr = I.copy() 

DCT_I = dct2( I_tr )


fig( )
subplot(1,3,1) , imshow(I_tr, cmap = 'gray' )
subplot(1,3,2) , imshow(np.abs(DCT_I)  )
subplot(1,3,3) , imshow(np.log( np.abs(DCT_I) + 1)  )

#%%
Ns = 200
Ni = I_tr.shape[0]

crop_DCT = DCT_I[0:Ns , 0:Ns]
# crop_DCT2 = crop_DCT[0:2:Ns , 0:2:Ns]
#---> İletim Hattı ----> 

DCT_temp = np.zeros((Ni,Ni))
DCT_temp[0:Ns , 0:Ns] = crop_DCT


Ir = idct2(DCT_temp)

fig( )
subplot(1,3,1) , imshow(I_tr, cmap = 'gray' )
subplot(1,3,2) , imshow(Ir, cmap = 'gray'  )
subplot(1,3,3) , imshow(np.log( np.abs(DCT_temp) + 1)  )

print(1-Ns**2/Ni**2)

#%% Kenar Bulan Filtreler
cla()
from scipy import signal as sg
#Prewitt
h = ary( [[ 1, 1,  1] , 
          [ 0, 0,  0] ,
          [-1, -1, -1]])

#Sobel
# h = ary( [[ 1, 2,  1] , 
#           [ 0, 0,  0] ,
#           [-1, -2, -1]])


# Laplacian
# h = ary( [[ 0, -1,  0] , 
#           [ -1, 4,  -1] ,
#           [0, -1, 0]])

## Laplacian 8-N
# h = ary( [[ -1, -1,  -1] , 
#           [ -1, 8,  -1] ,
#           [-1, -1, -1]])




Ix = cv.imread('img/E.png' , 0)

F_I  = fftshift(abs(fft2(Ix)))
F_h  = fftshift(abs(fft2(h , Ix.shape)))
F_Iy = F_I*F_h

Iy_open = cv.filter2D(Ix,-1,h)
Iy_conv = sg.convolve2d(Ix, h )

Iy_fft2 = ifft2(fft2(Ix)*fft2(h , Ix.shape))

fig()
subplot(2,3,1) , imshow( np.log(F_I+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]     )
subplot(2,3,2) , imshow( F_h , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]     )
subplot(2,3,3) , imshow( np.log(F_Iy+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]    ) 

subplot(2,3,4) , imshow( Ix , cmap = 'gray'     )
subplot(2,3,5) , imshow( h , cmap = 'gray'    )
subplot(2,3,6) , imshow( np.abs(Iy_open), cmap = 'gray'   ) 


fig()
subplot(1,3,1) , imshow( np.abs(Iy_open), cmap = 'gray'     )
subplot(1,3,2) , imshow( np.abs(Iy_conv), cmap = 'gray'    )
subplot(1,3,3) , imshow( np.abs(Iy_fft2), cmap = 'gray'    ) 

#%% Gürültü Temizleme



Ix = cv.imread('Ia.png' , 0)

Ins = Ix + 50*np.random.randn(Ix.shape[0] , Ix.shape[1])

F_I  = fftshift(abs(fft2(Ix)))
F_In  = fftshift(abs(fft2(Ins)))

fig()
subplot(2,2,1) , imshow( Ix , cmap = 'gray')
subplot(2,2,2) , imshow( Ins , cmap = 'gray')  

subplot(2,2,3) , imshow( np.log(F_I+1)  )
subplot(2,2,4) , imshow( np.log(F_In+1)  )  
#%% Gürültü Temizleme

sgm   = 5
xv,yv = np.meshgrid( np.arange( -5*sgm , 5*sgm+1 ) , np.arange( -5*sgm , 5*sgm+1 ) ) 
h_gs  = exp( - (xv**2+yv**2) / (2*sgm**2)) 
F_H   = fftshift(np.abs(fft2(h_gs))) 


Iy     = cv.filter2D(Ins,-1,h_gs)
F_Iy   = fftshift(np.abs(fft2(Iy))) 



plt.figure( )
plt.subplot(2,3,1) , plt.imshow(  Ins    , cmap = 'gray'  )
plt.subplot(2,3,2) , plt.imshow(  h_gs  , cmap = 'gray'  )
plt.subplot(2,3,3) , plt.imshow(  np.abs(Iy)    , cmap = 'gray'  )

plt.subplot(2,3,4) , plt.imshow( np.log(F_In+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )
plt.subplot(2,3,5) , plt.imshow(   (F_H+1)  , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )
plt.subplot(2,3,6) , plt.imshow( np.log(F_Iy+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )



#%% Filtreleme : Gauss filtresi hatırlatma
cla()
from scipy import signal as sg

I  = imread('img/S.png' , 0)
pi = np.pi

sgm = 3
K = 5
xv,yv = np.meshgrid( np.arange( -K*sgm , K*sgm+1  ) , np.arange( -K*sgm , K*sgm+1 ) ) 
    
h_gs  = 1/ (2*pi*sgm**2)*np.exp( - (xv**2+yv**2) / (2*sgm**2))  
    
I_bl_gs = sg.convolve2d(I , h_gs , mode='same')

fig()
subplot(1,3,1) , imshow( I    , cmap = 'gray'   )
subplot(1,3,2) , imshow( h_gs , cmap = 'gray'    )
subplot(1,3,3) , imshow( I_bl_gs   , cmap = 'gray'  ) 

#%%  Filtreleme : Uzamsalda Konv. Frekansta Çarpma 







Iy = ifft2(fft2(I)*fft2(h_gs , I.shape))

fig()
subplot(1,3,1) , imshow( I    , cmap = 'gray'   )
subplot(1,3,2) , imshow( I_bl_gs , cmap = 'gray'    )
subplot(1,3,3) , imshow(   np.real(Iy) , cmap = 'gray'  ) 


F_I  = fftshift(abs(fft2(I)))
F_h  = fftshift(abs(fft2(h_gs , I.shape)))
F_Iy = F_I*F_h

fig()
subplot(1,3,1) , imshow( np.log(F_I+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]     )
subplot(1,3,2) , imshow( F_h , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]     )
subplot(1,3,3) , imshow( np.log(F_Iy+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]    ) 
#%%

fig()
plot(h_gs.sum(0))


#%% Filtreleme : Filtre boyutu etkileri





sgm = 1
xv,yv = np.meshgrid( np.arange( -3*sgm , 3*sgm+1  ) , np.arange( -3*sgm , 3*sgm+1 ) ) 
    
h_gs  = 1/ (2*pi*sgm**2)*np.exp( - (xv**2+yv**2) / (2*sgm**2))  

F_I  = fftshift(abs(fft2(I)))
F_h  = fftshift(abs(fft2(h_gs , I.shape)))
F_Iy = F_I*F_h

Iy = np.real(ifft2(fft2(I)*fft2(h_gs , I.shape)))

fig()
subplot(2,3,1) , imshow( np.log(F_I+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]     )
subplot(2,3,2) , imshow( F_h , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]     )
subplot(2,3,3) , imshow( np.log(F_Iy+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ]    ) 

subplot(2,3,4) , imshow( I , cmap = 'gray'     )
subplot(2,3,5) , imshow( h_gs , cmap = 'gray'    )
subplot(2,3,6) , imshow( Iy, cmap = 'gray'   ) 


#%% Gizli Metin Uygulaması

plt.close('all')

Ix = cv.imread('img/Text.png' , 0)

Nxo = np.array([    4,8 ,12     ])
Nyo = np.array([    4,8 ,12    ])

N = 512

xv,yv = np.meshgrid( np.arange(0,N) , np.arange(0,N) )

I = np.zeros((N ,N ))

for i,fxo in enumerate(1/Nxo):
    for j,fyo in enumerate(1/Nyo):
        
        I = I + cos(2*pi*(fxo*xv + fyo*yv) )
         
Ix = 1000*I + Ix
     
plt.figure()
plt.imshow( Ix , cmap = 'gray'  )

F_Ix = fftshift(np.abs(fft2(Ix)))

 
plt.figure('Revealing Hidden Text-1')
plt.subplot(1,3,1) , plt.imshow( Ix             , cmap = 'gray'  )
plt.subplot(1,3,2) , plt.imshow( F_Ix           , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )
plt.subplot(1,3,3) , plt.imshow( np.log(F_Ix+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )

#%% Metnin Geri Elde Edilmesi (sgm: 1 3 5 9)
sgm   = 5
xv,yv = np.meshgrid( np.arange( -5*sgm , 5*sgm+1 ) , np.arange( -5*sgm , 5*sgm+1 ) ) 
h_gs  = exp( - (xv**2+yv**2) / (2*sgm**2)) 
F_H   = fftshift(np.abs(fft2(h_gs))) 


Iy     = cv.filter2D(Ix,-1,h_gs)
F_Iy   = fftshift(np.abs(fft2(Iy))) 



plt.figure('Revealing Hidden Text-2')
plt.subplot(2,3,1) , plt.imshow(  Ix    , cmap = 'gray'  )
plt.subplot(2,3,2) , plt.imshow(  h_gs  , cmap = 'gray'  )
plt.subplot(2,3,3) , plt.imshow(  Iy    , cmap = 'gray'  )

plt.subplot(2,3,4) , plt.imshow( np.log(F_Ix+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )
plt.subplot(2,3,5) , plt.imshow(   (F_H+1)  , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )
plt.subplot(2,3,6) , plt.imshow( np.log(F_Iy+1) , extent = [-0.5 , 0.5 , -0.5 , 0.5 ] )






 

 