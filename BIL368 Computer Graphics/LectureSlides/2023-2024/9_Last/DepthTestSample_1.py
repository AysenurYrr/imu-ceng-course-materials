from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

def init():
    glEnable(GL_DEPTH_TEST)  # Derinlik testini etkinleştir


def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)  # Renk ve derinlik tamponlarını temizle
    glLoadIdentity()  # Varsayılan model-görüş matrisini yükle

    # İlk küreyi çiz (ön planda)
    glPushMatrix()
    glTranslatef(-0.5, 0.0, -3.0)  # Küreyi konumlandır
    glColor3f(1.0, 0.0, 0.0)  # Kırmızı renk
    glutSolidSphere(0.5, 50, 50)  # Yarıçapı 0.5 olan bir küre çiz
    glPopMatrix()

    # İkinci küreyi çiz (arka planda)
    glPushMatrix()
    glTranslatef(0.5, 0.0, -5.0)  # Küreyi konumlandır
    glColor3f(0.0, 0.0, 1.0)  # Mavi renk
    glutSolidSphere(0.9, 50, 50)  # Yarıçapı 0.5 olan bir küre çiz
    glPopMatrix()

    glutSwapBuffers()  # Çift tamponlamayı kullanarak çizimi güncelle

def reshape(w, h):
    if h == 0:
        h = 1
    glViewport(0, 0, w, h)  # Görüntüleme alanını ayarla
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluPerspective(45, w / h, 1, 50.0)  # Perspektif projeksiyon matrisi oluştur
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()

def main():
    glutInit()
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)  # Çift tamponlama ve RGB renk modeli
    glutInitWindowSize(800, 600)
    glutCreateWindow("PyOpenGL Depth Test Example")
    init()
    glutDisplayFunc(display)
    glutReshapeFunc(reshape)
    glutMainLoop()

if __name__ == "__main__":
    main()
