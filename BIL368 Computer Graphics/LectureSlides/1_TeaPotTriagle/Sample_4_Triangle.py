from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys


def init():
    glClearColor(0.0, 0.0, 0.0, 1.0)


def plotpoints():
    glClear(GL_COLOR_BUFFER_BIT)
    glColor3f(1.0, 0.0, 0.0)


    glBegin(GL_LINES)
    glVertex2f(0.0, 0.0)
    glVertex2f(0.0, 0.5)
    glEnd()

    glBegin(GL_LINES)
    glVertex2f(0.0, 0.5)
    glVertex2f(0.5, 0.5)
    glEnd()

    glBegin(GL_LINES)
    glVertex2f(0.0, 0.0)
    glVertex2f(0.5, 0.5)
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
