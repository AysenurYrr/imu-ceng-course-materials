# PyFunc.py
# Plotting functions
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from random import randrange
import random as rn
import sys

points=[[0.0,0.0]]
color_list=[[0.0,0.0,0.3]]

def init():
    glClearColor(1.0, 1.0, 1.0, 1.0)
    gluOrtho2D(-5.0, 5.0, -5.0, 5.0)


def plotfunc():
    glClear(GL_COLOR_BUFFER_BIT)
    glColor3f(0.5, 0.2, 0.8)

    # X koordinat DÃ¼zlemi
    glBegin(GL_LINES)
    glVertex2f(-5.0, 0)
    glVertex2f(5.0, 0)
    glEnd()

    # Y koordinat DÃ¼zlemi
    glBegin(GL_LINES)
    glVertex2f(.0, -5.0)
    glVertex2f(.0, 5.0)
    glEnd()

    glPointSize(5.0)
    for i in range(len(points)):
        x=points[i][0]
        y=points[i][1]

        r=color_list[i][0] #RED
        g = color_list[i][1]  # GREEN
        b = color_list[i][2]  # BLUE

        glBegin(GL_POINTS)
        glColor3f(r,g,b)
        glVertex2f(x, y)
        glEnd()

    glFlush()


# The function called whenever a key is pressed. Note the use of Python tuples to pass in: (key, x, y)
def keyPressed(*args):
    print(args[0])
    #Escape code=\x1b
    if args[0] == b"\x1b" or args[0] == bytes('q', 'utf-8'):
        sys.exit()
    else:
        R = rn.random()
        G = rn.random()
        B = rn.random()

        new_x=float(randrange(-5, 5))
        new_y = float(randrange(-5, 5))
        points.append([new_x,new_y])
        color_list.append([R,G,B])

    glutPostRedisplay()

def main():
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowPosition(50, 50)
    glutInitWindowSize(400, 400)
    glutCreateWindow(b"Function Plotter")
    glutDisplayFunc(plotfunc)
    glutKeyboardFunc(keyPressed)
    init()
    glutMainLoop()

main()
# End of program
