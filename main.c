#include "raylib.h"
#include "raymath.h"
#include "collision.h"
#include "menu.h"
#include <stddef.h>
#include "ghost.h"
#include "game_config.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>



static const char *DifficultyName(int difficulty)
{
    if (difficulty == 0)
    return "Easy";
    if (difficulty == 2)
    return "Hard";

    return "Normal";
}


static bool near_tile_line(float value, float origin, float tolerance)
{
    float line = origin + roundf((value - origin) / tile_size_float) * tile_size_float;
    return fabsf(value - line) <= tolerance;
}

static float nearest_tile_line(float value, float origin)
{
    return origin + roundf((value - origin) / tile_size_float) * tile_size_float;
}



static bool LoadMap(const char *filename, char destination[htiles][wtiles + 1])
{
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        TraceLog(LOG_ERROR, "Could not open map file: %s", filename);
        return false;
    }

    char line[128];

    for (int row = 0; row < htiles; row++) {
        if (fgets(line, sizeof(line), file) == NULL) {
            TraceLog(LOG_ERROR, "Map has fewer than %d rows: %s", htiles, filename);
            fclose(file);
            return false;
        }

        line[strcspn(line, "\r\n")] = '\0';

        if ((int)strlen(line) != wtiles) {
            TraceLog(LOG_ERROR, "Map row %d must have exactly %d characters: %s", row + 1, wtiles, filename);
                     
            fclose(file);
            return false;
        }

        memcpy(destination[row], line, wtiles);
        destination[row][wtiles] = '\0';
    }

    fclose(file);
    return true;
}
char map[htiles][wtiles + 1];
//pacman er first position
Rectangle pacman= {pac_start_x, pac_start_y,pac_ghost_size,pac_ghost_size}; 
    
    

int main(){
    
    InitWindow(gwidth, gheight, "Pacman");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    InitAudioDevice(); 

    
    bool return_to_menu = false;
    bool after_intro = false;
    

    const char *background_path = "assets\\sprite\\pacman_intro.png";
    Image background_image = LoadImage(background_path);
    Texture2D background = { 0 };

    if (IsImageValid(background_image)) {
        ImageResize(&background_image, gwidth, gheight);
        background = LoadTextureFromImage(background_image);
        UnloadImage(background_image);
    } else {
        TraceLog(LOG_ERROR, "Could not load background image: %s", background_path);
    }

    Sound menu_hover_sound = LoadSound("assets\\audio\\tunetank.com_menu-hover-click.wav");
    SetSoundVolume(menu_hover_sound, 0.35f);
    Sound menu_click_sound = LoadSound("assets\\audio\\freesound_community-menu-selection-102220.mp3");
    
    Sound start_sound = LoadSound("assets\\audio\\02. Start Music.mp3");
    PlaySound(start_sound);

    while(!WindowShouldClose() && !after_intro){
        
        BeginDrawing();
        ClearBackground(BLACK);


        if (IsTextureValid(background)) {
            DrawTexture(background, 0, 0, WHITE);
        } else {
            DrawText("BACKGROUND IMAGE FAILED TO LOAD", 620, 430, 32, RED);
        }
        DrawText("Press Enter to Continue", 700, 900, 40, RAYWHITE);
        if(IsKeyPressed(KEY_ENTER)){
            after_intro=true;
        }


        EndDrawing();
    }

    UnloadSound(start_sound);
    
    bool is_sound_on = true;
    main_menu:
    bool paused = false;
    MenuScreen menu_screen = menu_main;
    LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES] = { 0 };
    int leaderboard_count = LoadLeaderboard(leaderboard);
    char player_name[MAX_PLAYER_NAME + 1] = "";
    int player_name_length = 0;
    int selected_difficulty = 1;
    int leaderboard_difficulty = 0;
    bool start_game = false;
    
    

    
    Texture blinky_rules = LoadTexture("assets\\ghost_sprite\\blinky_right_1.png");
    Texture pinky_rules = LoadTexture("assets\\ghost_sprite\\pinky_right_1.png");
    Texture inky_rules = LoadTexture("assets\\ghost_sprite\\inky_right_1.png");
    Texture clyde_rules = LoadTexture("assets\\ghost_sprite\\clyde_right_1.png");

    while (!WindowShouldClose() && !start_game) {
        BeginDrawing();
        ClearBackground((Color){ 8, 10, 28, 255 });
        

        

        DrawText("PACMAN", 820, 100, 64, YELLOW);

        if (menu_screen == menu_main) {
            DrawText("Main Menu", 860, 190, 34, RAYWHITE);

            if (menu_button((Rectangle){ 760, 320, 380, menu_text_size }, "Play",0,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                menu_screen = menu_name_input;
            }
            
            if (menu_button((Rectangle){ 760, 395, 380, menu_text_size }, "Leaderboard",1,menu_hover_sound,menu_click_sound,is_sound_on,100)){
                menu_screen = menu_leaderboard;
            }
            if (menu_button((Rectangle){ 760, 470, 380, menu_text_size },  is_sound_on? "Sound on":"Sound off",2,menu_hover_sound,menu_click_sound,is_sound_on,125)){
                is_sound_on = !is_sound_on;
            }
            if (menu_button((Rectangle){ 760, 545, 380, menu_text_size }, "Game Rules",20,menu_hover_sound,menu_click_sound,is_sound_on,113)){
                menu_screen = menu_rules;
            }
            if (menu_button((Rectangle){ 760, 620, 380, menu_text_size }, "About Us",3,menu_hover_sound,menu_click_sound,is_sound_on,125)){
                menu_screen = menu_about;
            }
            if (menu_button((Rectangle){ 760, 695, 380, menu_text_size }, "Quit",4,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                EndDrawing();
                CloseAudioDevice();
                CloseWindow();
                return 0;
            }
        }
        else if (menu_screen == menu_name_input) {
            DrawText("Enter your name", 810, 260, 36, RAYWHITE);
            DrawRectangle(700, 330, 500, 58, DARKBLUE);
            DrawRectangleLinesEx((Rectangle){ 700, 330, 500, 58 }, 2.0f, SKYBLUE);
            DrawText(player_name, 720, 345, 30, WHITE);
            DrawText("Press Enter to continue", 812, 430, 24, LIGHTGRAY);

            int key = GetCharPressed();
            while (key > 0) {
                if (isprint((unsigned char)key) && key != ' ' && player_name_length < MAX_PLAYER_NAME) {
                    
                    player_name[player_name_length++] = (char)key;
                    player_name[player_name_length] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && player_name_length > 0) {
                player_name[--player_name_length] = '\0';
            }
            if (IsKeyPressed(KEY_ENTER) && player_name_length > 0) {
                menu_screen = menu_difficulty;
            }
            if (menu_button((Rectangle){ 760, 560, 380, menu_text_size }, "Back",8,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                menu_screen = menu_main;
            }
        }
        else if (menu_screen == menu_difficulty) {
            DrawText("Choose difficulty", 735, 220, 36, RAYWHITE);
            if (menu_button((Rectangle){ 760, 300, 380, menu_text_size }, "Easy",5,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                selected_difficulty = 0;
                start_game = true;
            }
            if (menu_button((Rectangle){ 760, 380, 380, menu_text_size }, "Normal",6,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                selected_difficulty = 1;
                start_game = true;
            }
            if (menu_button((Rectangle){ 760, 460, 380, menu_text_size }, "Hard",7,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                selected_difficulty = 2;
                start_game = true;
            }
            if (menu_button((Rectangle){ 760, 560, 380, menu_text_size }, "Back",8,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                menu_screen = menu_main;
            }
        }
        else if (menu_screen ==menu_leaderboard) {
            DrawText("Leaderboard", 822, 185, 40, YELLOW);
            if (menu_button((Rectangle){ 590, 255, 210, 55 }, "Easy",9,menu_hover_sound,menu_click_sound,is_sound_on,63)) {
                leaderboard_difficulty = 0;
            }
            if (menu_button((Rectangle){ 845, 255, 210, 55 }, "Normal",10,menu_hover_sound,menu_click_sound,is_sound_on,60)) {
                leaderboard_difficulty = 1;
            }
            if (menu_button((Rectangle){ 1100, 255, 210, 55 }, "Hard",11,menu_hover_sound,menu_click_sound,is_sound_on,66)) {
                leaderboard_difficulty = 2;
            }

            DrawText(TextFormat("%s leaderboard", DifficultyName(leaderboard_difficulty)),810, 335, 28, RAYWHITE);
                     
            DrawText("Player", 600, 385, 26, SKYBLUE);
            DrawText("Score", 950, 385, 26, SKYBLUE);
            DrawText("Time", 1130, 385, 26, SKYBLUE);

            int rank = 1;
            for (int i = 0; i < leaderboard_count; i++) {
                if (leaderboard[i].difficulty != leaderboard_difficulty)
                    continue;

                DrawText(TextFormat("%d. %s", rank, leaderboard[i].name),600, 430 + (rank - 1) * 40, 24, RAYWHITE);
                         
                DrawText(TextFormat("%d", leaderboard[i].score),950, 430 + (rank - 1) * 40, 24, RAYWHITE);
                         
                DrawText(TextFormat("%02d:%02d", leaderboard[i].seconds / 60,leaderboard[i].seconds % 60),1130, 430 + (rank - 1) * 40, 24, RAYWHITE);
                                    
                         
                rank++;
            }
            if (rank == 1) {
                DrawText("No completed games for this difficulty.",690, 440, 26, LIGHTGRAY);
                         
            }
            if (menu_button((Rectangle){ 760, 850, 380, menu_text_size }, "Back",12,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                menu_screen = menu_main;
            }
        }
        
        else if (menu_screen == menu_rules) {
            DrawText("Game Rules", 838, 195, 40, YELLOW);
            if (menu_button((Rectangle){ 760, 300, 380, menu_text_size }, "General",23,menu_hover_sound,menu_click_sound,is_sound_on,143)){
                menu_screen = menu_control;
            }
            if (menu_button((Rectangle){ 760, 380, 380, menu_text_size }, "Ghost",24,menu_hover_sound,menu_click_sound,is_sound_on,150)){
                menu_screen = menu_ghost;
            }
            if (menu_button((Rectangle){ 760, 460, 380, menu_text_size }, "Phase",25,menu_hover_sound,menu_click_sound,is_sound_on,150)){
                menu_screen = menu_phase;
            }
            if (menu_button((Rectangle){ 760, 560, 380, menu_text_size }, "Back",26,menu_hover_sound,menu_click_sound,is_sound_on,160)){
                menu_screen = menu_main;
            }
            
            
            
            
        }

   
            
        else if(menu_screen == menu_control){
            const char *rule_text =
            "-Eat every dot to finish the level.\n\n"
            "-Avoid ghosts unless they are frightened.\n\n"
            "-You can eat ghosts during their frightened phase.\n\n"
            "-Power pellets make ghosts frightened for a short time.\n\n"
            "-Use arrow keys or W A S D to move.\n\n";

            
            DrawText(rule_text, 640, 290, text_size, RAYWHITE);
            if (menu_button((Rectangle){ 760, 670, 380, menu_text_size }, "Back",13,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                menu_screen = menu_rules;
            }
        }
        else if(menu_screen == menu_ghost){
            


            DrawTexture(blinky_rules, 430 , 290, WHITE);
            DrawText("(Blinky) :", 480, 290, text_size, RED);
            const char *blinky_text = "This ghost is the deadliest one. He will always chase you\nuntil he catches you. (Obviously during chase phase)";
            DrawText(blinky_text, 620, 290, text_size, RAYWHITE);

            DrawTexture(pinky_rules, 430, 390, WHITE );
            DrawText("(Pinky) :", 480, 390, text_size, PINK);
            const char *pinky_text = "This ghost doesn't stick with you like blinky does. But he\nwill ambush you. He always targets four tiles in front of you";
            DrawText(pinky_text, 620, 390, text_size, WHITE);

            DrawTexture(inky_rules, 430, 490, WHITE);
            DrawText("(Inky) :", 480, 490, text_size, SKYBLUE);
            const char *inky_text = "This ghost is totally unpredictable. He will come to catch\nyou all of a sudden before even you realise. Be careful of him!";
            DrawText(inky_text, 620, 490, text_size, WHITE);

            DrawTexture(clyde_rules, 430, 590, WHITE);
            DrawText("(Clyde) :", 480, 590, text_size, ORANGE);
            const char *clyde_text = "You can call him a coward one. He will chase you but suddenly\nhe changes his mind when he comes near you and go away from pacman";
            DrawText(clyde_text, 620, 590, text_size, WHITE);

            if (menu_button((Rectangle){ 760, 700, 380, menu_text_size }, "Back",13,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                menu_screen = menu_rules;
            }
        }
        else if(menu_screen == menu_phase){
            DrawText("Scatter :", 410, 290, text_size, BLUE);
            const char *scatter_text = "During this phase, ghosts won't chase you. They will go at\ndifferent corner of the map. This phase is 10 seconds long.";
            DrawText(scatter_text, 610, 290, text_size, WHITE);

            DrawText("Chase :", 410, 390, text_size, RED);
            const char *chase_text = "This is the most dangerous phase. Ghost will chase you continuously\nat this phase. You will be tired of avoiding them. This phase is 20 seconds long.";
            DrawText(scatter_text, 610, 390, text_size, WHITE);

            DrawText("Frightened :", 410, 490, text_size, GREEN);
            const char *frightened_text = "This your chance! You can eat the ghost during this phase.\nThis phase is 6 seconds long.";
            DrawText(scatter_text, 610, 490, text_size, WHITE);

            if (menu_button((Rectangle){ 760, 700, 380, menu_text_size }, "Back",13,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                menu_screen = menu_rules;
            }
        }
        else if (menu_screen == menu_about) {
            DrawText("About Us", 860, 220, 40, YELLOW);
            
            const char *about_text =
                "We are presenting our Level-1/Term-1 (L1T1) group project. We have recreated one of the most popular\n\n"
                "and OG game from 1980 till now using raylib functions. Presenting you the PACMAN! The game was developed \n\n"
                "by me Arin Kumar Chanda and my project partner Ahmed Muhaymin under the supervision of our respectful\n\n"
                "Dr. Ch. Md. Rakin Haider Sir. His supervision and guidance helped us to develop the game properly. We have\n\n"
                "added some exciting new maps and challenges. Let's see who can avoid those unpredictable ghosts and\n\n"
                "complete the maps fast with highest points.Hope you will enjoy the game.\n\n\n\n"
                "Thank you!\n";
                

            DrawText(about_text, 190, 300, text_size, RAYWHITE);
            
            if (menu_button((Rectangle){ 760, 830, 380, menu_text_size }, "Back",14,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                menu_screen = menu_main;
            }
        }
        

        EndDrawing();
    }

    

    if (!start_game) {
        CloseAudioDevice();
        CloseWindow();
        return 0;
    }
    restart:
    const char *map_file = "map_normal.txt";

    if (selected_difficulty == 0) {
        map_file = "map_easy.txt";
    } else if (selected_difficulty == 2) {
        map_file = "map_hard.txt";
    }

    
    if (!LoadMap(map_file, map)){
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    Sound dot_sound = LoadSound("assets\\audio\\freesound_community-carrotnom-92106.mp3");
    Sound big_dot_sound= LoadSound("assets\\audio\\chomp-1.mp3");
    Sound ghost_eaten_sound = LoadSound("assets\\audio\\universfield-power-punch-192118.mp3");
    Sound pac_eaten_sound = LoadSound("assets\\audio\\muhahaha-made-with-Voicemod.mp3");  
    Sound game_over_sound = LoadSound("assets\\audio\\cat-laughing-at-you-made-with-Voicemod.mp3");
    Sound game_finish_sound = LoadSound("assets\\audio\\dragon-studio-wow-423653.mp3");

    SetSoundVolume(dot_sound, 0.05f);
    SetSoundVolume(big_dot_sound, 0.5f);
    SetSoundVolume(ghost_eaten_sound, 0.5f);
    SetSoundVolume(pac_eaten_sound, 0.7f);
    SetSoundVolume(game_over_sound,0.5f);


    int minute=0,second=0;
    
    float timer_accumulator = 0.0f;
    int score=0;
    int wall_position_x[28];
    int wall_position_y[31];
    int life=3;
    int dot_count=0;
    float ghost_speed_multiplier;
    if(selected_difficulty == 0){
        ghost_speed_multiplier = 0.9f;

    }
    else if(selected_difficulty == 2){
        ghost_speed_multiplier = 1.2f;
    }
    else{
        ghost_speed_multiplier = 1.0f;
    }

    
    bool quit_requested = false;
                                   

    here:
    Vector2 position={976, 670};
    Vector2 speed = {0,0};
    Vector2 nextSpeed = {0, 0};
    float scattered_time=0.0f;
    float chase_time=0.0f;
    float frightened_time=0.0f;
    float eaten_time=0.0f;
    
    

    
    
    //w=horizontal single wall,v=vertical single wall, s=horizontal double wall, t=vertical double wall,d=dot,e=empty space
    //g = top right single corner, u = top left single corner
    //r= bottom right single corner, p = bottom left single corner
    //a = top right double corner, c = top left double corner
    //f= bottom right double corner, l= bottom left double corner
    
    //wall sprite er jonno
    Texture tex_c, tex_s, tex_a, tex_t, tex_v, tex_u;
    Texture tex_w, tex_g, tex_r, tex_f, tex_l, tex_p;

    const char *wall_folder;

    if (selected_difficulty == 0) {
        wall_folder = "assets\\sprite\\ez";
    }
    else if (selected_difficulty == 2) {
        wall_folder = "assets\\sprite\\hard";
    }
    else {
        wall_folder = "assets\\sprite";
    }

    tex_c = LoadTexture(TextFormat("%s\\WALL_DOUBLE_CORNER_TL.png", wall_folder));
    tex_s = LoadTexture(TextFormat("%s\\WALL_DOUBLE_H.png", wall_folder));
    tex_a = LoadTexture(TextFormat("%s\\WALL_DOUBLE_CORNER_TR.png", wall_folder));
    tex_t = LoadTexture(TextFormat("%s\\WALL_DOUBLE_V.png", wall_folder));
    tex_v = LoadTexture(TextFormat("%s\\WALL_SINGLE_V.png", wall_folder));
    tex_u = LoadTexture(TextFormat("%s\\WALL_SINGLE_CORNER_TL.png", wall_folder));
    tex_w = LoadTexture(TextFormat("%s\\WALL_SINGLE_H.png", wall_folder));
    tex_g = LoadTexture(TextFormat("%s\\WALL_SINGLE_CORNER_TR.png", wall_folder));
    tex_r = LoadTexture(TextFormat("%s\\WALL_SINGLE_CORNER_BR.png", wall_folder));
    tex_f = LoadTexture(TextFormat("%s\\WALL_DOUBLE_CORNER_BR.png", wall_folder));
    tex_l = LoadTexture(TextFormat("%s\\WALL_DOUBLE_CORNER_BL.png", wall_folder));
    tex_p = LoadTexture(TextFormat("%s\\WALL_SINGLE_CORNER_BL.png", wall_folder));
    Texture pac_sprite_up[3];
    Texture pac_sprite_down[3];
    Texture pac_sprite_right[3];
    Texture pac_sprite_left[3];
    Texture blinky_up[2];
    Texture pinky_up[2];
    Texture inky_up[2];
    Texture clyde_up[2];
    Texture blinky_down[2];
    Texture pinky_down[2];
    Texture inky_down[2];
    Texture clyde_down[2];
    Texture blinky_right[2];
    Texture pinky_right[2];
    Texture inky_right[2];
    Texture clyde_right[2];
    Texture blinky_left[2];
    Texture pinky_left[2];
    Texture inky_left[2];
    Texture clyde_left[2];
    Texture frightened_ghost[4];
    Texture eaten_ghost_up = LoadTexture("assets\\ghost_eaten\\eaten_up.png");
    Texture eaten_ghost_down = LoadTexture("assets\\ghost_eaten\\eaten_down.png");
    Texture eaten_ghost_right = LoadTexture("assets\\ghost_eaten\\eaten_right.png");
    Texture eaten_ghost_left = LoadTexture("assets\\ghost_eaten\\eaten_left.png");
    Texture portal_left = LoadTexture("assets\\portal\\portal_facing_left.png");
    Texture portal_right = LoadTexture("assets\\portal\\portal_facing_right.png");
    

    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\pac_sprite\\pac_man_up_%d.png", i+1);
        pac_sprite_up[i] = LoadTexture(path);
    }

    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\pac_sprite\\pac_man_down_%d.png", i+1);
        pac_sprite_down[i] = LoadTexture(path);
    }

    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\pac_sprite\\pac_man_right_%d.png", i+1);
        pac_sprite_right[i] = LoadTexture(path);
    }

    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\pac_sprite\\pac_man_left_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }

    for(int i = 0; i < 4; i++){
        char path[100];
        sprintf(path, "assets\\ghost_frightened\\frightened_%d.png", i+1);
        frightened_ghost[i] = LoadTexture(path);
    }


    
   ghost_sprite(blinky_up,2, "blinky_up");
   ghost_sprite(inky_up,2, "inky_up");
   ghost_sprite(pinky_up,2, "pinky_up");
   ghost_sprite(clyde_up,2, "clyde_up");

   ghost_sprite(inky_down,2, "inky_down");
   ghost_sprite(blinky_down,2, "blinky_down");
   ghost_sprite(pinky_down,2, "pinky_down");
   ghost_sprite(clyde_down,2, "clyde_down");

   ghost_sprite(inky_right,2, "inky_right");
   ghost_sprite(blinky_right,2, "blinky_right");
   ghost_sprite(pinky_right,2, "pinky_right");
   ghost_sprite(clyde_right,2, "clyde_right");

   ghost_sprite(inky_left,2, "inky_left");
   ghost_sprite(blinky_left,2, "blinky_left");
   ghost_sprite(pinky_left,2, "pinky_left");
   ghost_sprite(clyde_left,2, "clyde_left");
    
    
    
    
    
    
    ghost blinky_ghost;
    ghost pinky_ghost;
    ghost inky_ghost;
    ghost clyde_ghost;
    
    init_phase(&blinky_ghost, blinky);
    init_phase(&pinky_ghost, pinky);
    init_phase(&inky_ghost, inky);
    init_phase(&clyde_ghost, clyde);

    ghost_mode phase=scattered;
    bool over=false;
    bool level_complete = false;
    bool leaderboard_saved = false;
    
    while(!WindowShouldClose()){


        Rectangle ghost_rec_blinky=g_rec(&blinky_ghost,blinky);
        Rectangle ghost_rec_pinky=g_rec(&pinky_ghost,pinky);
        Rectangle ghost_rec_inky=g_rec(&inky_ghost,inky);
        Rectangle ghost_rec_clyde=g_rec(&clyde_ghost,clyde);

        float dt= GetFrameTime();

        if (IsKeyPressed(KEY_ESCAPE) && !over && !level_complete) {
            paused = !paused;
        }


        if(!paused){
            
            //pacman er movement:
            if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)){
                nextSpeed = (Vector2){-main_speed, 0};
            }
            else if(IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)){
                nextSpeed = (Vector2){main_speed, 0};
            }
            else if(IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)){
                nextSpeed = (Vector2){0, main_speed};
            }
            else if(IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)){
                nextSpeed = (Vector2){0, -main_speed};
            }

            float shrink = 2;

            
            bool moving_horizontally = fabsf(speed.x) > 0.0f;
            bool requesting_horizontal = fabsf(nextSpeed.x) > 0.0f;
            bool perpendicular_turn = (fabsf(speed.x) > 0.0f || fabsf(speed.y) > 0.0f) && (moving_horizontally != requesting_horizontal);
                                    
            float snap_tolerance = main_speed * dt + 0.01f;
            bool at_turn_line = !perpendicular_turn || (moving_horizontally ? near_tile_line(position.x, map_x_float, snap_tolerance) : near_tile_line(position.y, map_y_float, snap_tolerance));
                                    
            if (at_turn_line) {
                Vector2 turn_position = position;

                if (perpendicular_turn) {
                    if (moving_horizontally) {
                        turn_position.x = nearest_tile_line(position.x, map_x_float);
                    } else {
                        turn_position.y = nearest_tile_line(position.y, map_y_float);
                    }
                }

                Rectangle turnBox = {
                    turn_position.x + nextSpeed.x * dt + shrink / 2,
                    turn_position.y + nextSpeed.y * dt + shrink / 2,
                    tile_size - shrink,
                    tile_size - shrink
                };

                if (!collision(turnBox, map)) {
                    position = turn_position;
                    speed = nextSpeed;
                }
            }

            Rectangle collisionBox = {
                position.x + speed.x * dt + shrink / 2,
                position.y + speed.y * dt + shrink / 2,
                tile_size - shrink,
                tile_size - shrink
            };
            //pacman er position update
            
            
            if(!collision(collisionBox, map)){
                
                
                position.x += speed.x*dt;
                position.y += speed.y*dt;
                
            }
            else {
                speed = (Vector2){0, 0};
            }
            
            pacman= (Rectangle){position.x,position.y,pac_ghost_size,pac_ghost_size};
            //dot collection
                
            //pacman er center khuje tiles number ber kora
            int tile_j = (int)((position.x + 12 - map_x) / tile_size);
            int tile_i = (int)((position.y + 12 - map_y) / tile_size);

            if (tile_i >= 0 && tile_i < htiles && tile_j >= 0 && tile_j < wtiles) {
                if (map[tile_i][tile_j] == 'd') {
                    map[tile_i][tile_j] = 'e'; 
                    score += 10;
                    if(is_sound_on)
                    PlaySound(dot_sound);
                } 
                else if (map[tile_i][tile_j] == 'b') {
                    map[tile_i][tile_j] = 'e'; 
                    score += 50;
                    if(is_sound_on)
                    PlaySound(big_dot_sound);

                    phase=frightened;
                    frightened_time = 0.0f;
                    if (!blinky_ghost.eaten) {
                        blinky_ghost.ignore_frightened = false;
                        flip_dir(&blinky_ghost);
                    }

                    if (!pinky_ghost.eaten) {
                        pinky_ghost.ignore_frightened = false;
                        flip_dir(&pinky_ghost);
                    }

                    if (!inky_ghost.eaten) {
                        inky_ghost.ignore_frightened = false;
                        flip_dir(&inky_ghost);
                    }

                    if (!clyde_ghost.eaten) {
                        clyde_ghost.ignore_frightened = false;
                        flip_dir(&clyde_ghost);
                    }
                }
            }

            
        if(tile_i == 14){
                if(position.x < 564){
                    position.x = 1288;
                }
                else if(position.x > 1288){
                    position.x = 564;
                }
            }
            else{
                if(position.x < 612){
                    position.x = 612;
                }
                else if(position.x > 1262){
                    position.x=1262;
                }
            }

            if(position.y < 98){
                position.y = 98;
            }
            else if(position.y > 826){
                position.y = 826;
            }
            
            
            
            
            
            if(!over && !level_complete){
                if(phase == scattered){
                    if(scattered_time<10.0f){
                        
                        tile ghost_tile_blinky = tiles_no(blinky_ghost.position);
                        tile ghost_tile_pinky = tiles_no(pinky_ghost.position);
                        tile ghost_tile_inky = tiles_no(inky_ghost.position);
                        tile ghost_tile_clyde = tiles_no(clyde_ghost.position);

                        Vector2 blinky_tile_position =pixel(ghost_tile_blinky);
                        Vector2 pinky_tile_position =pixel(ghost_tile_pinky);
                        Vector2 inky_tile_position =pixel(ghost_tile_inky);
                        Vector2 clyde_tile_position =pixel(ghost_tile_clyde);

                        if (fabsf(blinky_ghost.position.x - blinky_tile_position.x) < 1.0f && fabsf(blinky_ghost.position.y - blinky_tile_position.y) < 1.0f) 
                        {
                            blinky_scatter_alg(&blinky_ghost);
                        }
                        if (fabsf(pinky_ghost.position.x - pinky_tile_position.x) < 1.0f && fabsf(pinky_ghost.position.y - pinky_tile_position.y) < 1.0f)  
                        {
                            pinky_scatter_alg(&pinky_ghost);
                        }

                        if (fabsf(inky_ghost.position.x - inky_tile_position.x) < 1.0f && fabsf(inky_ghost.position.y - inky_tile_position.y) < 1.0f)  
                        {
                            inky_scatter_alg(&inky_ghost);
                        }
                        if (fabsf(clyde_ghost.position.x - clyde_tile_position.x) < 1.0f && fabsf(clyde_ghost.position.y - clyde_tile_position.y) < 1.0f)  
                        {
                            clyde_scatter_alg(&clyde_ghost);
                        }
                            
                        
                        scattered_time+=dt;
                    }
                    else{
                        phase = chase;
                        scattered_time = 0.0f;
                        chase_time = 0.0f;
                    }
                    
                }
            
            
                Vector2 pacpos={position.x,position.y};
            

            
                if(phase == chase){
                    if(chase_time<20.0f){
                        
                        tile ghost_tile_blinky = tiles_no(blinky_ghost.position);
                        tile ghost_tile_pinky = tiles_no(pinky_ghost.position);
                        tile ghost_tile_inky = tiles_no(inky_ghost.position);
                        tile ghost_tile_clyde = tiles_no(clyde_ghost.position);

                        Vector2 blinky_tile_position =pixel(ghost_tile_blinky);
                        Vector2 pinky_tile_position =pixel(ghost_tile_pinky);
                        Vector2 inky_tile_position =pixel(ghost_tile_inky);
                        Vector2 clyde_tile_position =pixel(ghost_tile_clyde);

                        if (fabsf(blinky_ghost.position.x - blinky_tile_position.x) < 1.0f && fabsf(blinky_ghost.position.y - blinky_tile_position.y) < 1.0f)
                        {
                            
                            
                            blinky_chase_alg(&blinky_ghost, pacpos);
                            
                        }
                        if (fabsf(pinky_ghost.position.x - pinky_tile_position.x) < 1.0f && fabsf(pinky_ghost.position.y - pinky_tile_position.y) < 1.0f)
                        {
                            
                            
                            pinky_chase_alg(&pinky_ghost, pacpos, speed);
                            
                        }

                        if (fabsf(inky_ghost.position.x - inky_tile_position.x) < 1.0f && fabsf(inky_ghost.position.y - inky_tile_position.y) < 1.0f)
                        {
                            
                            
                            inky_chase_alg(&inky_ghost, &blinky_ghost, pacpos, speed);
                            
                        }
                        if (fabsf(clyde_ghost.position.x - clyde_tile_position.x) < 1.0f && fabsf(clyde_ghost.position.y - clyde_tile_position.y) < 1.0f)
                        {
                            
                            
                            clyde_chase_alg(&clyde_ghost, pacpos);
                            
                        }
                            
                        
                        chase_time += dt;
                    }
                    else{
                        phase = scattered;
                        scattered_time = 0.0f;
                        chase_time = 0.0f;
                    }
                    
                }
            
            
                if (phase == frightened){
                    if (CheckCollisionRecs(pacman, ghost_rec_blinky) && !blinky_ghost.eaten && !blinky_ghost.ignore_frightened) {
                        
                        blinky_ghost.eaten = true;
                        if(is_sound_on)
                        PlaySound(ghost_eaten_sound);
                        score += 200;
                    }

                    if (CheckCollisionRecs(pacman, ghost_rec_pinky) && !pinky_ghost.eaten && !pinky_ghost.ignore_frightened) {
                        
                        pinky_ghost.eaten = true;
                        if(is_sound_on)
                        PlaySound(ghost_eaten_sound);
                        score += 200;
                    }
                    if (CheckCollisionRecs(pacman, ghost_rec_inky) && !inky_ghost.eaten && !inky_ghost.ignore_frightened) {
                        
                        inky_ghost.eaten = true;
                        if(is_sound_on)
                        PlaySound(ghost_eaten_sound);
                        score += 200;
                    }
                    if (CheckCollisionRecs(pacman, ghost_rec_clyde) && !clyde_ghost.eaten && !clyde_ghost.ignore_frightened) {
                        
                        clyde_ghost.eaten = true;
                        if(is_sound_on)
                        PlaySound(ghost_eaten_sound);
                        score += 200;
                    }

                    
                    frightened_time += dt;

                    tile ghost_tile_blinky = tiles_no(blinky_ghost.position);
                    tile ghost_tile_pinky = tiles_no(pinky_ghost.position);
                    tile ghost_tile_inky = tiles_no(inky_ghost.position);
                    tile ghost_tile_clyde = tiles_no(clyde_ghost.position);

                    Vector2 blinky_tile_position =pixel(ghost_tile_blinky);
                    Vector2 pinky_tile_position =pixel(ghost_tile_pinky);
                    Vector2 inky_tile_position =pixel(ghost_tile_inky);
                    Vector2 clyde_tile_position =pixel(ghost_tile_clyde);

                    bool at_tile_blinky =fabsf(blinky_ghost.position.x - blinky_tile_position.x) < 1.0f && fabsf(blinky_ghost.position.y - blinky_tile_position.y) < 1.0f;
                        
                    bool at_tile_pinky = fabsf(pinky_ghost.position.x - pinky_tile_position.x) < 1.0f && fabsf(pinky_ghost.position.y - pinky_tile_position.y) < 1.0f;

                    bool at_tile_inky = fabsf(inky_ghost.position.x - inky_tile_position.x) < 1.0f && fabsf(inky_ghost.position.y - inky_tile_position.y) < 1.0f;

                    bool at_tile_clyde = fabsf(clyde_ghost.position.x - clyde_tile_position.x) < 1.0f && fabsf(clyde_ghost.position.y - clyde_tile_position.y) < 1.0f;
                        
                        

                    if (!blinky_ghost.eaten && at_tile_blinky && !blinky_ghost.ignore_frightened ) {
                        ghost_frightened(&blinky_ghost);
                        
                        
                    }
                    if (!pinky_ghost.eaten && at_tile_pinky && !pinky_ghost.ignore_frightened) {
                        ghost_frightened(&pinky_ghost);
                        
                        
                    }
                    if (!inky_ghost.eaten && at_tile_inky && !inky_ghost.ignore_frightened) {
                        ghost_frightened(&inky_ghost);
                        
                        
                    }
                    if (!clyde_ghost.eaten && at_tile_clyde && !clyde_ghost.ignore_frightened) {
                        ghost_frightened(&clyde_ghost);
                        
                        
                    }

                   
                    if (blinky_ghost.ignore_frightened && !blinky_ghost.eaten && at_tile_blinky) {
                        blinky_chase_alg(&blinky_ghost, pacpos);
                    }
                    if (pinky_ghost.ignore_frightened && !pinky_ghost.eaten && at_tile_pinky) {
                        pinky_chase_alg(&pinky_ghost, pacpos, speed);
                    }
                    if (inky_ghost.ignore_frightened && !inky_ghost.eaten && at_tile_inky) {
                        inky_chase_alg(&inky_ghost, &blinky_ghost, pacpos, speed);
                    }
                    if (clyde_ghost.ignore_frightened && !clyde_ghost.eaten && at_tile_clyde) {
                        clyde_chase_alg(&clyde_ghost, pacpos);
                    }


                    if (frightened_time >6.0f) {
                        phase = chase;
                        frightened_time = 0.0f;
                        chase_time = 0.0f;

                        blinky_ghost.ignore_frightened = false;
                        pinky_ghost.ignore_frightened = false;
                        inky_ghost.ignore_frightened = false;
                        clyde_ghost.ignore_frightened = false;
                    }
                    
                }
                
            
            
                if (blinky_ghost.eaten) {
                    
                    tile t = tiles_no(blinky_ghost.position);
                    Vector2 tile_pos = pixel(t);

                    bool at_tile = fabsf(blinky_ghost.position.x - tile_pos.x) < 1.5f && fabsf(blinky_ghost.position.y - tile_pos.y) < 1.5f;
                        
                        

                    if (at_tile) {
                        blinky_ghost.position = tile_pos;
                        eaten_phase_blinky(&blinky_ghost);
                    }

                    movement(&blinky_ghost, 156.0f);
                    check_eaten_reset(&blinky_ghost, (tile){ 11, 13 });
                }
                else if (phase == frightened && !blinky_ghost.ignore_frightened) {
                    movement(&blinky_ghost, ghost_frightended_speed * ghost_speed_multiplier);
                }
                else {
                    movement(&blinky_ghost, ghost_normal_speed * ghost_speed_multiplier);
                        
                }

                if (pinky_ghost.eaten) {
                    tile t = tiles_no(pinky_ghost.position);
                    Vector2 tile_pos = pixel(t);

                    bool at_tile =fabsf(pinky_ghost.position.x - tile_pos.x) < 1.5f && fabsf(pinky_ghost.position.y - tile_pos.y) < 1.5f;
                        
                        

                    if (at_tile) {
                        pinky_ghost.position = tile_pos;
                        eaten_phase_pinky(&pinky_ghost);
                    }
                    movement(&pinky_ghost, ghost_eaten_speed * ghost_speed_multiplier);
                    check_eaten_reset(&pinky_ghost, (tile){ 11, 12});
                }
                else if (phase == frightened && !pinky_ghost.ignore_frightened) {
                    movement(&pinky_ghost, ghost_frightended_speed * ghost_speed_multiplier);
                }
                else {
                    movement(&pinky_ghost, ghost_normal_speed * ghost_speed_multiplier);
                        
                }

            if (inky_ghost.eaten) {
                    
                    tile t = tiles_no(inky_ghost.position);
                    Vector2 tile_pos = pixel(t);

                    bool at_tile =fabsf(inky_ghost.position.x - tile_pos.x) < 1.5f && fabsf(inky_ghost.position.y - tile_pos.y) < 1.5f;
                        
                        

                    if (at_tile) {
                        inky_ghost.position = tile_pos;
                        eaten_phase_inky(&inky_ghost);
                    }
                    movement(&inky_ghost, ghost_eaten_speed * ghost_speed_multiplier);
                    check_eaten_reset(&inky_ghost, (tile){ 11, 15 });
                }
                else if (phase == frightened && !inky_ghost.ignore_frightened) {
                    movement(&inky_ghost, ghost_frightended_speed * ghost_speed_multiplier);
                }
                else {
                    movement(&inky_ghost, ghost_normal_speed * ghost_speed_multiplier);
                        
                }

                if (clyde_ghost.eaten) {
                    tile t = tiles_no(clyde_ghost.position);
                    Vector2 tile_pos = pixel(t);
 
                    bool at_tile = fabsf(clyde_ghost.position.x - tile_pos.x) < 1.5f && fabsf(clyde_ghost.position.y - tile_pos.y) < 1.5f;
                        
                        

                    if (at_tile) {

                        clyde_ghost.position = tile_pos;
                        eaten_phase_clyde(&clyde_ghost);
                    }
                    movement(&clyde_ghost, ghost_eaten_speed * ghost_speed_multiplier);
                    check_eaten_reset(&clyde_ghost, (tile){ 11, 14 });
                }
                else if (phase == frightened && !clyde_ghost.ignore_frightened) {
                    movement(&clyde_ghost, ghost_frightended_speed * ghost_speed_multiplier);
                }
                else {
                    movement(&clyde_ghost, ghost_normal_speed * ghost_speed_multiplier);
                        
                }
            }
        }

        

        BeginDrawing();
        ClearBackground(BLACK);

        
        for(int i = 0; i<htiles; i++){
            for(int j = 0; j<wtiles; j++){
                Texture *wall_map = NULL;
                switch(map[i][j]){
                    case 'c':
                    wall_map=&tex_c;
                    break;
                    
                    case 's':
                    wall_map=&tex_s;
                    break;

                    case 'a':
                    wall_map=&tex_a;
                    break;

                    case 't':
                    wall_map=&tex_t;
                    break;

                    case 'v':
                    wall_map=&tex_v;
                    break;

                    case 'u':
                    wall_map=&tex_u;
                    break;

                    case 'w':
                    wall_map=&tex_w;
                    break;

                    case 'g':
                    wall_map=&tex_g;
                    break;

                    case 'r':
                    wall_map=&tex_r;
                    break;

                    case 'f':
                    wall_map=&tex_f;
                    break;

                    case 'l':
                    wall_map=&tex_l;
                    break;

                    case 'p':
                    wall_map=&tex_p;

                    default:
                    break;
                }
                DrawTexture(portal_left,map_x-tile_size,map_y+tile_size*14-5,WHITE);
                DrawTexture(portal_right,map_x+wtiles*(tile_size),map_y+tile_size*14-5,WHITE);
                
                Rectangle dest = {map_x+j*tile_size, map_y+i*tile_size, tile_size, tile_size};
                if (wall_map != NULL) {
                    Rectangle source = {1, 1, (float)wall_map->width - 2, (float)wall_map->height - 2};
                    DrawTexturePro(*wall_map, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
                
                
            }
        
        
        }
    }

    if(phase == scattered){
        DrawText("Phase : ", 860, 30, 30, WHITE);
        DrawText("Scattered", 980,30, 30, BLUE);
    }
    if(phase == chase){
        DrawText("Phase : ", 860, 30, 30, WHITE);
        DrawText("Chase", 980,30, 30, RED);
    }
    if(phase == frightened){
        DrawText("Phase : ", 860, 30, 30, WHITE);
        DrawText("Frightened", 980,30, 30, GREEN);
    }

        
        for(int i=0; i<htiles;i++){
            for(int j=0; j<wtiles; j++){
                
                if(map[i][j]=='d'){
                    DrawRectangle(map_x + j * tile_size, map_y + i * tile_size, tile_size, tile_size , BLACK);
                    DrawCircle(map_x + j * tile_size + 13, map_y + i * tile_size + 13, 3, ORANGE);
                }
                else if(map[i][j]=='e'){
                    DrawRectangle(map_x + j * tile_size, map_y + i * tile_size, tile_size, tile_size, BLACK);
                }
                else if(map[i][j]=='b'){
                    DrawRectangle(map_x + j * tile_size, map_y + i * tile_size, tile_size, tile_size, BLACK);
                    DrawCircle(map_x + j * tile_size + 13, map_y + i * tile_size + 13, 8, YELLOW);
                }
                else if(map[i][j]=='i'){
                    DrawRectangle(map_x + j * tile_size, map_y + i * tile_size, tile_size, tile_size, PINK);
                    
                }



            }
        }
        bool hit_dangerous_ghost =(CheckCollisionRecs(pacman, ghost_rec_blinky) && !blinky_ghost.eaten) || (CheckCollisionRecs(pacman, ghost_rec_pinky)  && !pinky_ghost.eaten) || (CheckCollisionRecs(pacman, ghost_rec_inky)  && !inky_ghost.eaten) || (CheckCollisionRecs(pacman, ghost_rec_clyde)  && !clyde_ghost.eaten);
    
    
        
        if(!over && (phase == chase || phase == scattered) && hit_dangerous_ghost){
            if(CheckCollisionRecs(pacman, ghost_rec_blinky) || CheckCollisionRecs(pacman, ghost_rec_pinky) || CheckCollisionRecs(pacman, ghost_rec_inky) || CheckCollisionRecs(pacman, ghost_rec_clyde)){
                
                life--;
                if(life>0){
                    if(is_sound_on)
                    PlaySound(pac_eaten_sound);
                    goto here;
                }
                else if(life == 0){

                    over=true;
                    if(is_sound_on)
                    PlaySound(game_over_sound);
                    if (!leaderboard_saved) {
                        SaveLeaderboard(leaderboard, &leaderboard_count, player_name, score, minute * 60 + second, selected_difficulty);                
                        leaderboard_saved = true;
                    }
                    
                }
            }
            
        }

        if(over){
            speed = (Vector2){0, 0};
            nextSpeed = (Vector2){0, 0};
            movement(&blinky_ghost, 0.0f); 
            movement(&pinky_ghost, 0.0f); 
            movement(&inky_ghost, 0.0f);
            movement(&clyde_ghost, 0.0f);
        } 
        dot_count=0;
        for(int i=0;i<htiles;i++){
            for(int j=0;j<wtiles;j++){
                if(map[i][j]=='d' || map[i][j] == 'b'){
                    dot_count++;
                }
            }
        }

        if (dot_count == 0 && !leaderboard_saved) {
            level_complete = true;
            SaveLeaderboard(leaderboard, &leaderboard_count, player_name, score, minute * 60 + second,selected_difficulty);
                           
                            
            leaderboard_saved = true;
            if(is_sound_on)
            PlaySound(game_finish_sound);
        }
        /*if (level_complete) {
            DrawText("You passed the level!", 840, 30, 26, GREEN);
            nextSpeed=(Vector2){0.0f,0.0f};
            
                
        }*/
        
        
        

        //DrawRectangleRec(pacman, YELLOW);
        int frame = ((int)(GetTime() * 10)) % 3;
        if(speed.x>0){
            DrawTexture(pac_sprite_right[frame], position.x, position.y, WHITE);
        }
        else if(speed.x<0){
            DrawTexture(pac_sprite_left[frame], position.x, position.y, WHITE);
        }
        else if(speed.y>0){
            DrawTexture(pac_sprite_down[frame], position.x, position.y, WHITE);
        }
        else if(speed.y<0){
            DrawTexture(pac_sprite_up[frame], position.x, position.y, WHITE);
        }
        else if(speed.x == 0 && speed.y == 0){
            DrawTexture(pac_sprite_up[0], position.x, position.y, WHITE);
        }
        

        
        
        DrawText(TextFormat("SCORE: %d", score), 586, 30, game_screen_text, WHITE);
        DrawText("LIFE:",600,895,game_screen_text,WHITE);
        Texture2D life_sprite=LoadTexture("assets\\sprite\\life_sprite.png");


        
        if (!over && !level_complete && !paused) {
            timer_accumulator += dt;

            while (timer_accumulator >= 1.0f) {
                second++;
                timer_accumulator -= 1.0f;

                if (second >= 60) {
                    minute++;
                    second = 0;
                }
            }
        }
            
        
        DrawText(TextFormat("TIME: 0%d:%d", minute, second),1200,30,game_screen_text,WHITE);
        
        
        
        for(int i=0;i<life;i++){
            DrawTexture(life_sprite,700+50*i,895,YELLOW);
        }

        Rectangle draw_g_rec_blinky=g_rec(&blinky_ghost,blinky);
        Rectangle draw_g_rec_pinky = g_rec(&pinky_ghost,pinky);
        Rectangle draw_g_rec_inky = g_rec(&inky_ghost,inky);
        Rectangle draw_g_rec_clyde = g_rec(&clyde_ghost,clyde);

        //DrawRectangleRec(draw_g_rec_blinky,(phase == frightened && !blinky_ghost.ignore_frightened) ? BLUE : RED);
        int frame1 = ((int)(GetTime() * 10)) % 2;
        int frame2 = ((int)(GetTime() * 10)) % 4;

        if(phase == frightened && !blinky_ghost.ignore_frightened){
            DrawTexture(frightened_ghost[frame2],blinky_ghost.position.x,blinky_ghost.position.y,WHITE);
        }
        else{
            
            if(blinky_ghost.dir == up)
                draw_ghost_sprite(blinky_up, blinky_ghost, frame1);
            else if(blinky_ghost.dir == down)
                draw_ghost_sprite(blinky_down, blinky_ghost, frame1);
            else if(blinky_ghost.dir == right)
                draw_ghost_sprite(blinky_right, blinky_ghost, frame1);
            else if(blinky_ghost.dir == left)
                draw_ghost_sprite(blinky_left, blinky_ghost, frame1);
        }

        if(phase == frightened && !pinky_ghost.ignore_frightened){
            DrawTexture(frightened_ghost[frame2],pinky_ghost.position.x,pinky_ghost.position.y,WHITE);
        }
        else{
            
            if(pinky_ghost.dir == up)
                draw_ghost_sprite(pinky_up, pinky_ghost, frame1);
            else if(pinky_ghost.dir == down)
                draw_ghost_sprite(pinky_down, pinky_ghost, frame1);
            else if(pinky_ghost.dir == right)
                draw_ghost_sprite(pinky_right, pinky_ghost, frame1);
            else if(pinky_ghost.dir == left)
                draw_ghost_sprite(pinky_left, pinky_ghost, frame1);
        }

        if(phase == frightened && !inky_ghost.ignore_frightened){
            DrawTexture(frightened_ghost[frame2],inky_ghost.position.x,inky_ghost.position.y,WHITE);
        }
        else{
            
            if(inky_ghost.dir == up)
                draw_ghost_sprite(inky_up, inky_ghost, frame1);
            else if(inky_ghost.dir == down)
                draw_ghost_sprite(inky_down, inky_ghost, frame1);
            else if(inky_ghost.dir == right)
                draw_ghost_sprite(inky_right, inky_ghost, frame1);
            else if(inky_ghost.dir == left)
                draw_ghost_sprite(inky_left, inky_ghost, frame1);
        }

        if(phase == frightened && !clyde_ghost.ignore_frightened){
            DrawTexture(frightened_ghost[frame2],clyde_ghost.position.x,clyde_ghost.position.y,WHITE);
        }
        else{
            
            if(clyde_ghost.dir == up)
                draw_ghost_sprite(clyde_up, clyde_ghost, frame1);
            else if(clyde_ghost.dir == down)
                draw_ghost_sprite(clyde_down, clyde_ghost, frame1);
            else if(clyde_ghost.dir == right)
                draw_ghost_sprite(clyde_right, clyde_ghost, frame1);
            else if(clyde_ghost.dir == left)
                draw_ghost_sprite(clyde_left, clyde_ghost, frame1);
        }
            
        
        if(blinky_ghost.eaten){
            draw_eaten_ghost_sprite(eaten_ghost_up,eaten_ghost_down, eaten_ghost_right, eaten_ghost_left, blinky_ghost);
            
        }
        if(pinky_ghost.eaten){
           draw_eaten_ghost_sprite(eaten_ghost_up,eaten_ghost_down, eaten_ghost_right, eaten_ghost_left, pinky_ghost);
        }
        if(inky_ghost.eaten){
            draw_eaten_ghost_sprite(eaten_ghost_up,eaten_ghost_down, eaten_ghost_right, eaten_ghost_left, inky_ghost);
        }
        if(clyde_ghost.eaten){
            draw_eaten_ghost_sprite(eaten_ghost_up,eaten_ghost_down, eaten_ghost_right, eaten_ghost_left, clyde_ghost);
        }

        if (paused) {
            DrawRectangle(650, 280, 600, 480, Fade(BLACK, 0.85f));
            DrawRectangleLinesEx((Rectangle){ 650, 280, 600, 480 }, 2.0f, YELLOW);

            DrawText("PAUSED", 860, 320, 42, YELLOW);

            if (menu_button((Rectangle){ 760, 400, 380, 52 },"Resume", 17, menu_hover_sound, menu_click_sound,is_sound_on,160)){
                            
                paused = false;
            }

            if (menu_button((Rectangle){ 760, 470, 380, 52 },"Restart", 18, menu_hover_sound, menu_click_sound,is_sound_on,160)){
                
                paused = false;
                goto restart;
                
            }

            if (menu_button((Rectangle){ 760, 540, 380, 52 },"Main Menu", 21, menu_hover_sound, menu_click_sound,is_sound_on,160)){
                            
                return_to_menu = true;
                
            }

            if (menu_button((Rectangle){ 760, 610, 380, 52 },"Quit", 19, menu_hover_sound, menu_click_sound,is_sound_on,160)){
                            
                quit_requested = true;
            }

            DrawText("Press ESC to resume", 837, 690, 20, LIGHTGRAY);
        }

        
        if (over || level_complete) {
            nextSpeed=( Vector2){0,0};
            DrawRectangle(700, 560, 500, 270, Fade(BLACK, 0.85f));
            DrawRectangleLinesEx((Rectangle){ 700, 560, 500, 270 }, 2.0f, YELLOW);
            DrawText(over ? "Game Over" : "Level Complete", 855, 580, 34,over ? RED : GREEN);
                     

            
            if (menu_button((Rectangle){ 760, 635, 380, 48 }, "Restart",15,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                goto restart;
            }
            if (menu_button((Rectangle){ 760, 695, 380, 48 }, "Menu",16,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                return_to_menu = true;
            }
            if (menu_button((Rectangle){ 760, 755, 380, 48 }, "Quit",22,menu_hover_sound,menu_click_sound,is_sound_on,160)) {
                quit_requested = true;
            }

        }
   

        EndDrawing();

        if (quit_requested || return_to_menu) {
            break;
        }


    }
    UnloadTexture(tex_c);
    UnloadTexture(tex_s);
    UnloadTexture(tex_a);
    UnloadTexture(tex_t);
    UnloadTexture(tex_v);
    UnloadTexture(tex_u);
    UnloadTexture(tex_w);
    UnloadTexture(tex_g);
    UnloadTexture(tex_r);
    UnloadTexture(tex_f);
    UnloadTexture(tex_l);
    UnloadTexture(tex_p);

    for(int i=0 ;i < 3; i++){
        UnloadTexture(pac_sprite_up[i]);
        UnloadTexture(pac_sprite_down[i]);
        UnloadTexture(pac_sprite_right[i]);
        UnloadTexture(pac_sprite_left[i]);
    }

    UnloadTexture(eaten_ghost_up);
    UnloadTexture(eaten_ghost_down);
    UnloadTexture(eaten_ghost_right);
    UnloadTexture(eaten_ghost_left);

   UnloadTexture(blinky_rules);
   UnloadTexture(pinky_rules);
   UnloadTexture(inky_rules);
   UnloadTexture(clyde_rules);
    
    for(int i = 0; i < 2; i++){
        UnloadTexture(blinky_up[i]);
        UnloadTexture(blinky_down[i]);
        UnloadTexture(blinky_right[i]);
        UnloadTexture(blinky_left[i]);

        UnloadTexture(pinky_up[i]);
        UnloadTexture(pinky_down[i]);
        UnloadTexture(pinky_right[i]);
        UnloadTexture(pinky_left[i]);

        UnloadTexture(inky_up[i]);
        UnloadTexture(inky_down[i]);
        UnloadTexture(inky_right[i]);
        UnloadTexture(inky_left[i]);

        UnloadTexture(clyde_up[i]);
        UnloadTexture(clyde_down[i]);
        UnloadTexture(clyde_right[i]);
        UnloadTexture(clyde_left[i]);
    }

    for(int i = 0; i < 4; i++){
        UnloadTexture(frightened_ghost[i]);
    }
    
    

    UnloadSound(dot_sound);
    UnloadSound(big_dot_sound);
    UnloadSound(ghost_eaten_sound);
    UnloadSound(pac_eaten_sound);
    UnloadSound(game_over_sound);
    UnloadSound(game_finish_sound);
    
    if (return_to_menu) {
        return_to_menu = false;
        goto main_menu;
    }

    CloseAudioDevice();
    CloseWindow();
}
