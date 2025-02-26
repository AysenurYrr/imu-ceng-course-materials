from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

def init():
    glEnable(GL_DEPTH_TEST)  # Derinlik testini etkinleştir
    glEnable(GL_LIGHTING)    # Aydınlatmayı etkinleştir
    glEnable(GL_LIGHT0)      # İlk ışık kaynağını etkinleştir

    # Ambient light (çevresel aydınlatma) özelliklerini tanımla
    ambient_light = [0.2, 0.2, 0.2, 1.0]  # RGB ve alfa kanalı
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light)

    # Diffuse light (yayınmış aydınlatma) özelliklerini tanımla
    diffuse_light = [0.8, 0.8, 0.8, 1.0]  # RGB ve alfa kanalı
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light)

    # Specular light (yansıyan ışık) özelliklerini tanımla
    specular_light = [1.0, 1.0, 1.0, 1.0]  # RGB ve alfa kanalı
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular_light)

    # Işık kaynağının pozisyonunu ayarla
    light_position = [1.0, 1.0, 1.0, 0.0]  # X, Y, Z ve W (W=0 yön ışığı demektir)
    glLightfv(GL_LIGHT0, GL_POSITION, light_position)

    # Malzeme özelliklerini tanımla
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, [1.0, 0.5, 0.31, 1.0])  # Küre rengini ayarla
    glMaterialfv(GL_FRONT, GL_SPECULAR, [1.0, 1.0, 1.0, 1.0])  # Specular rengini ayarla
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0)  # Parlaklık katsayısı

def display():
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)  # Renk ve derinlik tamponlarını temizle
    glLoadIdentity()  # Varsayılan model-görüş matrisini yükle

    # Küreyi çiz
    glTranslatef(0.0, 0.0, -5.0)  # Küreyi biraz geriye taşı
    glutSolidSphere(1.0, 50, 50)  # Yarıçapı 1 olan bir küre çiz

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
    glutCreateWindow("PyOpenGL Specular Lighting Example")
    init()
    glutDisplayFunc(display)
    glutReshapeFunc(reshape)
    glutMainLoop()

if __name__ == "__main__":
    main()
