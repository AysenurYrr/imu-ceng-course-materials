from OpenGL.GL import *
from OpenGL.GLUT import *
import sys

# Bu örnekte 3D döndürme işlemi yapılıyor
rangle = 10.0

def draw():
    global rangle
    glClearColor(0.0, 0.0, 0.0, 0.0)
    glClear(GL_COLOR_BUFFER_BIT)
    glViewport(0, 0, 500, 500)
    glLoadIdentity()
    glMatrixMode(GL_MODELVIEW)

    glPushMatrix()
    glColor3f(0, 0, 1)  # Draw     BLUE
    glTranslatef(0, 0.5, 0)
    glRotatef(rangle, 1, 1, 1)
    glutWireTeapot(0.2)
    glPopMatrix()

    glPushMatrix()
    glColor3f(0, 1, 0)  # Draw   GREEN
    glTranslatef(0.5, 0, 0)
    glRotatef(rangle, 0, 1, 0)
    glutWireTeapot(0.1)
    glPopMatrix()

    rangle += 1
    print("açı deeğri: ",rangle)

    glutSwapBuffers()


def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
    glutInitWindowSize(500, 500)
    glutInitWindowPosition(10, 10)
    glutCreateWindow(b"3D teapot Sample")
    glutDisplayFunc(draw)
    glutIdleFunc(draw)
    glutMainLoop()

main()
