from OpenGL.GLUT import *
from OpenGL.GL import *
import sys

def ciz():

    #glutWireTeapot(0.5)
    #glutWireCube(1.0);
    #glutWireCone(1, 1, 10, 10);
    #glutWireSphere(0.5, 10, 10);
    glutWireTorus(1, 1, 50, 50)

    glFlush()

glutInit(sys.argv)
glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
glutInitWindowSize(250, 250)
glutInitWindowPosition(100, 100)
glutCreateWindow(bytes("Merhaba PyOpenGL", "ascii"))
glutDisplayFunc(ciz)
glutMainLoop()
