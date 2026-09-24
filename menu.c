#include "raylib.h"
#include "menu.h"
#include <stdio.h>
#include <string.h>


bool menu_button(Rectangle bounds, const char *label, int button_id, Sound hover_sound, Sound click_sound,bool is_sound_on,int side)
{
    
    static bool was_hovered[32] = { false };
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    

    if(hovered && !was_hovered[button_id]){
        if(is_sound_on)
        PlaySound(hover_sound);
        
    }

    was_hovered[button_id] = hovered;

    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if(clicked){
        if(is_sound_on)
        PlaySound(click_sound);
    }
    DrawRectangleRec(bounds, hovered ? DARKBLUE : BLUE);
    DrawRectangleLinesEx(bounds, 2.0f, SKYBLUE);
    DrawText(label, (int)(bounds.x + side), (int)(bounds.y + 12), 28, RAYWHITE);
    return clicked;
}

static bool IsValidDifficulty(int difficulty)
{
    return difficulty >= 0 && difficulty < DIFFICULTY_COUNT;
}

/* Keep only the best ten records for each difficulty. */
static void TrimLeaderboard(LeaderboardEntry entries[], int *count)
{
    LeaderboardEntry kept[MAX_LEADERBOARD_ENTRIES];
    int kept_per_difficulty[DIFFICULTY_COUNT] = { 0 };
    int kept_count = 0;

    SortLeaderboard(entries, *count);

    for (int i = 0; i < *count; i++) {
        int difficulty = entries[i].difficulty;
        if (IsValidDifficulty(difficulty) && kept_per_difficulty[difficulty] < MAX_LEADERBOARD_ENTRIES_PER_DIFFICULTY){
            
            kept[kept_count++] = entries[i];
            kept_per_difficulty[difficulty]++;
        }
    }

    memcpy(entries, kept, sizeof(LeaderboardEntry) * kept_count);
    *count = kept_count;
    SortLeaderboard(entries, *count);
}

static void InsertLeaderboardEntry(LeaderboardEntry entries[], int *count, LeaderboardEntry entry){
    if (!IsValidDifficulty(entry.difficulty)) return;

    if (*count < MAX_LEADERBOARD_ENTRIES) {
        entries[(*count)++] = entry;
        TrimLeaderboard(entries, count);
        return;
    }

    /* The board is full: replace only the weakest entry in this difficulty. */
    int worst = -1;
    for (int i = 0; i < *count; i++) {
        if (entries[i].difficulty != entry.difficulty) continue;
        if (worst == -1 || entries[i].score < entries[worst].score || (entries[i].score == entries[worst].score && entries[i].seconds > entries[worst].seconds)){
             
            worst = i;
        }
    }

    if (worst != -1 && (entry.score > entries[worst].score || (entry.score == entries[worst].score && entry.seconds < entries[worst].seconds))){
        
        entries[worst] = entry;
        TrimLeaderboard(entries, count);
    }
}

int LoadLeaderboard(LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES])
{
    FILE *file = fopen("leaderboard.txt", "r");
    int count = 0;
    LeaderboardEntry entry;

    if (file == NULL) 
    return 0;

    while (fscanf(file, "%15s %d %d %d", entry.name, &entry.score, &entry.seconds, &entry.difficulty) == 4){
                  
        InsertLeaderboardEntry(entries, &count, entry);
    }

    fclose(file);
    return count;
}

void SortLeaderboard(LeaderboardEntry entries[], int count)
{
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            bool swap = entries[j].score > entries[i].score || (entries[j].score == entries[i].score && entries[j].seconds < entries[i].seconds);
                
                 
            if (swap) {
                LeaderboardEntry temp = entries[i];
                entries[i] = entries[j];
                entries[j] = temp;
            }
        }
    }
}

void SaveLeaderboard(LeaderboardEntry entries[], int *count,const char *name, int score, int seconds,int difficulty){
    LeaderboardEntry entry = { "", score, seconds, difficulty };
    strncpy(entry.name, name, MAX_PLAYER_NAME);
    entry.name[MAX_PLAYER_NAME] = '\0';
    InsertLeaderboardEntry(entries, count, entry);

    FILE *file = fopen("leaderboard.txt", "w");
    if (file == NULL) return;

    for (int i = 0; i < *count; i++) {
        fprintf(file, "%s %d %d %d\n", entries[i].name, entries[i].score, entries[i].seconds, entries[i].difficulty);
               
    }
    fclose(file);
}
