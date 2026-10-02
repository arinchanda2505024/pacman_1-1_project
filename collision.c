#include "collision.h"
#include "raylib.h"
#include "game_config.h"
Rectangle wall_rect;
bool collision(Rectangle entity, char map[htiles][wtiles+1]){
    for(int i = 0; i < htiles; i++){
        for(int j = 0; j < wtiles; j++){
            if(map[i][j] == 'w' || map[i][j] == 's' ||map[i][j] == 'p' || map[i][j] == 'a' || map[i][j] == 't' || map[i][j] == 'v'||
               map[i][j] == 'g' || map[i][j] == 'r' || map[i][j] == 'u'|| map[i][j] == 'f' || map[i][j] == 'l' || map[i][j] == 'c'){
                wall_rect=(Rectangle){map_x+j*tile_size,map_y+i*tile_size,tile_size,tile_size};
                if(CheckCollisionRecs(entity, wall_rect)){
                    return true;
                }
    
            }
        }
    }
    return false;
}