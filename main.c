#include "raylib.h"
#include "raymath.h"
#include "collision.h"
#include <stddef.h>
#include "ghost.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define gheight 950
#define gwidth 1900
#define wtiles 28
#define htiles 31
#define main_speed 110

#define MAX_PLAYER_NAME 20
#define MAX_LEADERBOARD_ENTRIES 15

typedef enum {
    MENU_MAIN,
    MENU_NAME_INPUT,
    MENU_DIFFICULTY,
    MENU_LEADERBOARD,
    MENU_RULES,
    MENU_ABOUT,
    MENU_QUIT
} MenuScreen;

typedef struct {
    char name[MAX_PLAYER_NAME + 1];
    int score;
    int seconds;
    int difficulty;
} LeaderboardEntry;



void ghost_sprite(Texture *g_sprite,int n, char *address){
    for(int i=0; i<n; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\%s_%d.png", address,i+1);
        g_sprite[i] = LoadTexture(path);
    }
}

void draw_ghost_sprite(Texture *g_sprite, ghost g, int frame){
    
    DrawTexture(g_sprite[frame],(int)g.position.x,(int)g.position.y,WHITE);

}

void draw_eaten_ghost_sprite(Texture sprite_up,Texture sprite_down, Texture sprite_right, Texture sprite_left,ghost g){
    if(g.dir == up){
        DrawTexture(sprite_up,g.position.x, g.position.y , WHITE);
    }
    else if(g.dir == down){
        DrawTexture(sprite_down,g.position.x, g.position.y , WHITE);
    }
    else if(g.dir == right){
        DrawTexture(sprite_right,g.position.x, g.position.y , WHITE);
    }
    else if(g.dir == left){
        DrawTexture(sprite_left,g.position.x, g.position.y , WHITE);
    }
}



static bool menu_button(Rectangle bounds, const char *label)
{
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRec(bounds, hovered ? DARKBLUE : BLUE);
    DrawRectangleLinesEx(bounds, 2.0f, SKYBLUE);
    DrawText(label, (int)(bounds.x + 22), (int)(bounds.y + 12), 28, RAYWHITE);
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static int LoadLeaderboard(LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES])
{
    FILE *file = fopen("leaderboard.txt", "r");
    int count = 0;

    if (file == NULL) return 0;

    while (count < MAX_LEADERBOARD_ENTRIES && fscanf(file, "%20s %d %d %d", entries[count].name,&entries[count].score, &entries[count].seconds,&entries[count].difficulty) == 4) {     
                  
        count++;
    }

    fclose(file);
    return count;
}

static void SortLeaderboard(LeaderboardEntry entries[], int count)
{
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            bool swap = entries[j].score > entries[i].score ||
                (entries[j].score == entries[i].score &&
                 entries[j].seconds < entries[i].seconds);
            if (swap) {
                LeaderboardEntry temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
}

static void SaveLeaderboard(LeaderboardEntry entries[], int *count,const char *name, int score, int seconds,int difficulty){
    if (*count < MAX_LEADERBOARD_ENTRIES) {
        strncpy(entries[*count].name, name, MAX_PLAYER_NAME);
        entries[*count].name[MAX_PLAYER_NAME] = '\0';
        entries[*count].score = score;
        entries[*count].seconds = seconds;
        entries[*count].difficulty = difficulty;
        (*count)++;
    }
    else {
        entries[MAX_LEADERBOARD_ENTRIES - 1] = (LeaderboardEntry){"", score, seconds, difficulty};
        strncpy(entries[MAX_LEADERBOARD_ENTRIES - 1].name, name, MAX_PLAYER_NAME);
        entries[MAX_LEADERBOARD_ENTRIES - 1].name[MAX_PLAYER_NAME] = '\0';
    }

    SortLeaderboard(entries, *count);

    FILE *file = fopen("leaderboard.txt", "w");
    if (file == NULL) return;

    for (int i = 0; i < *count; i++) {
        fprintf(file, "%s %d %d %d\n", entries[i].name, entries[i].score, entries[i].seconds, entries[i].difficulty);
               
    }
    fclose(file);
}

static const char *DifficultyName(int difficulty)
{
    if (difficulty == 0)
    return "Easy";
    if (difficulty == 2)
    return "Hard";

    return "Normal";
}

/* Returns true when value can be safely snapped to a maze tile line. */
static bool near_tile_line(float value, float origin, float tolerance)
{
    float line = origin + roundf((value - origin) / 26.0f) * 26.0f;
    return fabsf(value - line) <= tolerance;
}

static float nearest_tile_line(float value, float origin)
{
    return origin + roundf((value - origin) / 26.0f) * 26.0f;
}



const char map_easy[htiles][wtiles+1]=
    {"cssssssssssssssssssssssssssa",
     "tddddddddddddvvddddddddddddt",
     "tduwwgduwwwgdvvduwwwgduwwgdt",
     "tbveevdveeevdvvdveeevdveevbt",
     "tdpwwrdpwwwrdprdpwwwrdpwwrdt",
     "tddddddddddddddddddddddddddt",
     "tduwwgdugduwwwwwwgdugduwwgdt",
     "tdpwwrdvvdpwwwwwwrdvvdpwwrdt",
     "tddddddvvddddvvddddvvddddddt",
     "lssssadvpwwgevveuwwrvdcssssf",
     "eeeeetdvuwwreprepwwgvdteeeee",
     "eeeeetdvveeeeeeeeeevvdteeeee",
     "eeeeetdvvecssiissaevvdteeeee",
     "sssssfdpreteeeeeeteprdlsssss",
     "eeeeeedeeeteeeeeeteeedeeeeee",
     "sssssadugeteeeeeeteugdcsssss",
     "eeeeetdvvelssssssfevvdteeeee",
     "eeeeetdvveeeeeeeeeevvdteeeee",
     "eeeeetdvveuwwwwwwgevvdteeeee",
     "cssssfdprepwwguwwreprdlssssa",
     "tddddddddddddvvddddddddddddt",
     "tduwwgduwwwgdvvduwwwgduwwgdt",
     "tdpwgvdpwwwrdprdpwwwrdvuwrdt",
     "tbddvvddddddddddddddddvvddbt",
     "twgdvvdugduwwwwwwgdugdvvduwt",
     "twrdprdvvdpwwguwwrdvvdprdpwt",
     "tddddddvvddddvvddddvvddddddt",
     "tduwwwwrpwwgdvvduwwrpwwwwgdt",
     "tdpwwwwwwwwrdprdpwwwwwwwwrdt",
     "tddddddddddddddddddddddddddt",
     "lssssssssssssssssssssssssssf"
    };

const char map_normal[htiles][wtiles+1]=
    {"cssssssssssssssssssssssssssa",//1
     "tdddddddddvvddddvvdddddddddt",//2
     "tbuwwwwwgdvvdugdvvduwwwwwgbt",//3
     "tdvuwwwwrdprdvvdprdpwwwwgvdt",//4
     "tdvvdddddddddvvdddddddddvvdt",//5
     "tdprdugduwwgdvvduwwgdugdprdt",//6
     "tddddvvdveevdvvdveevdvvddddt",//7done
     "twwgdvvdpwwrdprdpwwrdvvduwwt",//8done
     "twwrdvvddddddddddddddvvdpwwt",//9done
     "tddddvpwgduwwwwwwgduwrvddddt",//10done
     "tdugdpwwrdpwwwwwwrdpwwrdugdt",//11done
     "tdvvddddddddddddddddddddvvdt",//12done
     "tdvpwgdugdcssiissadugduwrvdt",//13done
     "tdpwwrdvvdteeeeeetdvvdpwwrdt",//14done
     "eddddddvvdteeeeeetdvvdddddde",//15done
     "tdugduwrvdteeeeeetdvpwgdugdt",//16done
     "tdvvdpwwrdlssssssfdpwwrdvvdt",//17done
     "tdvvddddddddddddddddddddvvdt",//18done
     "tdvpwgduwwwgdugduwwwgduwrvdt",//19done
     "tdpwwrdvuwwrdvvdpwwgvdpwwrdt",//20done
     "tddddddvvddddvvddddvvddddddt",//21done
     "twgdugdvvduwwrpwwgdvvdugduwt",//22done
     "twrdvvdprdpwwwwwwrdprdvvdpwt",//23done
     "tbddvvddddddddddddddddvvddbt",//24done
     "tduwrvduwwwgdugduwwwgdvpwgdt",//25done
     "tdpwwrdvuwwrdvvdpwwgvdpwwrdt",//26done
     "tddddddvvddddvvddddvvddddddt",//27done
     "tduwwgdvvduwwrpwwgdvvduwwgdt",//28done
     "tdpwwrdvvdpwwwwwwrdvvdpwwrdt",//29
     "tddddddvvddddddddddvvddddddt",//30
     "lssssssssssssssssssssssssssf"//31
    };

const char map_hard[htiles][wtiles+1]=
    {"cssssssssssssssssssssssssssa",//1
     "tddddddddddddddddddddddddddt",//2
     "tbugduwwgduwwwwwwgduwwgdugbt",//3done
     "tdvvdveevdvuwwwwgvdveevdvvdt",//4done
     "tdvvdpwwrdvvddddvvdpwwrdvvdt",//5done
     "tdvvddddddvvdugdvvddddddvvdt",//6done
     "tdvpwgdugdvvdvvdvvdugduwrvdt",//7done
     "tdpwwrdvvdprdvvdprdvvdpwwrdt",//8done
     "tddddddvvddddvvddddvvddddddt",//9done
     "lsaduwwrpwwgdvvduwwrpwwgdcsf",//10done
     "eetdpwwguwwrdprdpwwguwwrdtee",//11done
     "eetddddvvddddddddddvvddddtee",//12done
     "csfdugdvvdcssiissadvvdugdpsa",//13done
     "tdddvvdprdteeeeeetdprdvvdddt",//14done
     "eduwrvddddteeeeeetddddvpwgde",//15done
     "tdpwgvdugdteeeeeetdugdvuwrdt",//16done
     "tdddvvdvvdlssssssfdvvdvvdddt",//17done
     "lsadprdvvddddddddddvvdprdcsf",//18done
     "eetddddvpwwgdugduwwrvddddtee",//19done
     "eetdugdpwwwrdvvdpwwwrdugdtee",//20done
     "eetdvvdddddddvvdddddddvvdtee",//21done
     "eetdvpwwgdugdvvdugduwwrvdtee",//22done
     "csfdpwwwrdvvdprdvvdpwwwrdlsa",//23done
     "tbddddddddvvddddvvddddddddbt",//24done
     "tduwwgdugdvpwwwwrvdugduwwgdt",//25done
     "tdvuwrdvvdpwwwwwwrdvvdpwgvdt",//26done
     "tdvvdddvvddddddddddvvdddvvdt",//27done
     "tdvvduwrpwwgdugduwwrpwgdvvdt",//28done
     "tdprdpwwwwwrdvvdpwwwwwrdprdt",//29done
     "tddddddddddddvvddddddddddddt",//30done
     "lssssssssssssssssssssssssssf"
    };   
//map er array
char map[htiles][wtiles+1];
//pacman er first position
Rectangle pacman= {976, 670,24,24}; 
    
    

int main(){
    
    InitWindow(gwidth, gheight, "Pacman");
    SetTargetFPS(60);
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
    

    main_menu:;
    MenuScreen menu_screen = MENU_MAIN;
    LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES] = { 0 };
    int leaderboard_count = LoadLeaderboard(leaderboard);
    SortLeaderboard(leaderboard, leaderboard_count);
    char player_name[MAX_PLAYER_NAME + 1] = "";
    int player_name_length = 0;
    int selected_difficulty = 1;
    int leaderboard_difficulty = 0;
    bool start_game = false;
    

    
    

    while (!WindowShouldClose() && !start_game) {
        BeginDrawing();
        ClearBackground((Color){ 8, 10, 28, 255 });
        

        

        DrawText("PACMAN", 820, 100, 64, YELLOW);

        if (menu_screen == MENU_MAIN) {
            DrawText("Main Menu", 860, 190, 34, RAYWHITE);

            if (menu_button((Rectangle){ 760, 255, 380, 60 }, "Play")) {
                menu_screen = MENU_NAME_INPUT;
            }
            
            if (menu_button((Rectangle){ 760, 330, 380, 60 }, "Leaderboard")) {
                menu_screen = MENU_LEADERBOARD;
            }
            if (menu_button((Rectangle){ 760, 405, 380, 60 }, "Game Rules")) {
                menu_screen = MENU_RULES;
            }
            if (menu_button((Rectangle){ 760, 480, 380, 60 }, "About Us")) {
                menu_screen = MENU_ABOUT;
            }
            if (menu_button((Rectangle){ 760, 555, 380, 60 }, "Quit")) {
                EndDrawing();
                CloseAudioDevice();
                CloseWindow();
                return 0;
            }
        }
        else if (menu_screen == MENU_NAME_INPUT) {
            DrawText("Enter your name", 765, 260, 36, RAYWHITE);
            DrawRectangle(700, 330, 500, 58, DARKBLUE);
            DrawRectangleLinesEx((Rectangle){ 700, 330, 500, 58 }, 2.0f, SKYBLUE);
            DrawText(player_name, 720, 345, 30, WHITE);
            DrawText("Press Enter to continue", 770, 430, 24, LIGHTGRAY);

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
                menu_screen = MENU_DIFFICULTY;
            }
            if (IsKeyPressed(KEY_ESCAPE))
                menu_screen = MENU_MAIN;
        }
        else if (menu_screen == MENU_DIFFICULTY) {
            DrawText("Choose difficulty", 735, 220, 36, RAYWHITE);
            if (menu_button((Rectangle){ 760, 300, 380, 60 }, "Easy")) {
                selected_difficulty = 0;
                start_game = true;
            }
            if (menu_button((Rectangle){ 760, 380, 380, 60 }, "Normal")) {
                selected_difficulty = 1;
                start_game = true;
            }
            if (menu_button((Rectangle){ 760, 460, 380, 60 }, "Hard")) {
                selected_difficulty = 2;
                start_game = true;
            }
            if (menu_button((Rectangle){ 760, 560, 380, 60 }, "Back")) {
                menu_screen = MENU_MAIN;
            }
        }
        else if (menu_screen == MENU_LEADERBOARD) {
            DrawText("Leaderboard", 770, 185, 40, YELLOW);
            if (menu_button((Rectangle){ 590, 255, 210, 55 }, "Easy")) {
                leaderboard_difficulty = 0;
            }
            if (menu_button((Rectangle){ 845, 255, 210, 55 }, "Normal")) {
                leaderboard_difficulty = 1;
            }
            if (menu_button((Rectangle){ 1100, 255, 210, 55 }, "Hard")) {
                leaderboard_difficulty = 2;
            }

            DrawText(TextFormat("%s leaderboard", DifficultyName(leaderboard_difficulty)),760, 335, 28, RAYWHITE);
                     
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
            if (menu_button((Rectangle){ 760, 850, 380, 60 }, "Back")) {
                menu_screen = MENU_MAIN;
            }
        }
        else if (menu_screen == MENU_RULES) {
            DrawText("Game Rules", 838, 195, 40, YELLOW);
            DrawText("-Eat every dot to finish the level.", 670, 290, 28, RAYWHITE);
            DrawText("-Avoid ghosts unless they are frightened.", 670, 335, 28, RAYWHITE);
            DrawText("-You can eat ghosts during their frightened phase",670,380,28,RAYWHITE);
            DrawText("-Power pellets make ghosts frightened for a short time.", 670, 425, 28, RAYWHITE);
            DrawText("-Use arrow keys or W A S D to move.", 670, 470, 28, RAYWHITE);
            if (menu_button((Rectangle){ 760, 600, 380, 60 }, "Back")) {
                menu_screen = MENU_MAIN;
            }
        }
        else if (menu_screen == MENU_ABOUT) {
            DrawText("About Us", 860, 220, 40, YELLOW);
            
            const char *about_text ="We are presenting our Level-1/Term-1 (L1T1) group project. We have recreated one of\n\n"
                "the most popular and OG game from 1980 till now uaing raylib functions. Presenting you the PACMAN!\n\n"
                "The game was developed by me Arin Kumar Chanda and my project partner Ahmed Muhaymin under the supervision\n\n"
                "of our respectful Dr. Ch. Md. Rakin Haider Sir. His supervision and guidance helped us to develop the game\n\n"
                "properly. We have added some exciting new maps and challenges. Let's see who can avoid those unpredictable\n\n"
                "ghosts and complete the maps fast with highest points.Hope you will enjoy the game.\n\n\n\n"
                "Thank you!\n";
                

            DrawText(about_text, 420, 300, 22, RAYWHITE);
            
            if (menu_button((Rectangle){ 760, 750, 380, 60 }, "Back")) {
                menu_screen = MENU_MAIN;
            }
        }
        

        EndDrawing();
    }

    if (!start_game) {
        CloseAudioDevice();
        CloseWindow();
        return 0;
    }

    if (selected_difficulty == 0) {
        memcpy(map, map_easy, sizeof(map));
    } 
    else if (selected_difficulty == 2) {
        memcpy(map, map_hard, sizeof(map));
    } 
    else {
        memcpy(map, map_normal, sizeof(map));
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
    int time=5;
    

    
    
    //w=horizontal single wall,v=vertical single wall, s=horizontal double wall, t=vertical double wall,d=dot,e=empty space
    //g = top right single corner, u = top left single corner
    //r= bottom right single corner, p = bottom left single corner
    //a = top right double corner, c = top left double corner
    //f= bottom right double corner, l= bottom left double corner
    
    //wall sprite er jonno
    Texture tex_c = LoadTexture("assets\\sprite\\WALL_DOUBLE_CORNER_TL.png");
    Texture tex_s = LoadTexture("assets\\sprite\\WALL_DOUBLE_H.png");
    Texture tex_a = LoadTexture("assets\\sprite\\WALL_DOUBLE_CORNER_TR.png");
    Texture tex_t = LoadTexture("assets\\sprite\\WALL_DOUBLE_V.png");
    Texture tex_v = LoadTexture("assets\\sprite\\WALL_SINGLE_V.png");
    Texture tex_u = LoadTexture("assets\\sprite\\WALL_SINGLE_CORNER_TL.png");
    Texture tex_w = LoadTexture("assets\\sprite\\WALL_SINGLE_H.png");
    Texture tex_g = LoadTexture("assets\\sprite\\WALL_SINGLE_CORNER_TR.png");
    Texture tex_r = LoadTexture("assets\\sprite\\WALL_SINGLE_CORNER_BR.png");
    Texture tex_f = LoadTexture("assets\\sprite\\WALL_DOUBLE_CORNER_BR.png");
    Texture tex_l = LoadTexture("assets\\sprite\\WALL_DOUBLE_CORNER_BL.png");
    Texture tex_p = LoadTexture("assets\\sprite\\WALL_SINGLE_CORNER_BL.png");
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


    /*for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\pinky_left_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }
    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\inky_left_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }
    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\blinky_left_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }
    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\clyde_left_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }

    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\clyde_right_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }
    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\inky_right_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }
    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\blinky_right_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }
    for(int i=0; i<3; i++){
        char path[100];
        sprintf(path, "assets\\ghost_sprite\\pinky_right_%d.png", i+1);
        pac_sprite_left[i] = LoadTexture(path);
    }*/
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
        float dt= GetFrameTime();
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
        bool at_turn_line = !perpendicular_turn || (moving_horizontally ? near_tile_line(position.x, 586.0f, snap_tolerance) : near_tile_line(position.y, 72.0f, snap_tolerance));
                                 
        if (at_turn_line) {
            Vector2 turn_position = position;

            if (perpendicular_turn) {
                if (moving_horizontally) {
                    turn_position.x = nearest_tile_line(position.x, 586.0f);
                } else {
                    turn_position.y = nearest_tile_line(position.y, 72.0f);
                }
            }

            Rectangle turnBox = {
                turn_position.x + nextSpeed.x * dt + shrink / 2,
                turn_position.y + nextSpeed.y * dt + shrink / 2,
                26 - shrink,
                26 - shrink
            };

            if (!collision(turnBox, map)) {
                position = turn_position;
                speed = nextSpeed;
            }
        }

        Rectangle collisionBox = {
            position.x + speed.x * dt + shrink / 2,
            position.y + speed.y * dt + shrink / 2,
            26 - shrink,
            26 - shrink
        };
        //pacman er position update
        
        
        if(!collision(collisionBox, map)){
            
            
            position.x += speed.x*dt;
            position.y += speed.y*dt;
            
        }
        else {
            speed = (Vector2){0, 0};
        }
        
        pacman= (Rectangle){position.x,position.y,24,24};
        //dot collection
            
        //pacman er center khuje tiles number ber kora
        int tile_j = (int)((position.x + 12 - 586) / 26);
        int tile_i = (int)((position.y + 12 - 72) / 26);

        if (tile_i >= 0 && tile_i < htiles && tile_j >= 0 && tile_j < wtiles) {
            if (map[tile_i][tile_j] == 'd') {
                map[tile_i][tile_j] = 'e'; 
                score += 10;
                PlaySound(dot_sound);
            } 
            else if (map[tile_i][tile_j] == 'b') {
                map[tile_i][tile_j] = 'e'; 
                score += 50;
                PlaySound(big_dot_sound);

                phase=frightened;
                flip_dir(&blinky_ghost);
                flip_dir(&pinky_ghost);
                flip_dir(&inky_ghost);
                flip_dir(&clyde_ghost);
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
        
        Rectangle ghost_rec_blinky=g_rec(&blinky_ghost,blinky);
        Rectangle ghost_rec_pinky=g_rec(&pinky_ghost,pinky);
        Rectangle ghost_rec_inky=g_rec(&inky_ghost,inky);
        Rectangle ghost_rec_clyde=g_rec(&clyde_ghost,clyde);
        
        
        
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
                    PlaySound(ghost_eaten_sound);
                    score += 200;
                }

                if (CheckCollisionRecs(pacman, ghost_rec_pinky) && !pinky_ghost.eaten && !pinky_ghost.ignore_frightened) {
                    
                    pinky_ghost.eaten = true;
                    PlaySound(ghost_eaten_sound);
                    score += 200;
                }
                if (CheckCollisionRecs(pacman, ghost_rec_inky) && !inky_ghost.eaten && !inky_ghost.ignore_frightened) {
                    
                    inky_ghost.eaten = true;
                    PlaySound(ghost_eaten_sound);
                    score += 200;
                }
                if (CheckCollisionRecs(pacman, ghost_rec_clyde) && !clyde_ghost.eaten && !clyde_ghost.ignore_frightened) {
                    
                    clyde_ghost.eaten = true;
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
            /*if (phase == eaten) {
                tile ghost_tile_blinky = tiles_no(blinky_ghost.position);
                tile ghost_tile_pinky = tiles_no(pinky_ghost.position);
                tile ghost_tile_inky = tiles_no(inky_ghost.position);
                tile ghost_tile_clyde = tiles_no(clyde_ghost.position);

                Vector2 blinky_tile_position =pixel(ghost_tile_blinky);
                Vector2 pinky_tile_position =pixel(ghost_tile_pinky);
                Vector2 inky_tile_position =pixel(ghost_tile_inky);
                Vector2 clyde_tile_position =pixel(ghost_tile_clyde);
                
                bool at_tile_blinky = fabsf(blinky_ghost.position.x - blinky_tile_position.x) < 1.0f && fabsf(blinky_ghost.position.y - blinky_tile_position.y) < 1.0f;

                bool at_tile_pinky = fabsf(pinky_ghost.position.x - pinky_tile_position.x) < 1.0f && fabsf(pinky_ghost.position.y - pinky_tile_position.y) < 1.0f;

                bool at_tile_inky = fabsf(inky_ghost.position.x - inky_tile_position.x) < 1.0f && fabsf(inky_ghost.position.y - inky_tile_position.y) < 1.0f;

                bool at_tile_clyde = fabsf(clyde_ghost.position.x - clyde_tile_position.x) < 1.0f && fabsf(clyde_ghost.position.y - clyde_tile_position.y) < 1.0f;
                    

                if (at_tile_blinky) {
                    eaten_phase_blinky(&blinky_ghost); 
                }

                if (at_tile_pinky) {
                    eaten_phase_pinky(&pinky_ghost); 
                }

                if (at_tile_inky) {
                    eaten_phase_inky(&inky_ghost); 
                }
                if (at_tile_clyde) {
                    eaten_phase_clyde(&clyde_ghost); 
                }
                if (ghost_tile_blinky.row == 11 && ghost_tile_blinky.col == 13 ) {
                    phase = chase;      
                    chase_time = 0.0f;  
                    scattered_time = 0.0f; 
                }
                if (ghost_tile_pinky.row == 11 && ghost_tile_pinky.col == 12) {
                    phase = chase;      
                    chase_time = 0.0f;  
                    scattered_time = 0.0f; 
                }
                if (ghost_tile_inky.row == 11 && ghost_tile_inky.col == 15) {
                    phase = chase;      
                    chase_time = 0.0f;  
                    scattered_time = 0.0f; 
                }
                if (ghost_tile_clyde.row == 11 && ghost_tile_clyde.col == 14) {
                    phase = chase;      
                    chase_time = 0.0f;  
                    scattered_time = 0.0f; 
                }
                
            }*/
        
        
            if (blinky_ghost.eaten) {
                tile t = tiles_no(blinky_ghost.position);
                Vector2 tile_pos = pixel(t);

                bool at_tile = fabsf(blinky_ghost.position.x - tile_pos.x) < 1.0f && fabsf(blinky_ghost.position.y - tile_pos.y) < 1.0f;
      
                if (at_tile) {
                    blinky_ghost.position = tile_pos;
                    eaten_phase_blinky(&blinky_ghost);
                }

                movement(&blinky_ghost, ghost_eaten_speed * ghost_speed_multiplier);
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

                bool at_tile =
                    fabsf(pinky_ghost.position.x - tile_pos.x) < 1.0f && fabsf(pinky_ghost.position.y - tile_pos.y) < 1.0f;
                    

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

                bool at_tile =
                    fabsf(inky_ghost.position.x - tile_pos.x) < 1.0f && fabsf(inky_ghost.position.y - tile_pos.y) < 1.0f;
                    

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

                bool at_tile =
                    fabsf(clyde_ghost.position.x - tile_pos.x) < 1.0f && fabsf(clyde_ghost.position.y - tile_pos.y) < 1.0f;
                    

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
                
            

                Rectangle dest = {586+j*26, 72+i*26, 26, 26};
                if (wall_map != NULL) {
                    Rectangle source = {1, 1, (float)wall_map->width - 2, (float)wall_map->height - 2};
                    DrawTexturePro(*wall_map, source, dest, (Vector2){0.0f, 0.0f}, 0.0f, WHITE);
                
                
            }
        
        
        }
    }

        
        for(int i=0; i<htiles;i++){
            for(int j=0; j<wtiles; j++){
                
                if(map[i][j]=='d'){
                    DrawRectangle(586+j*26, 72+i*26, 26, 26, BLACK);
                    DrawCircle(586+j*26+13, 72+i*26+13, 3, ORANGE);
                }
                else if(map[i][j]=='e'){
                    DrawRectangle(586+j*26, 72+i*26, 26, 26, BLACK);
                }
                else if(map[i][j]=='b'){
                    DrawRectangle(586+j*26, 72+i*26, 26, 26, BLACK);
                    DrawCircle(586+j*26+13, 72+i*26+13, 8, YELLOW);
                }
                else if(map[i][j]=='i'){
                    DrawRectangle(586+j*26, 72+i*26, 26, 26, PINK);
                    
                }



            }
        }
        bool hit_dangerous_ghost =(CheckCollisionRecs(pacman, ghost_rec_blinky) && !blinky_ghost.eaten) || (CheckCollisionRecs(pacman, ghost_rec_pinky)  && !pinky_ghost.eaten) || (CheckCollisionRecs(pacman, ghost_rec_inky)  && !inky_ghost.eaten) || (CheckCollisionRecs(pacman, ghost_rec_clyde)  && !clyde_ghost.eaten);
    
    
        
        if(!over && (phase == chase || phase == scattered) && hit_dangerous_ghost){
            if(CheckCollisionRecs(pacman, ghost_rec_blinky) || CheckCollisionRecs(pacman, ghost_rec_pinky) || CheckCollisionRecs(pacman, ghost_rec_inky) || CheckCollisionRecs(pacman, ghost_rec_clyde)){
                
                life--;
                if(life>0){
                    PlaySound(pac_eaten_sound);
                    goto here;
                }
                else if(life == 0){

                    over=true;
                    PlaySound(game_over_sound);
                    if (!leaderboard_saved) {
                        SaveLeaderboard(leaderboard, &leaderboard_count, player_name,
                                        score, minute * 60 + second,
                                        selected_difficulty);
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
            PlaySound(game_finish_sound);
        }
        if (level_complete) {
            DrawText("You passed the level!", 800, 50, 26, GREEN);
            nextSpeed=(Vector2){0.0f,0.0f};
            DrawText(TextFormat("Player: %s   Difficulty: %s", player_name,DifficultyName(selected_difficulty)),760, 82, 22, RAYWHITE);
                
        }
        
        
        if(over){
            DrawText("Game Over", 898, 514,26, RED);
            nextSpeed=( Vector2){0,0};
            
        }

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
        

        
        
        DrawText(TextFormat("SCORE: %d", score), 586, 30, 30, WHITE);
        DrawText("LIFE:",600,895,26,WHITE);
        Texture2D life_sprite=LoadTexture("assets\\sprite\\life_sprite.png");


        
        if (!over && !level_complete) {
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
            
        
        DrawText(TextFormat("TIME: 0%d:%d", minute, second),1200,30,26,WHITE);
        
        
        
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

        
        if (over || level_complete) {
            DrawRectangle(700, 560, 500, 250, Fade(BLACK, 0.85f));
            DrawRectangleLinesEx((Rectangle){ 700, 560, 500, 250 }, 2.0f, YELLOW);
            DrawText(over ? "Game Over" : "Level Complete", 820, 580, 34,over ? RED : GREEN);
                     

            
            if (menu_button((Rectangle){ 760, 695, 380, 48 }, "Menu")) {
                return_to_menu = true;
            }
            if (menu_button((Rectangle){ 760, 755, 380, 48 }, "Quit")) {
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
