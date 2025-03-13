from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys
import math
import random

#Kırmızı olarak koyu kırmızı kullandım ve başlangıçta araba kırmızı
R=0.6
G=0.0
B=0.0

translate_x = 0.0
translate_xBird = 0.0
vertices = []
faces = []
#Dikdörtgen çizmek için fonksiyon
def plotQuad():
    glBegin(GL_QUAD_STRIP)
    glVertex2f(0.0, 0.0)
    glVertex2f(1.0, 0.0)
    glVertex2f(0.0, 0.5)
    glVertex2f(1.0, 0.5)
    glEnd()

#Çizilen dikdörtgenlerin çerçevesini çizmek için fonksiyon
def plotLines():
    glLineWidth(3)
    glBegin(GL_LINES)

    glVertex2f(0.0, 0.0)
    glVertex2f(1.0, 0.0)

    glVertex2f(0.0, 0.0)
    glVertex2f(0.0, 0.5)

    glVertex2f(0.0, 0.5)
    glVertex2f(1.0, 0.5)

    glVertex2f(1.0, 0.5)
    glVertex2f(1.0, 0.0)

    glEnd()

#Tekerlekleri çizmek için daire çizen fonksiyon
def plotCircle():
    glPointSize(5.0)
    glBegin(GL_POLYGON)
    for i in range(360):
        x = 0.2 * math.sin((i) * 3.14 / 180)
        y = 0.2 * math.cos((i) * 3.14 / 180)
        glVertex2f(x, y)

    glEnd()

def drawTree():
    glPushMatrix()
    glScalef(1.7, 1.7, 1.0)  # Ağacı küçültmek için
    glColor3f(0.5, 0.35, 0.05)  # Kahverengi gövde rengi
    glBegin(GL_POLYGON)
    glVertex2f(-0.05, 0.0)
    glVertex2f(0.05, 0.0)
    glVertex2f(0.05, 0.4)
    glVertex2f(-0.05, 0.4)
    glEnd()

    glColor3f(0.0, 0.5, 0.0)  # Yeşil yaprak rengi
    glBegin(GL_POLYGON)
    glVertex2f(-0.2, 0.4)
    glVertex2f(0.2, 0.4)
    glVertex2f(0.0, 0.9)
    glEnd()
    glPopMatrix()  # Ağacın boyutlandırma işlemini geri al

def load_obj(filename):
    global vertices
    global faces 
    with open(filename, 'r') as file:
        for line in file:
            if line.startswith('v '):
                vertex = list(map(float, line.strip().split()[1:4]))
                vertices.append(vertex)
            elif line.startswith('f '):
                face = [int(vertex.split('/')[0]) - 1 for vertex in line.strip().split()[1:]]
                faces.append(face)
def drawBird():
    glScalef(-0.04, -0.04, -0.04)
    glRotatef(120, 180, 120, -40)
    glColor3f(0.0, 0.0, 1.0)
    glBegin(GL_TRIANGLES)
    for face in faces:
        for vertex_index in face:
            glVertex3fv(vertices[vertex_index])
    glEnd()

def BirdRandom():
    global translate_xBird
    temp = random.choice([0.1, -0.1])
    if(translate_xBird + temp > 0.8):
        temp = 0
    elif (translate_xBird +temp < -0.8):
        temp = 0
    translate_xBird += temp

#Arabanın kendisini çizmek için fonksiyon
def plotpoints():
    global translate_x

    glClearColor(1, 1, 1, 1)
    glClear(GL_COLOR_BUFFER_BIT)

    glLoadIdentity()
    # Araba ekranın dışına çıkmışsa, arabayı geri getir
    if(translate_x > 0.0):
        translate_x = 0
    elif (translate_x < -0.8):
        translate_x = -0.8
    glTranslatef(translate_x, 0.0, 0.0)  # Modeli hareket ettir

    
    # Arka kaput için
    glViewport(0, 0, 250, 250)
    glColor3f(R, G, B)
    plotQuad()
    glColor3f(0, 0, 0)
    plotLines()

    # Ön kaput için
    glViewport(125, 0, 250, 250)
    glColor3f(R, G, B)
    plotQuad()
    glColor3f(0, 0, 0)
    plotLines()

    # Camlar için
    glViewport(63, 63, 250, 250)
    glColor4f(0.5, 1.0, 1.0, 1.0)
    plotQuad()
    glColor3f(0, 0, 0)
    plotLines()

    # Arka tekerlek için
    glViewport(25, -25, 250, 250)
    glColor3f(0, 0, 0)
    plotCircle()

    # Ön tekerlek için
    glViewport(220, -25, 250, 250)
    glColor3f(0, 0, 0)
    plotCircle()

    glLoadIdentity()
    glViewport(220, -25, 500, 500)
    glTranslatef(-0.3, -0.55, 0)  # Arabanın solundaki konuma yerleştir
    drawTree()

    glLoadIdentity()
    BirdRandom()
    glTranslatef(translate_xBird, 0.0, 0.0)  # Modeli hareket ettir

    glViewport(10, 200, 250, 250)
    glTranslatef(0.0, 0.0, 0.0)  # Kuşun çizim pozisyonu
    drawBird()

    glutSwapBuffers()
    
def mouse(button, state, x, y):
    global translate_x

    if button == GLUT_RIGHT_BUTTON:
        translate_x += 0.05
        print("sag",translate_x)

    elif button == GLUT_LEFT_BUTTON:
        translate_x -= 0.05
        print("sol",translate_x)

    glutPostRedisplay()

def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(500, 500)
    glutInitWindowPosition(300, 200)
    glutCreateWindow(b"Aysenur Odev")
    glutMouseFunc(mouse)
    glutDisplayFunc(plotpoints)
    glutIdleFunc(plotpoints)
    load_obj("bird.obj")
    glutMainLoop()

main()