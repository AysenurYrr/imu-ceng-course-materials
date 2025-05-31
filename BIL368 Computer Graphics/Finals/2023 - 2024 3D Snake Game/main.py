import sys
import random
import pygame
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from PIL import Image  # PIL kütüphanesini ekledik

# Global variables
window_width, window_height = 1000, 1000
map_size = 20
cell_size = 1
snake = [(5, 5)]
snake_dir = (1, 0)
game_over = False
angle = -20
speed = 130
button_pos = (-0.32, -0.5, 0.4, -0.3)
main_window = None
game_over_window = None
score =0


# Apple positions
red_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
bomb_apples = [(random.randint(0, map_size-1), random.randint(0, map_size-1)) for _ in range(4)]
diamond_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
stone_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
gold_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))

# Apple textures
snake_tail_texture=None
snake_head_texture=None
red_texture = None
bomb_texture = None
diamond_texture = None
stone_texture = None
gold_texture = None

# Apple timers
diamond_apple_timeout = 6000
stone_apple_timeout = 6000
bomb_apple_timeout= 10000

# Load PNG file as texture
def load_texture(filename):
    try:
        image = Image.open(filename)
        flipped_image = image.transpose(Image.FLIP_TOP_BOTTOM)  # Yüklenen görüntüyü ters çevir
        ix = flipped_image.size[0]
        iy = flipped_image.size[1]
        image = flipped_image.tobytes("raw", "RGB")
        glClearColor(0.0, 0.0, 0.0, 0.0)	

        texture_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, texture_id)
        #glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ix, iy, 0, GL_RGB, GL_UNSIGNED_BYTE, image)
        glEnable(GL_TEXTURE_2D)
        return texture_id
    except IOError as e:
        print(f"Error loading texture: {e}")
        return -1

def load_textures():
    global red_texture, bomb_texture, diamond_texture, stone_texture, gold_texture,snake_head_texture,snake_tail_texture
    red_texture = load_texture("apple.png")
    bomb_texture = load_texture("bomb.jpg")
    diamond_texture = load_texture("diamond_apple.png")
    stone_texture = load_texture("stone_apple.png")
    gold_texture = load_texture("gold_apple.jpg")
    snake_head_texture = load_texture("snake1.jpeg")
    snake_tail_texture = load_texture("snake.JPG")

    # Eğer texture'lar yüklenemediyse hata mesajı yazdır
    if red_texture == -1 or bomb_texture == -1 or diamond_texture == -1 or stone_texture == -1 or gold_texture == -1:
        print("Texture loading failed. Check if the files exist and are in the correct format.")
        
def load_sounds():
    global eat_sound, bomb_sound, stone_sound,game_over_sound
    pygame.mixer.init()
    eat_sound = pygame.mixer.Sound("eat.wav")
    bomb_sound = pygame.mixer.Sound("bomb_sound.wav")
    stone_sound = pygame.mixer.Sound("stone_sound.wav")
    game_over_sound=pygame.mixer.Sound("game_over_sound.wav")
    


# Initialize OpenGL
def init():
    
    glEnable(GL_DEPTH_TEST)
    glEnable(GL_LIGHTING)  # Işıklandırmayı etkinleştir
    glEnable(GL_LIGHT0)    # Bir ışık kaynağı ekleyin
    glEnable(GL_COLOR_MATERIAL)  # Nesnelerin renklerini ışıklandırmaya göre ayarlayın
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE)
    glShadeModel(GL_SMOOTH)
    gluPerspective(55, window_width / window_height, 1, 100)
    glTranslatef(-map_size / 2, -map_size / 2, -map_size * 1.09)
    
# Draw the game grid
def draw_grid():
    glColor3f(1.0, 1.0, 1.0)
    glBegin(GL_LINES)
    for x in range(map_size + 1):
        glVertex3f(x, 0, 0)
        glVertex3f(x, map_size, 0)
    for y in range(map_size + 1):
        glVertex3f(0, y, 0)
        glVertex3f(map_size, y, 0)
    glEnd()

# Draw a cube at a given position
def draw_cube(position, texture_id):
    x, y = position
    glBindTexture(GL_TEXTURE_2D, texture_id)

    half_depth = -0.5  # Küpün yarı derinliği

    # Front face
    glBegin(GL_QUADS)
    glTexCoord2f(0, 0)
    glVertex3f(x, y, -half_depth)
    glTexCoord2f(1, 0)
    glVertex3f(x + 1, y, -half_depth)
    glTexCoord2f(1, 1)
    glVertex3f(x + 1, y + 1, -half_depth)
    glTexCoord2f(0, 1)
    glVertex3f(x, y + 1, -half_depth)
    glEnd()

    # Back face
    glBegin(GL_QUADS)
    glTexCoord2f(0, 0)
    glVertex3f(x, y, half_depth)
    glTexCoord2f(1, 0)
    glVertex3f(x + 1, y, half_depth)
    glTexCoord2f(1, 1)
    glVertex3f(x + 1, y + 1, half_depth)
    glTexCoord2f(0, 1)
    glVertex3f(x, y + 1, half_depth)
    glEnd()

    # Top face
    glBegin(GL_QUADS)
    glTexCoord2f(0, 0)
    glVertex3f(x, y + 1, -half_depth)
    glTexCoord2f(1, 0)
    glVertex3f(x + 1, y + 1, -half_depth)
    glTexCoord2f(1, 1)
    glVertex3f(x + 1, y + 1, half_depth)
    glTexCoord2f(0, 1)
    glVertex3f(x, y + 1, half_depth)
    glEnd()

    # Bottom face
    glBegin(GL_QUADS)
    glTexCoord2f(0, 0)
    glVertex3f(x, y, -half_depth)
    glTexCoord2f(1, 0)
    glVertex3f(x, y, half_depth)
    glTexCoord2f(1, 1)
    glVertex3f(x + 1, y, half_depth)
    glTexCoord2f(0, 1)
    glVertex3f(x + 1, y, -half_depth)
    glEnd()

    # Right face
    glBegin(GL_QUADS)
    glTexCoord2f(0, 0)
    glVertex3f(x + 1, y, -half_depth)
    glTexCoord2f(1, 0)
    glVertex3f(x + 1, y, half_depth)
    glTexCoord2f(1, 1)
    glVertex3f(x + 1, y + 1, half_depth)
    glTexCoord2f(0, 1)
    glVertex3f(x + 1, y + 1, -half_depth)
    glEnd()

    # Left face
    glBegin(GL_QUADS)
    glTexCoord2f(0, 0)
    glVertex3f(x, y, -half_depth)
    glTexCoord2f(1, 0)
    glVertex3f(x, y + 1, -half_depth)
    glTexCoord2f(1, 1)
    glVertex3f(x, y + 1, half_depth)
    glTexCoord2f(0, 1)
    glVertex3f(x, y, half_depth)
    glEnd()

# Draw the snake
def draw_snake_tail():
    glColor3f(0.0, 0.5, 0.0)  # Yeşil renk
    for segment in snake[:-1]:  # Baş hariç her segmenti yeşil olarak çiz
        draw_cube(segment, snake_tail_texture)
        

def draw_snake():
    glColor3f(1.0, 1.0, 1.0)
    draw_cube(snake[-1], snake_head_texture)  # Baş kısmı texture ile çiz
    draw_snake_tail()  # Kuyruk kısmını yeşil renkte çiz

# Draw the apples
def draw_red_apple():
    glColor3f(1.0, 1.0, 1.0)
    glBindTexture(GL_TEXTURE_2D, red_texture)
    draw_cube(red_apple, red_texture)
    
def draw_bomb_apples():
    glColor3f(1.0, 1.0, 1.0)
    for bomb_apple in bomb_apples:
        glBindTexture(GL_TEXTURE_2D, bomb_texture)
        draw_cube(bomb_apple, bomb_texture)

def draw_diamond_apple():
    glColor3f(1.0, 1.0, 1.0)
    glBindTexture(GL_TEXTURE_2D, diamond_texture)
    draw_cube(diamond_apple, diamond_texture)

def draw_stone_apple():
    glColor3f(1.0, 1.0, 1.0)
    glBindTexture(GL_TEXTURE_2D, stone_texture)
    draw_cube(stone_apple, stone_texture)

def draw_gold_apple():
    glColor3f(1.0, 1.0, 1.0)
    glBindTexture(GL_TEXTURE_2D, gold_texture)
    draw_cube(gold_apple, gold_texture)


def setup_snake_lights():
    # Baş ışığını tanımla ve ayarla
    glEnable(GL_LIGHT1)
    glLightfv(GL_LIGHT1, GL_DIFFUSE, [3.0, 3.0, 3.0, 1.0])  # Beyaz renk
    glLightfv(GL_LIGHT1, GL_POSITION, [snake[-1][0], snake[-1][1], 1, 1])  # Başın konumu

    # Kuyruk ışığını tanımla ve ayarla
    glEnable(GL_LIGHT2)
    glLightfv(GL_LIGHT2, GL_DIFFUSE, [1.0, 1.0, 1.0, 1.0])  # Beyaz renk
    glLightfv(GL_LIGHT2, GL_POSITION, [snake[0][0], snake[0][1], 1, 1])  # Kuyruğun konumu
    



    
# Display callback
def display():
    global angle
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glPushMatrix()
    glRotatef(angle, 9, 0, 0)
    # Bir ışık kaynağı yerleştirin
    #light_position = [10.0, 10.0, 10.0, 1.0]  # X, Y, Z koordinatları ve W bileşeni (1.0 sabit)
    #glLightfv(GL_LIGHT0, GL_POSITION, light_position)
    
    draw_grid()
    draw_snake()
    draw_red_apple()
    draw_bomb_apples()
    draw_diamond_apple()
    draw_stone_apple()
    draw_gold_apple()
    glPopMatrix()
    glutSwapBuffers()
       
    setup_snake_lights()  # Baş ve kuyruk ışıklarını ayarla
    

# Timer callback
def timer(value):
    global game_over
    if not game_over:
        move_snake()
        check_collision()
        update_apple_timers()
        glutPostRedisplay()
        glutTimerFunc(speed, timer, 0)

def update_apple_timers():
    global diamond_apple_timeout, stone_apple_timeout,bomb_apple_timeout
    diamond_apple_timeout -= speed
    stone_apple_timeout -= speed
    bomb_apple_timeout -=speed

    if diamond_apple_timeout <= 0:
        place_diamond_apple()
        diamond_apple_timeout = 6000

    if stone_apple_timeout <= 0:
        place_stone_apple()
        stone_apple_timeout = 6000
        
    if bomb_apple_timeout <= 0:
        place_bomb_apples()
        bomb_apple_timeout = 10000

def check_collision():
    global game_over, speed,score
    head = snake[-1]
    if  speed > 40 :
        katsayi = (len(snake)-1) / 5
        speed = 130 - int(katsayi)*10

    # Harita sınırlarının kontrolü ve yılanın kendisine çarpması durumu
    if not (0 <= head[0] < map_size and 0 <= head[1] < map_size) or head in snake[:-1]:
        game_over = True
        show_game_over_window()
        return

    if head in bomb_apples:
        bomb_sound.play()  # Bomb apple çarptığında patlama sesi oynat
        game_over = True
        show_game_over_window()
        return

    if head == red_apple:
        eat_sound.play()  # Yeme sesi
        snake.append(snake[-1])
        score  = score + 1   
        place_red_apple()

    elif head == diamond_apple:
        eat_sound.play()  # Yeme sesi
        snake.append(snake[-1])
        score  = score + 1
        snake.append(snake[-1])
        score  = score + 1
        snake.append(snake[-1])
        score  = score + 1
        place_diamond_apple()

    elif head == stone_apple:
        stone_sound.play()  # Çarpma sesi
        if len(snake) == 1:
            game_over = True
            show_game_over_window()
            return
        else:
            snake.pop()
            score  = score - 1
        place_stone_apple()

    elif head == gold_apple:
        eat_sound.play()  # Yeme sesi
        snake.append(snake[-1])
        score  = score + 1
        snake.append(snake[-1])
        score  = score + 1
        place_gold_apple()

def special_input(key, x, y):
    global snake_dir
    if key == GLUT_KEY_UP and snake_dir != (0, -1):
        snake_dir = (0, 1)
    elif key == GLUT_KEY_DOWN and snake_dir != (0, 1):
        snake_dir = (0, -1)
    elif key == GLUT_KEY_LEFT and snake_dir != (1, 0):
        snake_dir = (-1, 0)
    elif key == GLUT_KEY_RIGHT and snake_dir != (-1, 0):
        snake_dir = (1, 0)

def keyboard(key, x, y):
    global snake_dir, angle,speed
    if key == b'w' and snake_dir != (0, -1):
        snake_dir = (0, 1)
    elif key == b's' and snake_dir != (0, 1):
        snake_dir = (0, -1)
    elif key == b'a' and snake_dir != (1, 0):
        snake_dir = (-1, 0)
    elif key == b'd' and snake_dir != (-1, 0):
        snake_dir = (1, 0)
    elif key == b'z':
        angle = (angle + 5) % 360
    elif key == b'x':
        angle = (angle - 5) % 360
    elif key == b' ':
        speed = speed - 5
    elif key == b'n':
        speed = speed + 5


def move_snake():
    global snake, game_over
    new_head = (snake[-1][0] + snake_dir[0], snake[-1][1] + snake_dir[1])
    
    if new_head[0] >= map_size:
        new_head = (0, new_head[1])
    elif new_head[0] < 0:
        new_head = (map_size - 1, new_head[1])
    elif new_head[1] >= map_size:
        new_head = (new_head[0], 0)
    elif new_head[1] < 0:
        new_head = (new_head[0], map_size - 1)

    if new_head in snake or not (0 <= new_head[0] < map_size and 0 <= new_head[1] < map_size):
        game_over = True
        show_game_over_window()
        return

    snake.append(new_head)
    snake.pop(0)

def place_red_apple():
    global red_apple
    while True:
        new_red_apple = (random.randint(0, map_size - 1), random.randint(0, map_size - 1))
        # Önceki kırmızı elma pozisyonları ile kıyaslayın
        valid_position = True
        for apple in bomb_apples + [diamond_apple, stone_apple, gold_apple]:
            if abs(new_red_apple[0] - apple[0]) < 4 and abs(new_red_apple[1] - apple[1]) < 4:
                valid_position = False
                break
        # Bomba elmaları ile çakışmayı kontrol edin
        if new_red_apple in bomb_apples:
            valid_position = False
        # Yılan ile çakışmayı kontrol edin
        if new_red_apple in snake:
            valid_position = False
        # Eğer geçerli bir konum bulursak, kırmızı elmayı yerleştirin
        if valid_position:
            red_apple = new_red_apple
            break

def place_bomb_apples():
    global bomb_apples
    bomb_apples = []
    while len(bomb_apples) < 4:
        new_bomb_apple = (random.randint(0, map_size - 1), random.randint(0, map_size - 1))
        # Önceki bomba elma pozisyonları ile kıyaslayın
        valid_position = True
        for apple in bomb_apples:
            if abs(new_bomb_apple[0] - apple[0]) < 4 and abs(new_bomb_apple[1] - apple[1]) < 4:
                valid_position = False
                break
        # Kırmızı elma ile çakışmayı kontrol edin
        if new_bomb_apple == red_apple:
            valid_position = False
        # Yılan ile çakışmayı kontrol edin
        if new_bomb_apple in snake:
            valid_position = False
        # Eğer geçerli bir konum bulursak, bomba elmalar listesine ekleyin
        if valid_position:
            bomb_apples.append(new_bomb_apple)

def place_diamond_apple():
    global diamond_apple
    while True:
        new_diamond_apple = (random.randint(0, map_size - 1), random.randint(0, map_size - 1))
        # Önceki elma pozisyonları ile kıyaslayın
        valid_position = True
        for apple in bomb_apples + [red_apple, stone_apple, gold_apple]:
            if abs(new_diamond_apple[0] - apple[0]) < 4 and abs(new_diamond_apple[1] - apple[1]) < 4:
                valid_position = False
                break
        # Bomba elmaları ile çakışmayı kontrol edin
        if new_diamond_apple in bomb_apples:
            valid_position = False
        # Yılan ile çakışmayı kontrol edin
        if new_diamond_apple in snake:
            valid_position = False
        # Eğer geçerli bir konum bulursak, elmayı yerleştirin
        if valid_position:
            diamond_apple = new_diamond_apple
            break
        
def place_stone_apple():
    global stone_apple
    while True:
        new_stone_apple = (random.randint(0, map_size - 1), random.randint(0, map_size - 1))
        # Önceki elma pozisyonları ile kıyaslayın
        valid_position = True
        for apple in bomb_apples + [red_apple, diamond_apple, gold_apple]:
            if abs(new_stone_apple[0] - apple[0]) < 4 and abs(new_stone_apple[1] - apple[1]) < 4:
                valid_position = False
                break
        # Bomba elmaları ile çakışmayı kontrol edin
        if new_stone_apple in bomb_apples:
            valid_position = False
        # Yılan ile çakışmayı kontrol edin
        if new_stone_apple in snake:
            valid_position = False
        # Eğer geçerli bir konum bulursak, elmayı yerleştirin
        if valid_position:
            stone_apple = new_stone_apple
            break

def place_gold_apple():
    global gold_apple
    while True:
        new_gold_apple = (random.randint(0, map_size - 1), random.randint(0, map_size - 1))
        # Önceki elma pozisyonları ile kıyaslayın
        valid_position = True
        for apple in bomb_apples + [red_apple, diamond_apple, stone_apple]:
            if abs(new_gold_apple[0] - apple[0]) < 4 and abs(new_gold_apple[1] - apple[1]) < 4:
                valid_position = False
                break
        # Bomba elmaları ile çakışmayı kontrol edin
        if new_gold_apple in bomb_apples:
            valid_position = False
        # Yılan ile çakışmayı kontrol edin
        if new_gold_apple in snake:
            valid_position = False
        # Eğer geçerli bir konum bulursak, elmayı yerleştirin
        if valid_position:
            gold_apple = new_gold_apple
            break

def show_game_over_window():
    global game_over_window,speed
    speed = 130
    if game_over_window is not None:
        glutDestroyWindow(game_over_window)
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
    glutInitWindowSize(400, 300)
    game_over_window = glutCreateWindow(b'Game Over')
    glutDisplayFunc(display_game_over)
    game_over_sound.play()
    glutMouseFunc(mouse_click)
    glutMainLoop()
    
def draw_play_button_text():
    glColor3f(0, 0, 0)
    glRasterPos2f(-0.27, -0.45)
    for char in b"TEKRAR OYNA":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, char)

def display_game_over():
    global score
    glClear(GL_COLOR_BUFFER_BIT)
    glColor3f(1, 1, 1)
    glRasterPos2f(-0.23, 0.2)
    for char in b"GAME OVER":
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, char)
    glColor3f(1, 1, 1)
    score_text = f"SCORE: {score}"
    glRasterPos2f(-0.15, 0.0)
    for char in score_text.encode():
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, char)
        
    glColor3f(1, 1, 1)
    score_text = f"TULIN BABALIK KOPMAZ - 20120101376"
    glRasterPos2f(-0.85, -0.7)
    for char in score_text.encode():
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, char)
        
    glColor3f(1, 1, 1)
    score_text = f"AYSENUR YORUR - 22120205384"
    glRasterPos2f(-0.85, -0.84)
    for char in score_text.encode():
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, char)
        
    glColor3f(1, 1, 1)
    score_text = f"KAAN OSMANOGLU - 21120205705"
    glRasterPos2f(-0.85, -0.97)
    for char in score_text.encode():
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, char)
    
    glColor3f(1, 1, 1)
    glBegin(GL_QUADS)
    glVertex2f(button_pos[0], button_pos[1])
    glVertex2f(button_pos[2], button_pos[1])
    glVertex2f(button_pos[2], button_pos[3])
    glVertex2f(button_pos[0], button_pos[3])
    glEnd()
    
    draw_play_button_text()
    
    glFlush()

def mouse_click(button, state, x, y):
    global score
    if button == GLUT_LEFT_BUTTON and state == GLUT_DOWN:
        ogl_x = (x / 400.0) * 2 - 1
        ogl_y = -((y / 300.0) * 2 - 1)
        score = 0
        if button_pos[0] <= ogl_x <= button_pos[2] and button_pos[1] <= ogl_y <= button_pos[3]:
            restart_game()

def restart_game():
    global snake, snake_dir, game_over, red_apple, diamond_apple, stone_apple, gold_apple, main_window, game_over_window,score
    snake = [(5, 5)]
    snake_dir = (1, 0)
    game_over = False
    score=0
    red_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
    place_bomb_apples()
    diamond_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
    stone_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
    gold_apple = (random.randint(0, map_size-1), random.randint(0, map_size-1))
    if main_window is not None:
        glutDestroyWindow(main_window)
    if game_over_window is not None:
        glutDestroyWindow(game_over_window)
    main()

def main():
    global main_window
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
    glutInitWindowSize(window_width, window_height)
    main_window = glutCreateWindow(b'Snake Game 3D')
    load_textures()  # Texture'ları yükle
    load_sounds()    # Ses dosyalarını yükle
    init()
    glutDisplayFunc(display)
    glutTimerFunc(speed, timer, 0)
    glutSpecialFunc(special_input)
    glutKeyboardFunc(keyboard)
    
    glutMainLoop()

if __name__ == '__main__':
    main()

