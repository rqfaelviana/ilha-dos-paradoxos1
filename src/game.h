#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// =============================================
// CONSTANTES
// =============================================
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720
#define TILE_SIZE       32
#define MAP_WIDTH       40
#define MAP_HEIGHT      30
#define MAX_NPCS        10
#define MAX_DIALOGUE    20
#define MAX_CLUES       50
#define MAX_DEDUCTIONS  30
#define MAX_TEXT_LEN    256
#define PLAYER_SPEED    2.5f
#define FPS             60

// =============================================
// ENUMS
// =============================================
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_INTRO,
    SCREEN_GAME,
    SCREEN_DIALOGUE,
    SCREEN_DEDUCTION,
    SCREEN_PAUSE,
    SCREEN_CREDITS,
    SCREEN_ENDING
} GameScreen;

typedef enum {
    NPC_VERAZ = 0,
    NPC_FALAZ,
    NPC_PARADOXAL
} NPCType;

typedef enum {
    CLUE_PROPOSITION = 0,
    CLUE_OBSERVATION,
    CLUE_DEDUCTION
} ClueType;

typedef enum {
    TILE_GRASS = 0,
    TILE_WATER,
    TILE_SAND,
    TILE_STONE,
    TILE_TREE,
    TILE_BUILDING,
    TILE_PATH,
    TILE_RUINS,
    TILE_DOOR_CLOSED,
    TILE_DOOR_OPEN
} TileType;

typedef enum {
    WEATHER_CLEAR = 0,
    WEATHER_FOG,
    WEATHER_RAIN,
    WEATHER_STORM
} WeatherType;

typedef enum {
    TIME_DAWN = 0,
    TIME_DAY,
    TIME_DUSK,
    TIME_NIGHT
} TimeOfDay;

// =============================================
// STRUCTS
// =============================================
typedef struct {
    char text[MAX_TEXT_LEN];
    char speaker[64];
    int  next_index;  // -1 = end dialogue
    bool triggers_clue;
    int  clue_id;
} DialogueLine;

typedef struct {
    char      name[64];
    NPCType   type;
    Vector2   pos;
    Color     color;
    int       dialogue_start;
    int       dialogue_count;
    bool      talked_to;
    bool      visible;
    char      symbol[8]; // V, F, P
    // Paradoxal pattern
    int       paradox_counter;
    bool      paradox_truth_now;
    // MELHORIA 5: revelacao progressiva do tipo do NPC
    bool      type_revealed;
} NPC;

typedef struct {
    char      text[MAX_TEXT_LEN];
    char      tag[16];     // [P], [Q], [R]...
    ClueType  type;
    bool      discovered;
    bool      confirmed;
    bool      negated;
    int       npc_source;  // -1 = environment
    Color     color;
} Clue;

typedef struct {
    char premise1[16];   // tag like "P"
    char premise2[16];
    char conclusion[16];
    char operator[8];    // "->", "&&", "||", "!"
    char text[MAX_TEXT_LEN];
    bool unlocked;
} Deduction;

typedef struct {
    int       tiles[MAP_HEIGHT][MAP_WIDTH];
    bool      door_state[MAP_HEIGHT][MAP_WIDTH]; // open/closed
    bool      door_needs_logic[MAP_HEIGHT][MAP_WIDTH];
    char      door_condition[MAP_HEIGHT][MAP_WIDTH][16]; // tag needed
    bool      door_puzzle_solved[MAP_HEIGHT][MAP_WIDTH]; // MELHORIA 1: tabela-verdade concluída
    int       width, height;
    Color     fog_of_war[MAP_HEIGHT][MAP_WIDTH];
    bool      explored[MAP_HEIGHT][MAP_WIDTH];
} GameMap;

typedef struct {
    Vector2   pos;
    Vector2   vel;
    Rectangle bounds;
    float     speed;
    bool      moving;
    int       facing; // 0=down,1=up,2=left,3=right
    int       anim_frame;
    float     anim_timer;
    bool      in_deduction_mode;
    bool      has_lighthouse_code;
    int       lighthouse_fragments;  // 0-3
} Player;

typedef struct {
    float     time_of_day;   // 0.0 - 24.0
    float     time_speed;    // how fast time passes
    WeatherType weather;
    float     fog_alpha;
    float     rain_timer;
    bool      is_night;
    // Rain particles
    Vector2   rain_drops[200];
    float     rain_speed[200];
    // Fog particles  
    Vector2   fog_patches[20];
    float     fog_alpha_individual[20];
} Environment;

typedef struct {
    bool      active;
    char      title[64];
    char      body[512];
    float     timer;
    Color     color;
} Notification;

typedef struct {
    // Menu
    int       menu_selection;
    float     menu_anim;
    bool      has_save;
    
    // Game state
    GameScreen current_screen;
    GameScreen prev_screen;
    
    // Map & world
    GameMap   map;
    Player    player;
    NPC       npcs[MAX_NPCS];
    int       npc_count;
    
    // Dialogue
    int       active_npc;       // -1 = no dialogue
    int       dialogue_line;
    bool      dialogue_typing;
    float     type_timer;
    int       type_char;
    char      typed_text[MAX_TEXT_LEN];
    
    // Clues & Deductions
    Clue      clues[MAX_CLUES];
    int       clue_count;
    Deduction deductions[MAX_DEDUCTIONS];
    int       deduction_count;
    int       selected_clue;
    int       deduction_scroll;
    // MELHORIA 2: sistema de deducao manual
    int       deduce_slot_a;      // indice da 1a pista selecionada (-1 = vazio)
    int       deduce_slot_b;      // indice da 2a pista selecionada (-1 = vazio)
    int       deduce_op;          // 0=AND 1=OR 2=IMPL 3=NEG
    float     deduce_error_timer; // > 0 = piscar erro em vermelho
    bool      deduce_panel_focus; // true = foco no painel de deducao
    
    // Dialogue database
    DialogueLine dialogue_db[100];
    int          dialogue_db_count;
    
    // Environment
    Environment  env;
    
    // UI
    Notification notif;
    float        fade_alpha;
    bool         fading_in;
    bool         fading_out;
    float        screen_shake;
    
    // Camera
    Camera2D  camera;
    
    // Puzzle state
    bool      is_new_game;   // true = novo jogo, nao carrega save
    bool      mystery_solved;
    // MELHORIA 3: estado da contradicao visual
    bool      contradiction_active; // P e nao-P presentes simultaneamente
    bool      lighthouse_activated;
    int       puzzle_step;    // 0=not started, 1=in progress, 2=solved
    bool      fisherman_alibi_checked;
    bool      priestess_alibi_checked;
    
    // Audio (using procedural sound)
    bool      audio_on;
    float     ambient_timer;
    
    // Frame counter
    int       frame;
    float     delta;
    
    // Font
    Font      font_main;
    Font      font_mono;

    // ============================================================
    // MELHORIA 1: Puzzle de Tabela-Verdade para Portas Lógicas
    // ============================================================
    bool  ttable_active;
    int   ttable_door_ty;
    int   ttable_door_tx;
    char  ttable_title[64];
    char  ttable_col_a[12];
    char  ttable_col_b[12];
    char  ttable_col_result[24];
    // rows[i][0]=val_A(1=V,2=F), [1]=val_B, [2]=resposta_correta(1=V,2=F)
    int   ttable_rows[4][3];
    int   ttable_user[4];        // 0=vazio 1=V 2=F
    int   ttable_cursor;         // linha selecionada
    bool  ttable_submitted;
    bool  ttable_result_ok;
    float ttable_feedback_timer;

    // ============================================================
    // MELHORIA 2: Quiz do NPC antes de revelar pista
    // ============================================================
    bool  npc_quiz_active;
    int   npc_quiz_pending_clue; // pista a revelar se acertar
    char  npc_quiz_question[256];
    char  npc_quiz_opts[3][128];
    int   npc_quiz_correct;      // índice da opção correta
    int   npc_quiz_selected;     // -1=sem resposta
    bool  npc_quiz_answered;
    bool  npc_quiz_is_correct;
    float npc_quiz_timer;

    // ============================================================
    // MELHORIA 3: Quiz de validação antes de confirmar dedução
    // ============================================================
    bool  ded_quiz_active;
    char  ded_quiz_pending_result[16];
    int   ded_quiz_pending_ded_idx;
    int   ded_quiz_pending_clue_ci;
    char  ded_quiz_question[256];
    char  ded_quiz_opts[3][128];
    int   ded_quiz_correct;
    int   ded_quiz_selected;
    bool  ded_quiz_answered;
    bool  ded_quiz_is_correct;
    float ded_quiz_timer;
    
    // Textures (procedural)
    Texture2D tex_tiles;
    Texture2D tex_player;
    Texture2D tex_npc;
    
} GameState;

// =============================================
// CORES DO JOGO
// =============================================
#define COL_BG          (Color){10, 12, 20, 255}
#define COL_BG2         (Color){15, 18, 30, 255}
#define COL_GRASS       (Color){34, 55, 35, 255}
#define COL_GRASS2      (Color){28, 48, 30, 255}
#define COL_WATER       (Color){20, 40, 80, 255}
#define COL_WATER2      (Color){15, 32, 68, 255}
#define COL_SAND        (Color){120, 100, 70, 255}
#define COL_STONE       (Color){70, 72, 80, 255}
#define COL_STONE2      (Color){55, 58, 65, 255}
#define COL_PATH        (Color){90, 80, 65, 255}
#define COL_TREE        (Color){25, 65, 30, 255}
#define COL_TREE2       (Color){18, 50, 22, 255}
#define COL_BUILDING    (Color){80, 70, 60, 255}
#define COL_BUILDING2   (Color){60, 52, 44, 255}
#define COL_RUINS       (Color){65, 60, 55, 255}
#define COL_DOOR_C      (Color){100, 70, 40, 255}
#define COL_DOOR_O      (Color){50, 35, 20, 255}

#define COL_UI_BG       (Color){8, 10, 18, 230}
#define COL_UI_BORDER   (Color){60, 80, 120, 255}
#define COL_UI_ACCENT   (Color){100, 160, 220, 255}
#define COL_UI_TEXT     (Color){200, 210, 230, 255}
#define COL_UI_DIM      (Color){100, 110, 130, 255}
#define COL_UI_GOLD     (Color){200, 170, 80, 255}
#define COL_UI_RED      (Color){180, 70, 70, 255}
#define COL_UI_GREEN    (Color){70, 160, 90, 255}

#define COL_VERAZ       (Color){80, 180, 120, 255}
#define COL_FALAZ       (Color){180, 80, 80, 255}
#define COL_PARADOXAL   (Color){150, 100, 200, 255}

#define COL_FOG         (Color){30, 40, 60, 180}
#define COL_NIGHT       (Color){5, 8, 25, 180}
#define COL_LIGHTHOUSE  (Color){255, 220, 100, 255}

// =============================================
// PROTÓTIPOS DE FUNÇÕES
// =============================================

// game.c
void GameInit(GameState* g);
void GameUpdate(GameState* g);
void GameDraw(GameState* g);
void GameCleanup(GameState* g);

// game.c - sub-screens (defined in game.c)
void PauseUpdate(GameState* g);
void PauseDraw(GameState* g);
void CreditsUpdate(GameState* g);
void CreditsDraw(GameState* g);
void EndingUpdate(GameState* g);
void EndingDraw(GameState* g);

// menu.c
void MenuUpdate(GameState* g);
void MenuDraw(GameState* g);

// map.c
void MapInit(GameState* g);
void MapDraw(GameState* g);
bool TileIsWalkable(int tile);
bool CheckCollision(GameState* g, Vector2 pos);
void UpdateExplored(GameState* g);

// player.c
void PlayerInit(GameState* g);
void PlayerUpdate(GameState* g);
void PlayerDraw(GameState* g);

// npc.c
void NPCsInit(GameState* g);
void NPCsDraw(GameState* g);
void NPCsUpdate(GameState* g);
NPC* GetNearbyNPC(GameState* g);
bool NPCSaysTruth(NPC* npc, GameState* g);

// dialogue.c
void DialogueInit(GameState* g);
void DialogueUpdate(GameState* g);
void DialogueDraw(GameState* g);
void StartDialogue(GameState* g, int npc_index);
void EndDialogue(GameState* g);
void AddClue(GameState* g, const char* text, const char* tag, ClueType type, int npc, Color col);

// deduction.c
void DeductionUpdate(GameState* g);
void DeductionDraw(GameState* g);
void AddDeduction(GameState* g, const char* p1, const char* op, const char* p2, const char* conc, const char* text);
void CheckPuzzleSolution(GameState* g);

// ui.c
void UIDrawHUD(GameState* g);
void UIDrawNotification(GameState* g);
void UIUpdateNotification(GameState* g);
void ShowNotification(GameState* g, const char* title, const char* body, Color col);
void DrawPixelBox(int x, int y, int w, int h, Color bg, Color border);
void DrawWrappedText(Font font, const char* text, int x, int y, int max_w, int size, int spacing, Color col);

// environment.c
void EnvInit(GameState* g);
void EnvUpdate(GameState* g);
void EnvDraw(GameState* g);
void EnvDrawOverlay(GameState* g);

// utils
float GameLerp(float a, float b, float t);
Color ColorLerp(Color a, Color b, float t);
Color ColorAlpha2(Color c, float alpha);
bool RectContains(Rectangle r, Vector2 p);
void DrawTextCentered(Font font, const char* text, int y, int size, int spacing, Color col);
void DrawTextCenteredX(Font font, const char* text, int cx, int y, int size, int spacing, Color col);

// quiz.c
bool NpcQuizNeeded(int clue_id);
void NpcQuizSetup(GameState* g, int clue_id);
void NpcQuizUpdate(GameState* g);
void NpcQuizDraw(GameState* g);
void TruthTableSetup(GameState* g, int door_ty, int door_tx, const char* cond);
void TruthTableUpdate(GameState* g);
void TruthTableDraw(GameState* g);
bool DedQuizNeeded(const char* result);
void DedQuizSetup(GameState* g, const char* result, int ded_idx, int clue_ci);
void DedQuizUpdate(GameState* g);
void DedQuizDraw(GameState* g);
void ApplyDeductionResult(GameState* g, const char* result, int ded_idx, int clue_ci);

#endif
