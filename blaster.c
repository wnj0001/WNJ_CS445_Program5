/*********************************************************************

    Software Architecture Statement:

    This program is a space-shooter style game, where the player must 
fire a laser at the enemy invaders falling from the top of the canvas, 
before the enemies can reach the bottom of the canvas. First, the 
variables and objects are all initialized, and the OpenGL specific
functions are registered. When the main loop begins, the 
initial_draw() method is called, which calls the spawn_enemy() 
function and then calls the animate() function after a frame's worth 
of time has passed. In the spawn_enemy() function, the enemy's center
point is set to a random location along the top of the canvas, and 
then the function is set to be called again after a random interval of
time between 2.75 and 3.50 seconds. 
    
    In the animate() function, as long as the game is not in a game 
over state, the draw_all_objects() function is called, followed by 
update_enemy() and update_player(). The animate function is then 
registered to be called again after a single frame's worth of time has
passed. If the game is in a game_over state, then the draw_game_over() 
function, which displays the game over canvas. The draw_all_objects() 
function calls the draw_cube() function once to draw the player, and 
if the enemy is alive, it also calls the draw_cube function to draw 
the enemy. If the laser is currently being fired, then the laser will 
be drawn on the canvas. Finally, the draw_scoreboard() function is 
called to draw the score to the canvas, and glutSwapBuffers() is 
called to swap the drawing buffers and display the drawn objects to 
the canvas. 
    
    In the update_enemy() function, if the enemy is alive, the
enemy's is moved towards the bottom of the canvas by an increment 
determined using the total distance and time, and if the enemy is 
touching the bottom of the canvas while alive, the game is put into a 
game over state by toggling the is_game_over variable. In the 
update_player() function, the player moves toward the left or right of
the canvas if the corresponding key was pressed by the player.
    
    In the draw_cube() function, a glutWireCube primitive is generated
and translated using the center point of the input cube. In the 
draw_laser() function, a line is drawn from the center of the player, 
to the top of the canvas. 
    
    The handle_keys() method sets a tri-state movement flag for the 
player when 'H' or 'L' are pressed, and this flag is then used by the
update_player() function to move the player left or right. When the
spacebar is pressed, the activate_laser() function will be called.
The function activate_laser() toggles the flag for the laser to be 
drawn, checks to see if the laser hits the enemy, and then registers
the disable_laser() function to be called after 0.15 seconds. The
disable_laser() function turns off the flag for the laser to be drawn.
The handle_keys_up() function is used to turn off the movement flags, 
and stop the movement of the player, when the 'H' or 'L' keys are no
longer being pressed.
    
    If the test_hit function determines that the laser hit the enemy,
then the kill_enemy() function is called, and a point is given to the
player. The kill_enemy() function toggles the is_alive state of the
enemy, and keeps it from being drawn until a new enemy is spawned.


    New Lighting Features:

    The new lighting features are initialized in the light_init() 
method. This method first enables lighting in OpenGL, then sets the 
diffuse light values, specular light values, and the light position of 
GL_LIGHT0. Finally, GL_LIGHT0 is enabled, and local viewer mode is 
activated. Next, in the draw_all_objects() method, in addition to the 
previous functions, the materials for the enemy cubes, and player
cubes are set before each is drawn. The ambient and diffuse colors 
are set to red for enemy cubes, and black for player cubes, specular
is set to white on each, and shininess is enabled for them, so that
highlights appear.

    
    WNJ  04/2014

 ********************************************************************/



#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include "my_setup_3D.h"

//  Constants for use with the my_setup() function.
#define canvas_width 400
#define canvas_height 400
#define canvas_name "Blaster Game"

// Represents a point in 3-Dimensional space.
typedef struct {
    float x;
    float y;
    float z;
} Point;

// Represents a rgb-style color.
typedef struct {
    float red;
    float green;
    float blue;
} Color;

// Represents a 3-Dimensional cube object and its attributes.
//      Movement is tri-state:
//          0: no movement
//          1: movement to left
//          2: movement to right
typedef struct {
    Point center;
    float size;
    Color color;
    int is_alive;
    int movement;
} Cube;

// Pointer to Point object that represents the origin of the scene.
Point* origin;

// Float that represents the current location of the z-plane on which 
// the drawn objects' origins lie.
float z_plane;

// Float that represents the current frame rate of the animation
float frame_rate;

// Floats that represent the size of the player and enemy cubes
float player_size;
float enemy_size;

// Pointers to Point objects that represent the starting positions
// of the player and enemy ships
Point* player_start;
Point* enemy_start;

// Pointers to Color objects that represent the colors used in the 
// program.
//      - White for the background, 
//      - player_color for player lines,
//      - Red for the enemy ships.
Color* bg_color;
Color* player_color;
Color* enemy_color;

// Pointers to Cube objects representing the player and enemy ships.
Cube* player;
Cube* enemy;

// Integers used to calculate the spawn point for the enemy ships.
int enemy_min_x, enemy_max_x;
int enemy_spawn_x;

// Integers used to calculate the time interval at which the enemy
// ships spawn.
int enemy_min_time, enemy_max_time;
int enemy_spawn_time;

// Floats used to calculate the rate at which the enemy ships move
// down the glSwapBuffers. 
float enemy_total_dist;
float enemy_total_time;
float enemy_step_dist;

// Floats used to calculate the rate at which the player's ship 
// can move along the bottom of the canvas.
float player_total_dist;
float player_total_time;
float player_step_dist;

// A boolean integer used to determine if the laser is currently
// firing.
int is_laser_firing;

// An integer representing the player's total score.
int player_score;

// A boolean integer used to determine if the game has entered the
// game over state.
int is_game_over;



// ------------------------------------
// -------> Utility Functions <--------
// ------------------------------------

// Used as a constructor to initialize a new Point object.
Point* make_point(float x, float y, float z) {
    Point* point = (Point*)malloc(sizeof(Point));

    point->x = x;
    point->y = y;
    point->z = z;

    return point;
}

// Used as a constructor to initialize a new Color object.
Color* make_color(float red, float green, float blue) {
    Color* color = (Color*)malloc(sizeof(Color));

    color->red   = red;
    color->green = green;
    color->blue  = blue;

    return color;
}

// Used as a constructor to initialize a new Color object.
Cube* make_cube(Point* center, float size, Color* color) {
    Cube* cube   = malloc(sizeof(Cube));
    cube->center = *center;
    cube->size   = size;
    cube->color  = *color;
    cube->is_alive = 0;
    cube->movement = 0;
    return cube;
}

// Sets the enemy's center point to a random point along the top of 
// the canvas, and then calls itself again after a time interval 
// between 2.75 and 3.50 minutes. 
void spawn_enemy() {
    srand(time(NULL) * -time(NULL));
    enemy_spawn_x = (rand() % ((enemy_max_x+1) - enemy_min_x)) + enemy_min_x;
    enemy->center.y = enemy_start->y;
    enemy->center.x = enemy_spawn_x;

    srand(time(NULL));
    enemy_spawn_time = (rand() % (enemy_max_time+1 - enemy_min_time)) + enemy_min_time;
    enemy->is_alive = 1;
    glutTimerFunc(enemy_spawn_time, spawn_enemy, 1);
}

// Updates the enemy's center point, allowing it to move down the 
// canvas. Also checks if the enemy has reached the bottom of the
// canvas and triggers a game over if so.
void update_enemy() {
    if(enemy->is_alive) {
        enemy->center.y -= enemy_step_dist;
        if((enemy->center.y - (enemy->size / 2)) < (origin->y - (canvas_height / 2))) {
            is_game_over = 1;
        }
    }
}

// Updates the player's center point, allowing it to move along 
// the bottom of the canvas.
void update_player() {
    if (player->movement == 1 && (player->center.x >= (origin->x - (canvas_width / 2) + player->size))) {
        player->center.x -= player_step_dist;
    }
    else if (player->movement == 2 && (player->center.x <= (origin->x + (canvas_width / 2) - player->size))) {
        player->center.x += player_step_dist;
    }
}

// Adds a point to the player's score.
void add_point() {
    player_score++;
}

// Kills the enemy ship, keeping it from being drawn until
// another is drawn.
void kill_enemy() {
    enemy->is_alive = 0;
}

// Checks to see if the laser has been fired within the hitbox of the
// enemy ship. Kills the enemy and adds a point to the player's score
// if the hit is successful.
void test_hit() {
    if(player->center.x < enemy->center.x + (enemy->size / 2) &&
       player->center.x > enemy->center.x - (enemy->size / 2)) {
        kill_enemy();
        add_point();
    }
}

// Disables the laser from being drawn.
void disable_laser() {
    is_laser_firing = 0;
}

// Activates the drawing of the laser, initiates a check to see if the
// enemy ship has been hit, and disables drawing of the laser after
// 0.15 seconds.
void activate_laser() {
    if(is_laser_firing == 0) {
        is_laser_firing = 1;
        test_hit();
        glutTimerFunc(1000 * (0.15), disable_laser, 1);
    }
}



// ------------------------------------
// -------> Drawing Functions <--------
// ------------------------------------

// Draws a Cube object at the translated point stored in its 
// center field.
void draw_cube(Cube* cube) {
    glPushMatrix();
    glTranslatef(cube->center.x,
                 cube->center.y,
                 cube->center.z);
    // glColor3f(cube->color.red, cube->color.green, cube->color.blue);
    glutSolidCube(cube->size);
    glTranslatef((cube->center.x * -1),
                 (cube->center.y * -1),
                 (cube->center.z * -1));
    glPopMatrix();
}

// Draws a laser line at the translated point stored in the Cube 
// parameter's center field.
void draw_laser(Cube* cube) {
    glColor3f(cube->color.red, cube->color.green, cube->color.blue);
    glBegin(GL_LINES);
        glVertex3f(cube->center.x,
                   cube->center.y + (cube->size),
                   z_plane + 15);
        glVertex3f(cube->center.x, 
                   200.0,
                   z_plane + 15);
    glEnd();
}

// Draws the scoreboard onto the top right of the canvas.
void draw_scoreboard() {
    glColor3f(player_color->red ,player_color->green ,player_color->blue);
    glRasterPos3f(125.0, 180.0, z_plane + 15);
    char *string = "Score: ";
    char *c;
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    char* score[20];
    sprintf(score, "%d", player_score);
    for (c = score; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
}

// Lighting is enabled, diffuse, specular, and light position are set
// up, light0 is activated, and light model local viewer is turned on.
// CITATION:
// This method of activating light comes from the textbook on pages 
// 426-427.
void light_init() {
    glEnable(GL_LIGHTING);

    float light_value[]    = {1.0, 1.0, 1.0, 1.0};
    float light_position[] = {0.0, 0.0, 0.0, 1.0};

    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_value);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_value);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHT0);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

// Draws all of the objects onto the canvas. Materials are also set here.
// CITATION:
// This method of setting materials comes from the textbook on pages 
// 426-427.
void draw_all_objects() {
    float player_diffuse[] = { 0.0, 0.0, 0.0, 1.0 };
    float enemy_diffuse[] = { 0.9, 0.0, 0.0, 1.0 };
    float specular[] = { 1.0, 1.0, 1.0, 1.0 };
    float shine[] = { 50.0 };
    glClearColor(bg_color->red, bg_color->green, bg_color->blue, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_scoreboard();
    light_init();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, player_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
    draw_cube(player); 
    if(enemy->is_alive) {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, enemy_diffuse);
        draw_cube(enemy);
    }
    glColor3f(player->color.red, player->color.green, player->color.blue);
    if(is_laser_firing) {
        
        draw_laser(player);
    }
    glutSwapBuffers();
}

// Draws a game over message when the player has failed to kill the 
// enemy before it reached the bottom of the canvas.
void draw_game_over() {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(player_color->red ,player_color->green ,player_color->blue);
    glRasterPos3f(-80.0, 0.0, z_plane + 15);
    char *string = "Too Bad! You Lost...";
    char *c;
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    glutSwapBuffers();
}


// If the game is not currently in a game over state, then all the 
// objects will be drawn, centers of both the enemy and player will
// be updated, and then the function will be called again after a
// time interval equal to the current frame rate has passed.
void animate() {
    if(!is_game_over) {
        draw_all_objects();
        update_enemy();
        update_player();
        glutTimerFunc(1000 * frame_rate, animate, 1);
    }
    else {
        draw_game_over();
    }
}

// 
void initial_draw() {
    spawn_enemy();
    glutTimerFunc(1000 * frame_rate, animate, 1);
}



// ------------------------------------
// -----> User Input Functions <-------
// ------------------------------------

// Allows the game to be controlled using the keyboard. 
//      - The 'H' key moves the player left.
//      - The 'L' key moves the player right.
//      - The spacebar fires the laser.
//      - The 'Q' key quits the game.
void handle_keys(unsigned char c, GLint x, GLint y) {
    if(c == 'h' || c == 'h') {
        player->movement = 1;
    }
    else if(c == 'l' || c == 'L') {
        player->movement = 2;
    }
    else if(c == ' ') {
        activate_laser();
    }
    else if ((c == 'q') || (c == 'Q'))
    {
        exit(0);
    }
}

// Stops the player's movement when the 'H' or 'L' keys are no longer
// being pressed.
void handle_keys_up(unsigned char c, GLint x, GLint y) {
    if(c == 'h' || c == 'h') {
        player->movement = 0;
    }
    else if(c == 'l' || c == 'L') {
        player->movement = 0;
    }
}



// ------------------------------------
// --------> Main Functions <----------
// ------------------------------------

// Initializes the objects and variables that will be used.
void init() {
    origin = make_point(0.0, 0.0, 0.0);
    z_plane = -15.0;
    frame_rate = 1.0 / 30.0;

    player_size = 25.0;
    enemy_size  = 25.0;

    bg_color     = make_color(1.0, 1.0, 1.0);
    player_color = make_color(0.0, 0.0, 0.0);
    enemy_color  = make_color(0.9, 0.1, 0.1);

    player_start = make_point(origin->x,
                              origin->y - (canvas_height / 2) + (player_size / 2),
                              z_plane);
    enemy_start  = make_point(origin->x,
                              origin->y + (canvas_height / 2) + (enemy_size / 2),
                              z_plane);

    player = make_cube(player_start, 25.0, player_color);
    enemy  = make_cube(enemy_start, 25.0, enemy_color);

    enemy_min_x = origin->x - (canvas_width / 2.0) + enemy->size / 2.0;
    enemy_max_x = origin->x + (canvas_width / 2.0) - enemy->size / 2.0;

    enemy_spawn_x = 0.0;
    enemy_spawn_time = 0.0;

    // Minimum and Maximum times in Milliseconds.
    enemy_min_time = 2750.0;
    enemy_max_time = 3500.0;

    // enemy animation rate calculation
    enemy_total_dist = (canvas_height - enemy->size);
    enemy_total_time = 2.5;
    enemy_step_dist  = (enemy_total_dist / enemy_total_time) * frame_rate;

    // player animation rate calculation
    player_total_dist = (canvas_width - player->size);
    player_total_time = 0.75;
    player_step_dist  = (player_total_dist / player_total_time) * frame_rate;

    player_score = 0;

    is_laser_firing = 0;

    is_game_over = 0;

}



int main(int argc, char** argv) {
    init();
    glutInit(&argc, argv);
    my_setup(canvas_width, canvas_height, canvas_name);
    glutDisplayFunc(initial_draw);
    glutKeyboardFunc(handle_keys);
    glutKeyboardUpFunc(handle_keys_up);
    glutMainLoop();
    return 0;
}