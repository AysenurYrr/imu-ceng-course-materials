from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys


def init():
    glClearColor(0.0, 0.0, 0.0, 1.0)
    gluOrtho2D(-5.0, 5.0, -5.0, 5.0)


def plotTerrain():
    glClear(GL_COLOR_BUFFER_BIT)
    glColor3f(1.0, 0.0, 0.0)

    #glBegin(GL_LINE_STRIP)
    glBegin(GL_TRIANGLE_FAN)
    glVertex2f(0.0, -5.0)  # Merkez
    glVertex2f(-5.0, -5.0)  # sol alt köşe

    glVertex2f(-5.0, -3.0)  # A(-5,-3)
    glVertex2f(-3.0, 0.0)  # B
    glVertex2f(-1.0, 0.0)  # C

    glVertex2f(1.0, -3.0)  # D
    glVertex2f(3.0, 3.0)  # E
    glVertex2f(5.0, 3.0)  # F

    glVertex2f(5.0, -5.0)
    glVertex2f(-5.0, -5.0)
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
