import pygame
import numpy as np
from math import *
import sys


WHITE = (255, 255, 255)
ORANGE = (255, 165, 0)
RED = (255, 0, 0)
BLACK = (0, 0, 0)

WIDTH, HEIGHT = 800, 600
pygame.display.set_caption("3D projection in pygame!")
screen = pygame.display.set_mode((WIDTH, HEIGHT))

scale = 100

circle_pos = [WIDTH/2, HEIGHT/2]  # x, y

angle = 0

points = []
faces  = []

# all the cube vertices

if len(sys.argv) > 1:
  file_name = sys.argv[1]
else :
  print("Please supply a file name!")
  sys.exit()

with open(file_name, 'r') as file:
    for line in file:
        if line.startswith('v '):
            vertex = np.array(list(map(float, line.strip().split()[1:])))
            points.append(vertex)
        elif line.startswith('f '):  # Yüzey (face) tanımları
            face_indices = [int(x.split('/')[0]) - 1 for x in line.strip().split()[1:]]
            # -1 işlemi, OBJ dosyasındaki indekslerin sıfır tabanlı olduğundan dolayı yapılır
            faces.append(face_indices)

projection_matrix = np.matrix([
    [1, 0, 0],
    [0, 1, 0]
])


projected_points = [
    [n, n] for n in range(len(points))
]

def connect_edges(face_indices, projected_points):
    for i in range(len(face_indices)):
        j = (i + 1) % len(face_indices)  # Döngüyü kapatmak için son köşeden ilk köşeye geçiş
        point1_index = face_indices[i]
        point2_index = face_indices[j]
        point1 = projected_points[point1_index]
        point2 = projected_points[point2_index]
        pygame.draw.line(screen, ORANGE, point1, point2)

def draw_faces(faces, projected_points):
    for face_indices in faces:
        # Yüzeyin köşe indekslerini al
        vertices = [projected_points[i] for i in face_indices]

        # Yüzeyi çiz
        pygame.draw.polygon(screen, RED, vertices)

clock = pygame.time.Clock()
while True:

    clock.tick(10)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                pygame.quit()
                exit()
    # update stuff

    rotation_z = np.matrix([
        [cos(angle), -sin(angle), 0],
        [sin(angle), cos(angle), 0],
        [0, 0, 1],
    ])

    rotation_y = np.matrix([
        [cos(angle), 0, sin(angle)],
        [0, 1, 0],
        [-sin(angle), 0, cos(angle)],
    ])

    rotation_x = np.matrix([
        [1, 0, 0],
        [0, cos(angle), -sin(angle)],
        [0, sin(angle), cos(angle)],
    ])
    angle += 0.01

    screen.fill(WHITE)
    # drawining stuff

    for face_indices in faces:
        draw_faces([face_indices], projected_points)
        connect_edges(face_indices, projected_points)

    i = 0
    for point in points:
        rotated2d = np.dot(rotation_z, point.reshape((3, 1)))
        rotated2d = np.dot(rotation_y, rotated2d)
        rotated2d = np.dot(rotation_x, rotated2d)

        projected2d = np.dot(projection_matrix, rotated2d)

        x = int(projected2d[0][0] * scale) + circle_pos[0]
        y = int(projected2d[1][0] * scale) + circle_pos[1]

        projected_points[i] = [x, y]
        pygame.draw.circle(screen, BLACK, (x, y), 1)
        i += 1


    pygame.display.update()