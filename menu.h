#ifndef MENU_H
#define MENU_H

#define MAX_PLAYER_NAME 15
#define DIFFICULTY_COUNT 3
#define MAX_LEADERBOARD_ENTRIES_PER_DIFFICULTY 10
#define MAX_LEADERBOARD_ENTRIES (DIFFICULTY_COUNT * MAX_LEADERBOARD_ENTRIES_PER_DIFFICULTY)

typedef enum {
    menu_main,
    menu_name_input,
    menu_difficulty,
    menu_leaderboard,
    menu_rules,
    menu_about,
    menu_quit,
    menu_sound,
    menu_control,
    menu_ghost,
    menu_phase
} MenuScreen;

typedef struct {
    char name[MAX_PLAYER_NAME + 1];
    int score;
    int seconds;
    int difficulty;
} LeaderboardEntry;

bool menu_button(Rectangle bounds, const char *label, int button_id, Sound hover_sound, Sound click_sound,bool is_sound_on,int side);
int LoadLeaderboard(LeaderboardEntry entries[MAX_LEADERBOARD_ENTRIES]);
void SortLeaderboard(LeaderboardEntry entries[], int count);
void SaveLeaderboard(LeaderboardEntry entries[], int *count,const char *name, int score, int seconds,int difficulty);

#endif
