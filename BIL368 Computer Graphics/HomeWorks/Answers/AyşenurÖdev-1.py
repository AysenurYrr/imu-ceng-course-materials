from OpenGL.GL import *
from OpenGL.GLUT import *

# Global değişkenler
translate_x = 0.0
translate_y = 0.0
translate_z = 0.0
rotate_angle = 0.0

def load_model(filename):
    """Model dosyasını oku ve yüzeylerin indekslerini al"""
    vertices = []
    faces = []
    with open(filename, 'r') as file:
        for line in file:
            if line.startswith('v '):
                parts = line.split()
                x, y, z = map(float, parts[1:])
                vertices.append((x, y, z))
            elif line.startswith('f '):
                parts = line.split()
                face = [int(part.split('/')[0]) - 1 for part in parts[1:]]  # 1'e 1 indeksleme yapmak için -1
                faces.append(face)
    return vertices, faces

def draw_model(vertices, faces):
    """Modeli çiz"""
    glBegin(GL_TRIANGLES)
    for face in faces:
        for vertex_index in face:
            glVertex3f(*vertices[vertex_index])
    glEnd()

def display():
    """Ekranı güncelle ve modeli çiz"""
    glClear(GL_COLOR_BUFFER_BIT)
    glLoadIdentity()
    
    glTranslatef(translate_x, translate_y, translate_z)  # Modeli hareket ettir
    glRotatef(rotate_angle, 0, 1, 0)  # Modeli y ekseninde döndür
    
    vertices, faces = load_model("teapot.obj")  # Model dosyasını yükle
    
    glColor3f(1.0, 1.0, 1.0)  # Beyaz renk
    draw_model(vertices, faces)
    
    glFlush()

def special_keys(key, x, y):
    """Klavye girişini işle"""
    global translate_x, translate_y, translate_z, rotate_angle
    # Klavye tuşlarına göre hareket et ve döndür
    if key == GLUT_KEY_LEFT:
        translate_x -= 0.1
    elif key == GLUT_KEY_RIGHT:
        translate_x += 0.1
    elif key == GLUT_KEY_UP:
        translate_y += 0.1
    elif key == GLUT_KEY_DOWN:
        translate_y -= 0.1
    elif key == GLUT_KEY_PAGE_UP:
        rotate_angle += 5
    elif key == GLUT_KEY_PAGE_DOWN:
        rotate_angle -= 5
    # Ekranı yeniden çiz
    glutPostRedisplay()

def main():
    glutInit()
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(800, 600)
    glutCreateWindow(b"Model")
    glClearColor(0.0, 0.0, 0.0, 1.0)  # Siyah arka plan
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(-3.0, 3.0, -3.0, 3.0, -1.0, 1.0)  # Koordinat düzlemi
    glMatrixMode(GL_MODELVIEW)  # Model görüntüleme matrisi modu
    glLoadIdentity()
    glTranslatef(0.0, 0.0, -5.0)  # Kamerayı geriye doğru kaydır
    
    glutDisplayFunc(display)
    glutSpecialFunc(special_keys)  # Klavye olayları için callback fonksiyonu
    glutMainLoop()

if __name__ == "__main__":
    main()
