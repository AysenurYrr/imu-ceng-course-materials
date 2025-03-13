import numpy as np 
import cv2    
from   matplotlib import pyplot as plt 
import time as tm

fig     = plt.figure
subplot = plt.subplot 
imshow  = plt.imshow 
imread  = cv2.imread
plot    = plt.plot
#%%
import time as tm

I = imread("B.jpg" , 0)


h = np.zeros(256)
i_val = np.arange(0,256)
str_time = tm.time()
for i in range(I.shape[0]):
    for j in range(I.shape[1]):
        tmp_int = I[i,j]
        h[tmp_int] += 1
stp_time = tm.time()    

print(f" Fonksiyon suresi : {stp_time-str_time}")
#%%

h = np.zeros(256)
str_time = tm.time()
for i in range(0,256):
    h[i] = np.sum(I == i)
stp_time = tm.time()    

print(f" Fonksiyon suresi : {stp_time-str_time}")
    
def hist_fun(I):
    h = np.zeros(256)
    for i in range(0,256):
        h[i] = np.sum(I == i)
    return h
#%%
str_time = tm.time()

cv2.calcHist( [I] , [0] , None , [256] , [0,256] )
stp_time = tm.time() 

print(f" Fonksiyon suresi : {stp_time-str_time}") 
#%%
I = imread("C.jpg" , 0)

I_ort = np.mean(I)

I_std = np.sqrt( np.sum((I-I_ort)**2)*1/(I.shape[0] * I.shape[1]) )

print([I_ort , I_std])
fig()
imshow(I , cmap = "gray" , vmin = 0 , vmax = 255)
#%%

I = imread("C.jpg" , 0)

I_zm = I - np.mean(I)

I_nr = I_zm / np.max(np.abs(I_zm))

I_128 = 128*I_nr

I_y0 = np.uint8(I_128 + 127)

fig()
subplot(1,2,1) , imshow(I , cmap = "gray" , vmin = 0 , vmax = 255)
subplot(1,2,2) , imshow(I_y0 , cmap = "gray" , vmin = 0 , vmax = 255)


h = hist_fun(I_y0)

fig()
plot(h)







