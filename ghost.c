#include "ghost.h"
#include "raylib.h"
#include "collision.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include "raymath.h"





float dt=1.0f/60;




void init_phase(ghost *g,ghost_name name){
    g->name=name;
    g->ignore_frightened = false;
    switch (name)
    {
    case blinky:
        g->color=RED;
        g->scatter_row=1;
        g->scatter_col=26;
        g->position = pixel((tile){ 11, 13 });
        g->dir=up;
        g->eaten = false;
        break;
    case inky:
        g->color=SKYBLUE;
        g->scatter_row=29;
        g->scatter_col=26;
        g->position=pixel((tile){11,15});
        g->dir=up;
        g->eaten = false;
        break;
    case pinky:
        g->color=PINK;
        g->scatter_col=1;
        g->scatter_row=1;
        g->position=pixel((tile){11,12});
        g->dir=up;
        g->eaten = false;
        break;
    case clyde:
        g->color=ORANGE;
        g->scatter_col=1;
        g->scatter_row=29;
        g->position=pixel((tile){11,14});
        g->dir=up;
        g->eaten = false;
        break;
    
    
    }
    

}
//ghost_rec g={ghost_normal_speed * dt,ghost_normal_speed * dt,24,24};

bool wall(tile t){
    if (t.row == 14 && (t.col < 0 || t.col >= wtiles)) {
        return false;   
    }

    if (t.row < 0 || t.row >= htiles || t.col < 0 || t.col >= wtiles) {
        return true;    
    }
    
    if(map[t.row][t.col] == 'w' || map[t.row][t.col] == 's' ||map[t.row][t.col] == 'p' || map[t.row][t.col] == 'a' || map[t.row][t.col] == 't' || map[t.row][t.col] == 'v'||
        map[t.row][t.col] == 'g' || map[t.row][t.col] == 'r' || map[t.row][t.col] == 'u'|| map[t.row][t.col] == 'f' || map[t.row][t.col] == 'l' || map[t.row][t.col] == 'c' || map[t.row][t.col] == 'i'){
        return true;
    }
    else{
        return false;
    }
        
    
}

tile tiles_no(Vector2 position){
    tile t;
    t.col=(position.x-ghost_map_x)/26;
    t.row=(position.y-ghost_map_y)/26;
    return t;
}

Vector2 pixel(tile t){
    Vector2 p;
    p.x=ghost_map_x+26*t.col+1;
    p.y=ghost_map_y+26*t.row+1;
    return p;
}


void blinky_chase_alg(ghost *g,Vector2 pacpos){
    tile t=tiles_no(g->position);
    bool w = wall(t);
    
    float check1,check2,check3,check4; 
        
      if(g->dir == up){
        t.row-=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check1=Vector2Distance(pix, pacpos);
                
            }
            else{
                check1=1000000.0f;
            }
            t.row+=1;
            
            
            t.col+=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check3=Vector2Distance(pix, pacpos);
                
            }
            else{
                check3=1000000.0f;
            }
            t.col-=1;
            t.col-=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check4=Vector2Distance(pix, pacpos);
                
            }
            else{
                check4=1000000.0f;
            }
            t.col+=1;
            if(check1<check3 && check1<check4){
                g->dir=up;
                
            }
            
            else if(check3<check1 && check3<check4){
                g->dir=right;
            }
            else{
                g->dir=left;
            }
        }
        else if(g->dir == down){
            
            
            t.row+=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check2=Vector2Distance(pix, pacpos);
                
                
            }
            else{
                check2=10000.0f;
            }
            t.row-=1;
            t.col+=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check3=Vector2Distance(pix, pacpos);
                
            }
            else{
                check3=10000.0f;
            }
            t.col-=1;
            t.col-=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check4=Vector2Distance(pix, pacpos);
                
            }
            else{
                check4=10000.0f;
            }
            t.col+=1;
            
            if(check2<check3 && check2<check4){
                g->dir=down;
            }
            else if(check3<check2 && check3<check4){
                g->dir=right;
            }
            else{
                g->dir=left;
            }
        }
        else if(g->dir==right){
            t.row-=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check1=Vector2Distance(pix, pacpos);
                
        
            }
            else{
                check1=10000.0f;
            }
            t.row+=1;
            t.row+=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check2=Vector2Distance(pix, pacpos);
                
                
            }
            else{
                check2=10000.0f;
            }
            t.row-=1;
            t.col+=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check3=Vector2Distance(pix, pacpos);
                
            }
            else{
                check3=10000.0f;
            }
            t.col-=1;
            
            if(check1<check2 && check1<check3 ){
                g->dir=up;
                
            }
            else if(check2<check1 && check2<check3){
                g->dir=down;
            }
            else{
                g->dir=right;
            }
            
        }
        else if(g->dir==left){
            t.row-=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check1=Vector2Distance(pix, pacpos);
                
        
            }
            else{
                check1=10000.0f;
            }
            t.row+=1;
            t.row+=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check2=Vector2Distance(pix, pacpos);
                
                
            }
            else{
                check2=10000.0f;
            }
            t.row-=1;
            t.col-=1;
            w=wall(t);
            if(!w){
                Vector2 pix=pixel(t);
                check4=Vector2Distance(pix, pacpos);
                
            }
            else{
                check4=10000.0f;
            }
            
            
            if(check1<check2 && check1<check4 ){
                g->dir=up;
                
            }
            else if(check2<check1 && check2<check4){
                g->dir=down;
            }
            else{
                g->dir=left;
            }
            
            
        }
        if(t.row == 14  && (t.col < 0 || t.col >= wtiles)){
            if(g->position.x < 564){
                g->position.x = 1288;
            }
            else if(g->position.x > 1288){
                g->position.x = 564;
            }
        
        }

    }
        

void pinky_chase_alg(ghost *g,Vector2 pacpos,Vector2 speed){
    Vector2 target=pacpos;
    if(speed.x>0)
        target.x+=4*26;
    else if(speed.x<0)
        target.x-=4*26;
    else if(speed.y>0)
        target.y+=4*26;
    else if(speed.y<0){
        target.y-=4*26;
    }
    tile t=tiles_no(g->position);
    bool w = wall(t);
    float check1,check2,check3,check4;
    Vector2 pix;
    blinky_chase_alg(g, target);
}


void inky_chase_alg(ghost *g,ghost *g1,Vector2 pacpos, Vector2 speed){

    Vector2 target=pacpos;
    Vector2 new_target;
    if(speed.x>0)
    target.x+=2*26;
    else if(speed.x<0)
    target.x-=2*26;
    else if(speed.y>0)
    target.y+=2*26;
    else if(speed.y<0){
    target.y-=2*26;
    }
    tile t=tiles_no(g->position);
    
    bool w = wall(t);
    float check1,check2,check3,check4;
    Vector2 pix;

    new_target.x=2*target.x-g1->position.x;
    new_target.y=2*target.y-g1->position.y;

    blinky_chase_alg(g,new_target);



}

void clyde_chase_alg(ghost *g,Vector2 pacpos){

    float distance=Vector2Distance(g->position,pacpos);
    if(distance>(8*26)){
        blinky_chase_alg(g,pacpos);
    }
    else if(distance<=(8*26)){
        clyde_scatter_alg(g);
    }
}

void blinky_scatter_alg(ghost *g){
    
    Vector2 scatter_row_pix=pixel((tile){g->scatter_row,g->scatter_col});
    

    blinky_chase_alg(g,scatter_row_pix);


}

Vector2 pac_speed;

void pinky_scatter_alg(ghost *g){
    
    Vector2 scatter_row_pix=pixel((tile){g->scatter_row,g->scatter_col});
    

    blinky_chase_alg(g,scatter_row_pix);


}
    
void inky_scatter_alg(ghost *g){
    
    Vector2 scatter_row_pix=pixel((tile){g->scatter_row,g->scatter_col});

    blinky_chase_alg(g,scatter_row_pix);

}

void clyde_scatter_alg(ghost *g){
    
    Vector2 scatter_row_pix=pixel((tile){g->scatter_row,g->scatter_col});

    blinky_chase_alg(g,scatter_row_pix);

}

float speed;


void movement(ghost *g,float speed){
    tile current = tiles_no(g->position);
    Vector2 aligned = pixel(current);

    Vector2 next = g->position;
    
    if(g->dir == up)
        next.y -= speed*dt;
    else if(g->dir == down) 
         next.y += speed*dt;
    else if(g->dir == right) 
        next.x += speed*dt;
    else if(g->dir == left) 
         next.x -= speed*dt;

    Vector2 lead = next;
    if(g->dir == right) 
        lead.x += 23;
    else if(g->dir == down) 
        lead.y += 23;

    tile next_tile = tiles_no(lead);

    if(!wall(next_tile)){
        g->position = next;              
    } else {
        g->position = aligned;           
    }        
    

    if (current.row == 14) {
        if (g->position.x < 564) {
            g->position.x = pixel((tile){ 14, 27 }).x;
        }
        else if (g->position.x > 1314) {
            g->position.x = pixel((tile){ 14, 0 }).x;
        }
    }
    
}

int random;

void flip_dir(ghost *g){

    if(g->dir==up){
        g->dir=down;
    }
    else if(g->dir==down){
        g->dir=up;
    }
    else if(g->dir==right){
        g->dir=left;
    }
    else if(g->dir==left){
        g->dir=right;
    }
}

void ghost_frightened(ghost *g){
    tile current = tiles_no(g->position);
    direction choices[3];
    int count = 0;

    
    direction candidates[4] = { up, down, right, left };
    direction reverse = g->dir;
    if (reverse == up)
        reverse = down;
    else if (reverse == down)
        reverse = up;
    else if (reverse == right)
        reverse = left;
    else
        reverse = right;

    for (int i = 0; i < 4; ++i) {
        direction candidate = candidates[i];
        tile next = current;
        if (candidate == up)
            --next.row;
        else if (candidate == down)
            ++next.row;
        else if (candidate == right)
            ++next.col;
        else
            --next.col;

        if (!wall(next) && candidate != reverse)
            choices[count++] = candidate;
    }

    if (count == 0) {
        g->dir = reverse; 
    } else {
        g->dir = choices[rand() % count];
    }
    g->color=BLUE;
    
}


static void choose_eaten_direction(ghost *g, tile home) {
    tile current = tiles_no(g->position);
    direction candidates[4] = { up, left, down, right };
    
    
    direction reverse = g->dir;
    if (reverse == up) reverse = down;
    else if (reverse == down) reverse = up;
    else if (reverse == right) reverse = left;
    else reverse = right;

    float best_distance = INFINITY;
    direction best_dir = reverse;
    bool found_path = false;

    for (int i = 0; i < 4; ++i) {
        
        if (candidates[i] == reverse) continue;

        tile next = current;
        if (candidates[i] == up)         --next.row;
        else if (candidates[i] == down)  ++next.row;
        else if (candidates[i] == right) ++next.col;
        else                             --next.col;

        if (!wall(next)) {
            float distance = Vector2Distance(pixel(next), pixel(home));
            if (distance < best_distance) {
                best_distance = distance;
                best_dir = candidates[i];
                found_path = true;
            }
        }
    }

    
    if (found_path) {
        g->dir = best_dir;
    } else {
        g->dir = reverse;
    }
}

void eaten_phase_blinky(ghost *g){
    choose_eaten_direction(g, (tile){ 11, 13 });
}

void eaten_phase_pinky(ghost *g){
    choose_eaten_direction(g, (tile){ 11, 12 });
}

void eaten_phase_inky(ghost *g){
    choose_eaten_direction(g, (tile){ 11, 15 });
}

void eaten_phase_clyde(ghost *g){
    choose_eaten_direction(g, (tile){ 11, 14 });
}

void check_eaten_reset(ghost *g, tile home) {
    tile t = tiles_no(g->position);
    if (g->eaten && t.row == home.row && t.col == home.col) {
        g->eaten = false;
        g->ignore_frightened = true;
    }
}

Rectangle g_rec(ghost *g, ghost_name name){
    return (Rectangle){g->position.x,g->position.y,24,24};
}


