yil=int(input("Geri Ödeme süresi "))
aidat=int(input("Geri Ödeme miktarı "))
kredi=int(input("ÇEkilecek kredi "))
faiz_orani=float(input("Faiz oranı "))

if kredi>yil*aidat:
    print("Geçersizdir ")

BORC=kredi+kredi*faiz_orani
BORC=BORC-aidat
toplam_faiz=kredi*faiz_orani

for i in range(1,yil):
    BORC = BORC - aidat
    toplam_faiz+=BORC*faiz_orani
    BORC=BORC+BORC*faiz_orani

print("toplam faiz tutarı: ",toplam_faiz)









