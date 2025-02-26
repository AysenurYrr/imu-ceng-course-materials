from OpenGL.GL import *
from OpenGL.GLUT import *
import sys

#Bu örnekte 3D döndürme işlemi yapılıyor

aci=50
def draw():
    global aci
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClear(GL_COLOR_BUFFER_BIT)
    glViewport(0, 0, 500, 500)
    glLoadIdentity()
    glMatrixMode(GL_MODELVIEW)
    glColor3f(0, 0, 1)  # Draw     BLUE
    glRotatef(aci, 0, 0, 1)
    glutWireTeapot(0.5)
    glFlush()

    aci+=1


def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(500, 500)
    glutInitWindowPosition(10, 10)
    glutCreateWindow(b"3D teapot Sample")
    glutDisplayFunc(draw)
    glutMainLoop()


main()
