from OpenGL.GL import *
from OpenGL.GLUT import *
import sys

def init(): #İstediğiniz ismi verebilirsiniz
    glClearColor(0.0, 0.0, 0.0, 1.0)

def plotpoints():#İstediğiniz ismi verebilirsiniz

    glClear(GL_COLOR_BUFFER_BIT)
    glColor3f(1.0, 0.0, 0.0)

    glPointSize(10.0)
    glBegin(GL_POINTS)
    glVertex2f(0.0, 0.0)
    glEnd()

    glFlush()

def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(500, 500)
    glutInitWindowPosition(50, 50)
    glutCreateWindow(b"Plot Points")
    glutDisplayFunc(plotpoints)
    init()
    glutMainLoop()

main()

