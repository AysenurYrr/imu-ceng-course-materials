from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import sys

weight=500
height=500
zoom=1

def draw():
    glClearColor(1.0, 1.0, 1.0, 1.0)
    glClear(GL_COLOR_BUFFER_BIT)
    glViewport(0, 0, 500, 500)
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    glColor3f(0, 0, 1)  # Draw     BLUE
    glScale(zoom,zoom,zoom)
    glutWireTeapot(1)
    glFlush()

def MouseWheelFonskiyonum(*args):
    global zoom
    #print(args)
    print("zoom : ", zoom)

    if args[1]==-1:
        zoom-=0.05
        if zoom<=0:
            zoom=0.5
    elif args[1]==1:
        zoom +=0.05
    else:
        pass
    glutPostRedisplay()



def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(weight, height)
    glutInitWindowPosition(100, 100)
    glutCreateWindow(b"3D teapot Sample")
    glutDisplayFunc(draw)
    glutIdleFunc(draw)
    glutMouseWheelFunc(MouseWheelFonskiyonum)
    glutMainLoop()



main()
