#include "game.h"

// Variáveis de estado das sub-telas (escopo de arquivo = compartilhadas entre funções)
int   pause_selection = 0;    // Opção selecionada no menu de pausa (0=Continuar, 1=Créditos, 2=Menu)
float credits_scroll  = 0.0f; // Posição de scroll (pixels) dos créditos
float ending_timer    = 0.0f; // Tempo decorrido na tela de fim de jogo
int   ending_phase    = 0;    // Fase do ending: 0=Farol acendendo, 1=Narrativa, 2=Prompt

// Declarações antecipadas das funções de sub-telas (implementadas abaixo)
void PauseUpdate(GameState* g);
void PauseDraw(GameState* g);
void CreditsUpdate(GameState* g);
void CreditsDraw(GameState* g);
void EndingUpdate(GameState* g);
void EndingDraw(GameState* g);

// =============================================================
// SaveData — estrutura serializada no arquivo de save
// =============================================================
// Tudo que precisa persistir entre sessões é copiado para essa
// struct e gravado em binário. Novos campos SEMPRE vão ao final
// para manter compatibilidade com saves antigos.
#define SAVE_FILE "savegame.dat"

typedef struct {
    Vector2 player_pos;                      // Posição do jogador no mapa
    int     lighthouse_fragments;            // Fragmentos do Farol coletados (0-3)
    bool    mystery_solved;                  // Mistério foi resolvido?
    bool    lighthouse_activated;            // Farol foi ativado (fim de jogo)?
    bool    clue_discovered[MAX_CLUES];      // Quais pistas foram descobertas
    bool    clue_confirmed[MAX_CLUES];       // Quais pistas foram confirmadas por dedução
    bool    npc_talked[MAX_NPCS];            // Com quais NPCs o jogador conversou
    int     deduction_unlocked[MAX_DEDUCTIONS]; // Quais deduções foram realizadas
    float   time_of_day;                     // Hora atual do ciclo dia/noite (0.0–24.0)
    // Campos adicionados: sempre no final para não quebrar saves anteriores
    bool    npc_type_revealed[MAX_NPCS];     // Tipo de cada NPC foi revelado ao jogador?
    bool    contradiction_active;            // Contradição P ∧ ¬P' foi detectada?
} SaveData;

// =============================================================
// GameSave — grava o estado atual do jogo em arquivo binário
// =============================================================
static void GameSave(GameState* g) {
    SaveData sd = {0}; // Zera tudo antes de preencher

    // Copia cada campo relevante do GameState para a SaveData
    sd.player_pos           = g->player.pos;
    sd.lighthouse_fragments = g->player.lighthouse_fragments;
    sd.mystery_solved       = g->mystery_solved;
    sd.lighthouse_activated = g->lighthouse_activated;
    sd.time_of_day          = g->env.time_of_day;

    // Arrays: pistas e NPCs
    for (int i = 0; i < g->clue_count; i++) {
        sd.clue_discovered[i] = g->clues[i].discovered;
        sd.clue_confirmed[i]  = g->clues[i].confirmed;
    }
    for (int i = 0; i < g->npc_count; i++) {
        sd.npc_talked[i]        = g->npcs[i].talked_to;
        sd.npc_type_revealed[i] = g->npcs[i].type_revealed;
    }
    for (int i = 0; i < g->deduction_count; i++)
        sd.deduction_unlocked[i] = g->deductions[i].unlocked ? 1 : 0;

    sd.contradiction_active = g->contradiction_active;
    sd.lighthouse_fragments = g->player.lighthouse_fragments;

    // Abre o arquivo no modo "wb" (write binary) e grava a struct de uma vez
    FILE* f = fopen(SAVE_FILE, "wb");
    if (f) { fwrite(&sd, sizeof(SaveData), 1, f); fclose(f); }
}

// =============================================================
// GameLoad — carrega o estado do jogo a partir do arquivo de save
// =============================================================
// Retorna true se o arquivo existia e foi carregado com sucesso.
static bool GameLoad(GameState* g) {
    FILE* f = fopen(SAVE_FILE, "rb"); // "rb" = read binary
    if (!f) return false;             // Arquivo não existe: novo jogo
    SaveData sd = {0};
    if (fread(&sd, sizeof(SaveData), 1, f) != 1) { fclose(f); return false; } // Leitura falhou
    fclose(f);

    // Restaura cada campo do GameState a partir da SaveData
    g->player.pos                  = sd.player_pos;
    g->player.lighthouse_fragments = sd.lighthouse_fragments;
    g->mystery_solved              = sd.mystery_solved;
    g->lighthouse_activated        = sd.lighthouse_activated;
    g->env.time_of_day             = sd.time_of_day;

    for (int i = 0; i < g->clue_count; i++) {
        g->clues[i].discovered = sd.clue_discovered[i];
        g->clues[i].confirmed  = sd.clue_confirmed[i];
    }
    for (int i = 0; i < g->npc_count; i++) {
        g->npcs[i].talked_to     = sd.npc_talked[i];
        g->npcs[i].type_revealed = sd.npc_type_revealed[i];
    }
    for (int i = 0; i < g->deduction_count; i++)
        g->deductions[i].unlocked = sd.deduction_unlocked[i] != 0;

    g->contradiction_active = sd.contradiction_active;
    return true;
}

// =============================================================
// GameInit — inicializa todos os subsistemas do jogo
// =============================================================
// Chamada uma única vez no main() antes do loop principal.
void GameInit(GameState* g) {
    // Fontes padrão da Raylib (podem ser trocadas por fontes customizadas)
    g->font_main = GetFontDefault();
    g->font_mono = GetFontDefault();

    // Estado inicial das telas e variáveis globais
    g->current_screen = SCREEN_MENU;   // Começa no menu principal
    g->prev_screen    = SCREEN_MENU;
    g->menu_selection = 0;
    g->menu_anim      = 0.0f;          // Clock global de animação (incrementado a cada frame)
    g->active_npc     = -1;            // -1 = nenhum NPC em diálogo
    g->selected_clue  = -1;
    g->fade_alpha     = 1.0f;          // Começa em 1.0 (tela preta) e faz fade in
    g->fading_in      = true;
    g->fading_out     = false;
    g->audio_on       = true;
    g->frame          = 0;
    g->screen_shake   = 0.0f;
    g->is_new_game          = false;
    g->mystery_solved       = false;
    g->lighthouse_activated = false;
    g->puzzle_step          = 0;
    g->contradiction_active = false;

    // Estado inicial do combinador de deduções (Caderno)
    g->deduce_slot_a      = -1; // Nenhuma pista selecionada no slot A
    g->deduce_slot_b      = -1; // Nenhuma pista selecionada no slot B
    g->deduce_op          = 0;  // Operador padrão: ∧ (AND)
    g->deduce_error_timer = 0;
    g->deduce_panel_focus = false;

    // Inicializa cada subsistema na ordem correta
    MapInit(g);       // Gera o mapa tile a tile
    PlayerInit(g);    // Posiciona o jogador e inicializa animações
    NPCsInit(g);      // Posiciona os 3 NPCs e define seus tipos
    DialogueInit(g);  // Carrega banco de diálogos, pistas e deduções
    EnvInit(g);       // Inicia ciclo dia/noite e clima

    // Configura a câmera 2D centralizada no jogador
    g->camera.target   = g->player.pos;
    g->camera.offset   = (Vector2){SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f};
    g->camera.rotation = 0.0f;
    g->camera.zoom     = 1.5f; // Zoom in para dar sensação de escala pixel art

    // Verifica se existe um arquivo de save (para mostrar "Continuar" no menu)
    g->has_save = (fopen(SAVE_FILE, "rb") != NULL);
}

// =============================================================
// GameUpdate — atualiza toda a lógica do jogo a cada frame
// =============================================================
void GameUpdate(GameState* g) {
    // --- Sistema de fade in/out (transições entre telas) ---
    if (g->fading_in) {
        // Fade in: alpha vai de 1.0 (preto) até 0.0 (transparente)
        g->fade_alpha -= g->delta * 1.5f;
        if (g->fade_alpha <= 0.0f) { g->fade_alpha = 0.0f; g->fading_in = false; }
    }
    if (g->fading_out) {
        // Fade out: alpha vai de 0.0 até 1.0 (escurece)
        g->fade_alpha += g->delta * 1.5f;
        if (g->fade_alpha >= 1.0f) {
            g->fade_alpha = 1.0f;
            g->fading_out = false;
            g->fading_in  = true; // Inicia o fade in da próxima tela

            // Aplica a mudança de tela pendente (definida antes de iniciar o fade)
            g->current_screen = g->prev_screen;

            // Carrega o save ao entrar no jogo (somente se for "Continuar", não "Novo Jogo")
            if (g->current_screen == SCREEN_GAME && g->has_save && !g->is_new_game) {
                GameLoad(g);
            }
            // Limpa a flag de novo jogo ao entrar no jogo de fato
            if (g->current_screen == SCREEN_GAME) {
                g->is_new_game = false;
                g->has_save    = true;
            }
            // Ao entrar na intro: reinicia TUDO do zero (novo jogo)
            if (g->current_screen == SCREEN_INTRO) {
                MapInit(g);
                PlayerInit(g);
                NPCsInit(g);
                DialogueInit(g);
                EnvInit(g);
                // Reseta todos os flags de progresso
                g->mystery_solved       = false;
                g->lighthouse_activated = false;
                g->puzzle_step          = 0;
                g->selected_clue        = -1;
                g->deduction_scroll     = 0;
                g->screen_shake         = 0;
                g->notif.active         = false;
                g->ambient_timer        = 0;
                g->contradiction_active = false;
                g->deduce_slot_a        = -1;
                g->deduce_slot_b        = -1;
                g->deduce_op            = 0;
                g->deduce_error_timer   = 0;
                // Reseta estados das sub-telas
                pause_selection = 0;
                credits_scroll  = 0.0f;
                ending_timer    = 0.0f;
                ending_phase    = 0;
                // Centraliza câmera no jogador reiniciado
                g->camera.target   = g->player.pos;
                g->camera.offset   = (Vector2){SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f};
                g->camera.zoom     = 1.5f;
                // Inicia a narração da intro (linha 50 do banco de diálogos)
                g->active_npc      = -2; // -2 = narrador (sem NPC real)
                g->dialogue_line   = 50;
                g->dialogue_typing = true;
                g->type_timer      = 0;
                g->type_char       = 0;
                g->typed_text[0]   = 0;
            }
        }
    }

    // Screen shake: decai 5 unidades por segundo após uma dedução importante
    if (g->screen_shake > 0) { g->screen_shake -= g->delta * 5.0f; }
    if (g->screen_shake < 0)   g->screen_shake = 0;

    // Clock global de animação: cresce continuamente (usado em sinf() para efeitos)
    g->menu_anim += g->delta;

    // --- Despacha a atualização para a tela correta ---
    switch (g->current_screen) {
        case SCREEN_MENU:      MenuUpdate(g);      break; // Menu principal
        case SCREEN_INTRO:     DialogueUpdate(g);  break; // Narração de intro

        case SCREEN_GAME:
            EnvUpdate(g);    // Avança o ciclo dia/noite e clima
            UIUpdateNotification(g);
            // MELHORIA 1: puzzle de tabela-verdade bloqueia movimento
            if (g->ttable_active) {
                TruthTableUpdate(g);
            } else {
                PlayerUpdate(g); // Processa input e move o jogador
                NPCsUpdate(g);   // Atualiza estado dos NPCs (ex: Ferreiro dia/noite)
            }
            // Centraliza a câmera no centro do jogador
            g->camera.target = (Vector2){
                g->player.pos.x + TILE_SIZE/2.0f,
                g->player.pos.y + TILE_SIZE/2.0f
            };
            // Auto-save a cada 60 segundos de gameplay
            g->ambient_timer += g->delta;
            if (g->ambient_timer > 60.0f) {
                g->ambient_timer = 0;
                GameSave(g);
            }
            // Verifica se o Farol foi ativado para acionar o ending
            if (g->lighthouse_activated) {
                g->current_screen = SCREEN_ENDING;
                GameSave(g); // Salva o estado final antes do ending
            }
            break;

        case SCREEN_DIALOGUE:
            DialogueUpdate(g);       // Processa input no diálogo (avançar, pular)
            EnvUpdate(g);            // Continua o ciclo dia/noite durante o diálogo
            UIUpdateNotification(g);
            break;

        case SCREEN_DEDUCTION: DeductionUpdate(g); break; // Caderno de Deduções
        case SCREEN_PAUSE:     PauseUpdate(g);     break; // Menu de pausa
        case SCREEN_CREDITS:   CreditsUpdate(g);   break; // Tela de créditos
        case SCREEN_ENDING:    EndingUpdate(g);    break; // Tela de fim de jogo
        default: break;
    }
}

// =============================================================
// DrawGameWorld — renderiza o mundo do jogo com câmera 2D
// =============================================================
// Aplica o screen shake deslocando o offset da câmera aleatoriamente
// e então desenha mapa, NPCs, jogador e efeitos de ambiente.
static void DrawGameWorld(GameState* g) {
    float sx = 0, sy = 0;
    if (g->screen_shake > 0) {
        // Offset aleatório proporcional à intensidade do shake
        sx = (float)(GetRandomValue(-3,3)) * g->screen_shake;
        sy = (float)(GetRandomValue(-3,3)) * g->screen_shake;
    }
    Camera2D cam = g->camera;
    cam.offset.x += sx; // Aplica o shake sem modificar a câmera original
    cam.offset.y += sy;

    BeginMode2D(cam); // Todas as chamadas abaixo usam coordenadas do mundo
        MapDraw(g);    // Tiles do terreno + portas lógicas
        NPCsDraw(g);   // Sprites dos NPCs com animação e nameplate
        PlayerDraw(g); // Sprite do jogador com indicador de interação
        EnvDraw(g);    // Efeitos de ambiente: partículas, névoa, luz
    EndMode2D();       // Volta para coordenadas de tela
    EnvDrawOverlay(g); // Overlay de neblina e vinheta (sempre em cima, sem câmera)
}

// =============================================================
// GameDraw — desenha a tela correta conforme o estado atual
// =============================================================
void GameDraw(GameState* g) {
    switch (g->current_screen) {
        case SCREEN_MENU:
            MenuDraw(g); // Menu principal com animações e logo
            break;

        case SCREEN_INTRO:
            ClearBackground(COL_BG);
            DialogueDraw(g); // Caixa de diálogo da narração de intro
            break;

        case SCREEN_GAME:
            DrawGameWorld(g);         // Mundo completo
            UIDrawHUD(g);             // HUD: fragmentos do Farol, hora, pistas
            UIDrawNotification(g);    // Notificações flutuantes
            TruthTableDraw(g);        // MELHORIA 1: overlay do puzzle de porta
            break;

        case SCREEN_DIALOGUE:
            DrawGameWorld(g);         // Mundo ao fundo (o jogo continua visível)
            UIDrawHUD(g);
            DialogueDraw(g);          // Caixa de diálogo em cima do mundo
            UIDrawNotification(g);
            break;

        case SCREEN_DEDUCTION:
            DeductionDraw(g); // Caderno de Deduções (tela dedicada, sem mundo)
            break;

        case SCREEN_PAUSE:
            DrawGameWorld(g);  // Mundo ao fundo (pausado)
            UIDrawHUD(g);
            PauseDraw(g);      // Overlay de pausa sobre o mundo
            break;

        case SCREEN_CREDITS:
            CreditsDraw(g); // Tela de créditos com scroll automático
            break;

        case SCREEN_ENDING:
            DrawGameWorld(g);  // Mundo ao fundo durante o ending
            EndingDraw(g);     // Overlay de ending (farol, narração, navio)
            break;

        default: break;
    }

    // --- Overlay de fade global (preto transparente) ---
    // Desenhado por cima de TUDO a cada frame.
    // Quando fade_alpha = 0: invisível. Quando = 1: tela completamente preta.
    if (g->fade_alpha > 0.0f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
            (Color){0,0,0,(unsigned char)(g->fade_alpha * 255)});
    }
}

// =============================================================
// GameCleanup — libera recursos e salva ao encerrar
// =============================================================
void GameCleanup(GameState* g) {
    GameSave(g); // Garante que o progresso seja salvo ao fechar o jogo
}

// =============================================================
// PauseUpdate — processa input no menu de pausa
// =============================================================
void PauseUpdate(GameState* g) {
    // Navega entre as opções com setas ou WASD
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) { pause_selection--; if (pause_selection < 0) pause_selection = 2; }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { pause_selection++; if (pause_selection > 2) pause_selection = 0; }

    // ESC ou P: retorna ao jogo sem fazer nada
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        g->current_screen = SCREEN_GAME;
        pause_selection = 0;
    }
    // ENTER/ESPAÇO: executa a opção selecionada
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        switch (pause_selection) {
            case 0: // Continuar: volta ao jogo
                g->current_screen = SCREEN_GAME;
                pause_selection = 0;
                break;
            case 1: // Créditos
                g->current_screen = SCREEN_CREDITS;
                break;
            case 2: // Menu principal: salva e volta
                GameSave(g);
                g->current_screen = SCREEN_MENU;
                g->has_save = true;
                pause_selection = 0;
                break;
        }
    }
}

// =============================================================
// PauseDraw — desenha o overlay de pausa sobre o mundo
// =============================================================
void PauseDraw(GameState* g) {
    int W = GetScreenWidth(), H = GetScreenHeight();
    float t = g->menu_anim;

    // Escurece o mundo ao fundo sem apagar totalmente
    DrawRectangle(0, 0, W, H, (Color){0,0,0,180});

    // Painel central
    int pw = 400, ph = 280;
    int px = W/2 - pw/2, py = H/2 - ph/2;
    DrawRectangle(px, py, pw, ph, (Color){6,9,20,245});
    DrawRectangleLinesEx((Rectangle){(float)px,(float)py,(float)pw,(float)ph}, 2, COL_UI_BORDER);

    // Linha de destaque no topo do painel
    DrawRectangle(px+2, py+2, pw-4, 3, COL_UI_ACCENT);

    DrawTextCentered(g->font_main, "PAUSADO", py+20, 32, 2, COL_UI_ACCENT);
    DrawRectangle(px+20, py+62, pw-40, 1, COL_UI_BORDER);

    // Lista de opções com highlight na selecionada
    const char* opts[] = {"Continuar Jogo", "Créditos", "Voltar ao Menu"};
    for (int i = 0; i < 3; i++) {
        bool sel = (pause_selection == i); // Esta opção está selecionada?
        int oy = py + 85 + i*52;
        float pulse = sel ? (0.85f + 0.15f*sinf(t*3.0f)) : 1.0f;

        if (sel) {
            // Fundo azul pulsante na opção selecionada
            DrawRectangle(px+20, oy-6, pw-40, 36, (Color){20,40,80,180});
            DrawRectangleLinesEx((Rectangle){(float)(px+20),(float)(oy-6),(float)(pw-40),36.0f},
                1, (Color){80,120,200,(unsigned char)(180*pulse)});
            DrawText(">", px+26, oy+2, 18, COL_UI_ACCENT); // Seta indicadora
        }
        Color col = sel ? (Color){220,235,255,(unsigned char)(255*pulse)} : COL_UI_DIM;
        DrawTextCentered(g->font_main, opts[i], oy, sel?20:18, 1, col);
    }
    // Dica de controles na base do painel
    DrawTextCentered(g->font_main, "↑↓ Navegar  ENTER Confirmar  ESC Continuar",
        py+ph-22, 12, 1, (Color){60,80,120,200});
}

// =============================================================
// CreditsUpdate — lógica da tela de créditos (scroll automático)
// =============================================================
void CreditsUpdate(GameState* g) {
    credits_scroll += g->delta * 30.0f; // Avança 30 pixels por segundo

    // Qualquer tecla de confirmação sai dos créditos
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_SPACE)  || IsKeyPressed(KEY_BACKSPACE)) {
        // Volta para pausa (se chegou de lá) ou para o menu
        g->current_screen = (g->prev_screen == SCREEN_PAUSE) ? SCREEN_PAUSE : SCREEN_MENU;
        credits_scroll = 0.0f;
        g->prev_screen = SCREEN_CREDITS;
    }
}

// =============================================================
// CreditsDraw — renderiza os créditos com scroll vertical
// =============================================================
void CreditsDraw(GameState* g) {
    int W = GetScreenWidth(), H = GetScreenHeight();
    float t = g->menu_anim;

    ClearBackground(COL_BG);

    // Estrelas de fundo (posições determinísticas para não piscarem)
    for (int i = 0; i < 60; i++) {
        float bri = 0.4f + 0.6f*sinf(t*1.2f + i*0.9f); // Brilho oscilante
        int sx = (i*137)%W, sy = (i*97)%(H/2);
        DrawRectangle(sx, sy, 1+(i%3==0?1:0), 1+(i%3==0?1:0),
            (Color){200,210,255,(unsigned char)(bri*140)});
    }

    // Farol decorativo no topo com luz pulsante
    float lp = 0.5f + 0.5f*sinf(t*1.5f);
    DrawCircle(W/2, 80, (int)(40*lp), (Color){255,220,80,(unsigned char)(30*lp)});  // Aura externa
    DrawCircle(W/2, 80, 18, (Color){255,220,100,(unsigned char)(200*lp)});            // Núcleo luminoso
    DrawRectangle(W/2-6, 80, 12, 120, (Color){70,72,82,255});                        // Torre do farol

    // Posição Y do topo do texto (rola para cima com credits_scroll)
    int base_y = H - (int)credits_scroll + 80;

    // Tabela de linhas de crédito: texto, tamanho e cor
    struct { const char* text; int size; Color col; } lines[] = {
        {"ILHA DOS PARADOXOS",            32, COL_UI_ACCENT},
        {"",                               8, WHITE},
        {"Um jogo de mistério e lógica",  16, COL_UI_DIM},
        {"",                              16, WHITE},
        {"─────────────────────",         14, COL_UI_BORDER},
        {"DESENVOLVIMENTO",               20, COL_UI_GOLD},
        {"─────────────────────",         14, COL_UI_BORDER},
        {"",                              12, WHITE},
        {"Design & Programação",          16, COL_UI_TEXT},
        {"Ilha dos Paradoxos Team",       14, COL_UI_DIM},
        {"",                              12, WHITE},
        {"Motor Gráfico",                 16, COL_UI_TEXT},
        {"Raylib 5.0  —  raylib.com",     14, COL_UI_DIM},
        {"",                              12, WHITE},
        {"Linguagem",                     16, COL_UI_TEXT},
        {"C (C11 Standard)",              14, COL_UI_DIM},
        {"",                              16, WHITE},
        {"─────────────────────",         14, COL_UI_BORDER},
        {"REFERÊNCIAS",                   20, COL_UI_GOLD},
        {"─────────────────────",         14, COL_UI_BORDER},
        {"",                              12, WHITE},
        {"Return of the Obra Dinn",       14, COL_UI_DIM},
        {"The Case of the Golden Idol",   14, COL_UI_DIM},
        {"Outer Wilds",                   14, COL_UI_DIM},
        {"Undertale",                     14, COL_UI_DIM},
        {"OneShot",                       14, COL_UI_DIM},
        {"",                              16, WHITE},
        {"─────────────────────",         14, COL_UI_BORDER},
        {"AGRADECIMENTOS",                20, COL_UI_GOLD},
        {"─────────────────────",         14, COL_UI_BORDER},
        {"",                              12, WHITE},
        {"A todos os jogadores que",      14, COL_UI_DIM},
        {"acreditam que a lógica",        14, COL_UI_DIM},
        {"pode ser uma aventura.",        14, COL_UI_DIM},
        {"",                              20, WHITE},
        {"\"A lógica é a linguagem",      18, COL_UI_ACCENT},
        {"   da ilha...\"",               18, COL_UI_ACCENT},
        {"",                              24, WHITE},
        {"ESC / ENTER para voltar",       14, COL_UI_DIM},
    };

    int count = (int)(sizeof(lines)/sizeof(lines[0]));
    int cy = base_y;
    for (int i = 0; i < count; i++) {
        // Só desenha linhas visíveis na tela (otimização)
        if (cy > -30 && cy < H+30)
            DrawTextCentered(g->font_main, lines[i].text, cy, lines[i].size, 1, lines[i].col);
        cy += lines[i].size + 10;
    }

    // Reinicia o scroll quando tudo já passou (loop infinito dos créditos)
    if (base_y + cy - 80 < 0) credits_scroll = 0;

    // Vinheta: gradiente preto nas bordas superior e inferior para suavizar o scroll
    for (int y = 0; y < 80; y++) {
        float a = 1.0f - (float)y/80.0f; // Vai de 1.0 (opaco) até 0.0 (transparente)
        DrawRectangle(0, y, W, 1, (Color){COL_BG.r,COL_BG.g,COL_BG.b,(unsigned char)(a*255)});
        DrawRectangle(0, H-1-y, W, 1, (Color){COL_BG.r,COL_BG.g,COL_BG.b,(unsigned char)(a*255)});
    }
}

// =============================================================
// EndingUpdate — controla as fases da cena final do jogo
// =============================================================
// O ending tem 3 fases temporizadas:
//   Fase 0 (0–4s):   Farol acendendo + raios giratórios
//   Fase 1 (4–12s):  Texto narrativo + navio aparecendo
//   Fase 2 (12s+):   Prompt de continuar para créditos
void EndingUpdate(GameState* g) {
    ending_timer += g->delta; // Avança o timer do ending

    // Transições automáticas de fase por tempo
    if (ending_phase == 0 && ending_timer > 4.0f)  ending_phase = 1;
    if (ending_phase == 1 && ending_timer > 12.0f) ending_phase = 2;

    // Na fase final, aguarda input do jogador
    if (ending_phase >= 2) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            ending_phase = 0; ending_timer = 0;
            g->current_screen = SCREEN_CREDITS; // Vai para os créditos
            g->prev_screen = SCREEN_ENDING;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            ending_phase = 0; ending_timer = 0;
            g->current_screen = SCREEN_MENU;
            g->has_save = false;
            remove(SAVE_FILE); // Apaga o save ao retornar ao menu (jogo concluído)
        }
    }
}

// =============================================================
// EndingDraw — renderiza as 3 fases do ending cinematicamente
// =============================================================
void EndingDraw(GameState* g) {
    int W = GetScreenWidth(), H = GetScreenHeight();
    float t = g->menu_anim;

    // Overlay escuro sobre o mundo (o mapa ainda aparece ao fundo)
    DrawRectangle(0, 0, W, H, (Color){0,0,10,200});

    // --- FASE 0: Farol acendendo (raios de luz giratórios) ---
    // Calcula o alpha de fade-out da fase 0 (some quando a fase 1 começa)
    float phase0_alpha = 1.0f;
    if (ending_phase > 0) phase0_alpha = (float)fmax(0, 1.0f - (ending_timer-4.0f)*0.5f);

    if (phase0_alpha > 0.0f) {
        // 3 raios de luz giratórios separados 120° entre si
        float beam_rot = fmodf(t * 60.0f, 360.0f); // Gira 60°/segundo
        for (int b = 0; b < 3; b++) {
            float ang = (beam_rot + b*120.0f) * DEG2RAD;
            float len = 500.0f;
            Vector2 tip = {(float)W/2, (float)H/2 - 60}; // Origem no farol
            // Dois pontos laterais formam o triângulo do raio
            Vector2 b1 = {tip.x + cosf(ang-0.18f)*len, tip.y + sinf(ang-0.18f)*len};
            Vector2 b2 = {tip.x + cosf(ang+0.18f)*len, tip.y + sinf(ang+0.18f)*len};
            DrawTriangle(tip, b1, b2, (Color){255,220,80,(unsigned char)(50*phase0_alpha)});
        }
        // Glow central pulsante do farol
        float glow = 0.6f + 0.4f*sinf(t*3.0f);
        DrawCircle(W/2, H/2-60, (int)(60*glow), (Color){255,220,100,(unsigned char)(80*phase0_alpha*glow)});
        DrawCircle(W/2, H/2-60, 24, (Color){255,240,180,(unsigned char)(240*phase0_alpha)});

        // Texto da fase 0 com fade in suave
        float ta = (float)fmin(1.0f, ending_timer*0.5f) * phase0_alpha;
        DrawTextCentered(g->font_main, "O FAROL DESPERTA",
            H/2+40, 28, 2, (Color){255,220,100,(unsigned char)(200*ta)});
        DrawTextCentered(g->font_main, "A lógica da ilha se revela completa...",
            H/2+80, 16, 1, (Color){180,200,240,(unsigned char)(160*ta)});
    }

    // --- FASE 1: Texto narrativo + navio no horizonte ---
    if (ending_phase >= 1) {
        float alpha = (float)fmin(1.0f, (ending_timer-4.0f)*0.4f); // Fade in após 4s

        // Overlay escuro completo para contrastar com as estrelas e o texto
        DrawRectangle(0, 0, W, H, (Color){0,0,5,(unsigned char)(220*alpha)});

        // Navio pixel art aparecendo à direita com balanço suave
        float ship_x = W * 0.7f + sinf(t*0.3f)*10.0f; // Balança levemente
        float ship_y = H * 0.4f;
        float sa = (float)fmin(1.0f, (ending_timer-5.0f)*0.3f) * alpha; // Aparece após 5s
        DrawRectangle((int)ship_x-30, (int)ship_y+10, 60, 14, (Color){100,80,60,(unsigned char)(200*sa)}); // Casco
        DrawRectangle((int)ship_x-1,  (int)ship_y-30, 2, 42, (Color){80,70,55,(unsigned char)(200*sa)});   // Mastro
        DrawTriangle(                                                                                         // Vela
            (Vector2){ship_x, ship_y-28},
            (Vector2){ship_x-18, ship_y+6},
            (Vector2){ship_x+18, ship_y+6},
            (Color){200,200,190,(unsigned char)(180*sa)});
        DrawCircle((int)ship_x, (int)ship_y, 30, (Color){255,200,80,(unsigned char)(30*sa)}); // Brilho do farol no navio

        // Estrelas no céu noturno
        for (int i = 0; i < 80; i++) {
            float bri = 0.4f + 0.6f*sinf(t*1.2f+i*0.7f);
            int sx2 = (i*137)%W, sy2 = (i*97)%(H*2/3);
            DrawRectangle(sx2, sy2, 1+(i%5==0?1:0), 1+(i%5==0?1:0),
                (Color){200,210,255,(unsigned char)(bri*160*alpha)});
        }

        // Parágrafos narrativos: cada linha aparece com delay progressivo
        struct { const char* line; int y; int size; } narr[] = {
            {"Você descobriu a verdade.",               H/2-80, 22},
            {"Martim desapareceu nas Ruínas Antigas,",  H/2-44, 16},
            {"atraído por um símbolo do Código do Farol.", H/2-22, 16},
            {"O Farol foi ativado.",                    H/2+20, 22},
            {"Um navio se aproxima no horizonte.",      H/2+56, 16},
            {"Você vai partir.",                        H/2+90, 18},
            {"Mas a ilha... sempre traz mais",           H/2+126, 14},
            {"do que leva.",                            H/2+146, 14},
        };
        for (int i = 0; i < 8; i++) {
            // Cada linha começa a aparecer 0.4s após a anterior
            float la = (float)fmin(1.0f, (ending_timer - 4.5f - i*0.4f)*0.8f) * alpha;
            if (la <= 0) continue;
            DrawTextCentered(g->font_main, narr[i].line, narr[i].y,
                narr[i].size, 1, (Color){200,215,240,(unsigned char)(220*la)});
        }
    }

    // --- FASE 2: Prompt final piscante ---
    if (ending_phase >= 2) {
        float pulse = 0.5f + 0.5f*sinf(t*2.5f); // Pisca suavemente
        DrawTextCentered(g->font_main, "[ ENTER ] Ver créditos",
            H - 70, 16, 1, (Color){120,160,220,(unsigned char)(220*pulse)});
        DrawTextCentered(g->font_main, "[ ESC ] Voltar ao menu",
            H - 44, 14, 1, (Color){80,100,140,(unsigned char)(160*pulse)});
    }
}
