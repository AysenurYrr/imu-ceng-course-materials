from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys


def init():
    glClearColor(0.5, .5, 0.0, 1.0)
    gluOrtho2D(-6.0, 6.0, -6.0, 6.0)


def plotTerrain():
    glClear(GL_COLOR_BUFFER_BIT)
    glColor3f(1.0, 0.0, 0.0)

    glPointSize(3.0)

    #glBegin(GL_LINE_LOOP)
    glBegin(GL_POLYGON)
    glVertex2f(-5.0, -3.0)  # A(-5,-3)
    glVertex2f(-3.0, 0.0)  # B
    glVertex2f(-1.0, 0.0)  # C

    glVertex2f(1.0, -3.0)  # D
    glVertex2f(3.0, 3.0)  # E
    glVertex2f(5.0, 3.0)  # F

    glVertex2f(5.0, -5.0) # G
    glVertex2f(-5.0, -5.0) # H

    glVertex2f(-5.0, -3.0)  # A(-5,-3)
    glEnd()

    glFlush()


def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(500, 500)
    glutInitWindowPosition(50, 50)
    glutCreateWindow(b"Plot Points")
    glutDisplayFunc(plotTerrain)
    init()
    glutMainLoop()


main()
