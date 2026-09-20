#ifndef GHOST_H
#define GHOST_H

#include <stdbool.h>
#include "raylib.h"

#define ghost_map_x 586
#define ghost_map_y 72
#define ghost_size 24

#define wtiles 28
#define htiles 31
#define ghost_normal_speed 110.0f
#define ghost_eaten_speed 150.0f
#define ghost_frightended_speed 70.0f

extern char map[htiles][wtiles+1];
typedef enum
{
    blinky,
    pinky,
    inky,
    clyde
}ghost_name;

typedef struct{
    float chase;
    float eaten;
    float frightened;
    float scatter;

}mode_time;

typedef enum{
    up,
    down,
    right,
    left
}direction;


typedef struct 
{
    Vector2 position;
    ghost_name name;
    Color color;
    int scatter_row;
    int scatter_col;
    direction dir;
    bool eaten;
    bool ignore_frightened;

}ghost;

void init_phase(ghost *g, ghost_name name);

typedef struct
{
    float x;
    float y;
    float width;
    float height;

} ghost_rec;



typedef enum{
    eaten,
    scattered,
    frightened,
    chase
}ghost_mode;

typedef struct{
    int row;
    int col;
}tile;



bool wall(tile t);
tile tiles_no(Vector2 position);
char check_distance(ghost *g, bool wall);
void blinky_chase_alg(ghost *g,Vector2 pacpos);
void blinky_scatter_alg(ghost *g);
void movement(ghost *g,float speed);
void ghost_frightened(ghost *g);
void flip_dir(ghost *g);
void eaten_phase_blinky(ghost *g);
void pinky_scatter_alg(ghost *g);
void pinky_chase_alg(ghost *g,Vector2 pacpos,Vector2 speed);
void eaten_phase_pinky(ghost *g);
void inky_chase_alg(ghost *g,ghost *g1,Vector2 pacpos, Vector2 speed);
void inky_scatter_alg(ghost *g);
void eaten_phase_inky(ghost *g);
void clyde_chase_alg(ghost *g,Vector2 pacpos);
void eaten_phase_clyde(ghost *g);
void clyde_scatter_alg(ghost *g);
void check_eaten_reset(ghost *g, tile home);

Rectangle g_rec(ghost *g,ghost_name name);
Vector2 pixel(tile t);



#endif