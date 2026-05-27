#include "game.h"

// =============================================================
// NPCSaysTruth — decide se um NPC está dizendo a verdade agora
// =============================================================
// Essa função é o coração da lógica proposicional dos personagens.
// Cada tipo de NPC tem um comportamento fixo de veracidade:
//   - Veraz:     sempre verdade
//   - Falaz:     sempre mentira
//   - Paradoxal: depende do horário do dia (dia=verdade, noite=mentira)
bool NPCSaysTruth(NPC* npc, GameState* g) {
    switch (npc->type) {
        case NPC_VERAZ:    return true;  // Sempre fala a verdade
        case NPC_FALAZ:    return false; // Sempre mente

        case NPC_PARADOXAL:
            // O Ferreiro Kael usa o ciclo dia/noite como regra determinística:
            // Das 6h às 20h (dia)  → diz a verdade
            // Das 20h às 6h (noite) → mente
            if (g != NULL) {
                float tod = g->env.time_of_day; // Horário atual (0.0 a 24.0)
                return (tod >= 6.0f && tod < 20.0f);
            }
            // Fallback caso não haja ambiente: alterna por número de falas
            return (npc->paradox_counter % 2 == 0);
    }
    return true;
}

// =============================================================
// NPCsInit — posiciona e configura os 3 NPCs do jogo
// =============================================================
void NPCsInit(GameState* g) {
    g->npc_count = 3; // Total de NPCs no jogo

    // --- NPC 0: Velho Pescador — tipo VERAZ (sempre diz a verdade) ---
    NPC* fish = &g->npcs[0];
    strcpy(fish->name, "Velho Pescador");
    fish->type           = NPC_VERAZ;
    fish->pos            = (Vector2){7 * TILE_SIZE, 8 * TILE_SIZE}; // Posição no mapa (coluna 7, linha 8)
    fish->color          = COL_VERAZ;  // Verde: identifica visualmente como confiável
    fish->visible        = true;
    fish->talked_to      = false;
    fish->dialogue_start = 0; // Seus diálogos começam no índice 0 do banco
    fish->dialogue_count = 4; // Tem 4 linhas de diálogo
    strcpy(fish->symbol, "V"); // Símbolo lógico exibido acima do NPC quando revelado
    fish->paradox_counter   = 0;
    fish->paradox_truth_now = true;
    fish->type_revealed     = false; // Tipo só aparece quando o jogador descobrir a pista certa

    // --- NPC 1: Sacerdotisa Mira — tipo FALAZ (sempre mente) ---
    NPC* priest = &g->npcs[1];
    strcpy(priest->name, "Sacerdotisa Mira");
    priest->type           = NPC_FALAZ;
    priest->pos            = (Vector2){17 * TILE_SIZE, 7 * TILE_SIZE};
    priest->color          = COL_FALAZ; // Vermelho: alerta visual de perigo/mentira
    priest->visible        = true;
    priest->talked_to      = false;
    priest->dialogue_start = 4; // Seus diálogos começam no índice 4 do banco
    priest->dialogue_count = 4;
    strcpy(priest->symbol, "F"); // "F" de Falaz
    priest->paradox_counter   = 0;
    priest->paradox_truth_now = false; // Nunca diz a verdade
    priest->type_revealed     = false;

    // --- NPC 2: Ferreiro Kael — tipo PARADOXAL (alterna dia/noite) ---
    NPC* smith = &g->npcs[2];
    strcpy(smith->name, "Ferreiro Kael");
    smith->type           = NPC_PARADOXAL;
    smith->pos            = (Vector2){20 * TILE_SIZE, 10 * TILE_SIZE};
    smith->color          = COL_PARADOXAL; // Roxo: indica ambiguidade/dualidade
    smith->visible        = true;
    smith->talked_to      = false;
    smith->dialogue_start = 8; // Seus diálogos começam no índice 8 do banco
    smith->dialogue_count = 5;
    strcpy(smith->symbol, "P"); // "P" de Paradoxal
    smith->paradox_counter   = 0;
    smith->paradox_truth_now = true; // Começa de dia, então começa dizendo verdade
    smith->type_revealed     = false;
}

// =============================================================
// NPCsUpdate — atualiza o estado dos NPCs a cada frame
// =============================================================
void NPCsUpdate(GameState* g) {
    NPC* smith = &g->npcs[2]; // Pega o Ferreiro (índice 2)
    if (smith->type == NPC_PARADOXAL) {
        float tod = g->env.time_of_day;
        // Sincroniza o estado do Ferreiro com o horário atual
        smith->paradox_truth_now = (tod >= 6.0f && tod < 20.0f);
    }

    // Revela o tipo de cada NPC progressivamente conforme o jogador avança:
    // Pescador revelado quando pista P é confirmada
    if (g->clues[0].confirmed)     g->npcs[0].type_revealed = true;
    // Sacerdotisa revelada quando a dedução de contradição é desbloqueada
    if (g->deductions[0].unlocked) g->npcs[1].type_revealed = true;
    // Ferreiro revelado quando pista Q é confirmada
    if (g->clues[7].confirmed)     g->npcs[2].type_revealed = true;
}

// =============================================================
// GetNearbyNPC — detecta se há um NPC próximo para interagir
// =============================================================
NPC* GetNearbyNPC(GameState* g) {
    float interact_range = TILE_SIZE * 2.2f; // Raio de interação em pixels

    // Calcula o centro do jogador
    Vector2 ppos = {
        g->player.pos.x + TILE_SIZE/2,
        g->player.pos.y + TILE_SIZE/2
    };

    for (int i = 0; i < g->npc_count; i++) {
        if (!g->npcs[i].visible) continue; // Ignora NPCs invisíveis

        // Calcula o centro do NPC
        Vector2 npos = {
            g->npcs[i].pos.x + TILE_SIZE/2,
            g->npcs[i].pos.y + TILE_SIZE/2
        };

        // Distância euclidiana entre jogador e NPC: √(dx² + dy²)
        float dx = ppos.x - npos.x;
        float dy = ppos.y - npos.y;
        if (sqrtf(dx*dx + dy*dy) < interact_range) return &g->npcs[i]; // Retorna o NPC próximo
    }
    return NULL; // Nenhum NPC dentro do alcance
}

// =============================================================
// DrawNPCPixelArt — desenha o sprite pixel art de um NPC
// =============================================================
static void DrawNPCPixelArt(NPC* npc, float t, int frame) {
    (void)frame;
    int px = (int)npc->pos.x;
    int py = (int)npc->pos.y;

    // Efeito de flutuação suave usando seno do tempo
    float hover = sinf(t * 1.8f + npc->pos.x * 0.1f) * 2.0f;
    py += (int)hover;

    Color body = npc->color; // Cor principal do corpo (diferente por tipo)
    Color skin = {210,175,145,255}; // Tom de pele
    Color dark = {(unsigned char)(body.r/2),(unsigned char)(body.g/2),(unsigned char)(body.b/2),255};

    // Sombra elíptica no chão
    DrawEllipse(px + TILE_SIZE/2, (int)(npc->pos.y) + TILE_SIZE - 2, 9, 4, (Color){0,0,0,60});

    // Corpo (retângulos que formam o personagem pixel art)
    DrawRectangle(px+5, py+10, 22, 16, body);  // Tronco
    DrawRectangle(px+7, py+14, 18, 14, dark);  // Detalhe do tronco
    DrawRectangle(px+2,  py+11, 5, 10, body);  // Braço esquerdo
    DrawRectangle(px+25, py+11, 5, 10, body);  // Braço direito

    DrawRectangle(px+9, py+2, 14, 12, skin);   // Cabeça

    // Chapéu/cabelo diferente por tipo de NPC
    switch (npc->type) {
        case NPC_VERAZ:
            DrawRectangle(px+9, py+2, 14, 4, (Color){80,60,40,255}); // Chapéu marrom simples
            break;
        case NPC_FALAZ:
            DrawRectangle(px+9, py+2, 14, 5, (Color){150,20,20,255}); // Capuz vermelho
            DrawRectangle(px+7, py+2, 18, 3, (Color){130,15,15,255});
            break;
        case NPC_PARADOXAL: {
            DrawRectangle(px+9, py+2, 14, 4, (Color){50,30,80,255}); // Capuz roxo escuro
            // Aura pulsante que muda conforme está mentindo ou falando verdade
            float pg = 0.4f + 0.6f * sinf(t * 3.0f);
            if (npc->paradox_truth_now) {
                // Aura dourada quando diz a verdade (período diurno)
                DrawCircle(px+TILE_SIZE/2, py+TILE_SIZE/2, 18,
                    (Color){200,180,80,(unsigned char)(20*pg)});
            } else {
                // Aura roxa intensa quando está mentindo (período noturno)
                DrawCircle(px+TILE_SIZE/2, py+TILE_SIZE/2, 22,
                    (Color){150,100,200,(unsigned char)(50*pg)});
                DrawCircle(px+TILE_SIZE/2, py+TILE_SIZE/2, 12,
                    (Color){200,100,255,(unsigned char)(30*pg)});
            }
            break;
        }
    }

    // Olhos (dois pixels escuros)
    DrawRectangle(px+11, py+8, 2, 2, (Color){30,30,40,255});
    DrawRectangle(px+18, py+8, 2, 2, (Color){30,30,40,255});

    // Exibe o símbolo lógico (V, F ou P) acima do NPC quando seu tipo foi revelado
    if (npc->type_revealed) {
        float sym_alpha = 0.6f + 0.4f * sinf(t * 2.0f); // Pisca suavemente
        Color sym_col = npc->color;
        sym_col.a = (unsigned char)(sym_alpha * 200);
        DrawText(npc->symbol, px + TILE_SIZE/2 - 4, py - 14, 12, sym_col);
    }
}

// =============================================================
// DrawNPCNameplate — desenha o nome do NPC quando o jogador está perto
// =============================================================
static void DrawNPCNameplate(NPC* npc, bool is_nearby, float t) {
    if (!is_nearby) return; // Só exibe quando o jogador está próximo
    int px = (int)npc->pos.x;
    int py = (int)npc->pos.y;
    float hover = sinf(t * 1.8f + npc->pos.x * 0.1f) * 2.0f;
    int ny = py - 28 + (int)hover; // Posiciona a nameplate acima do personagem
    int tw = MeasureText(npc->name, 12) + 10; // Largura do texto + margem
    // Caixa de fundo semi-transparente
    DrawRectangle(px + TILE_SIZE/2 - tw/2, ny - 2, tw, 16, (Color){8,10,18,200});
    DrawRectangleLinesEx((Rectangle){(float)(px + TILE_SIZE/2 - tw/2),(float)(ny-2),(float)tw,16.0f},
        1, npc->color); // Borda na cor do NPC
    DrawText(npc->name, px + TILE_SIZE/2 - tw/2 + 5, ny, 12, COL_UI_TEXT);
}

// =============================================================
// NPCsDraw — desenha todos os NPCs visíveis no mapa
// =============================================================
void NPCsDraw(GameState* g) {
    NPC* nearby = GetNearbyNPC(g); // Descobre qual NPC está perto do jogador
    static int walk_frame = 0;
    static float walk_timer = 0;
    walk_timer += g->delta;
    // Avança o frame de animação a cada 0.3 segundos (ciclo de 4 frames)
    if (walk_timer > 0.3f) { walk_timer = 0; walk_frame = (walk_frame+1)%4; }
    for (int i = 0; i < g->npc_count; i++) {
        if (!g->npcs[i].visible) continue;
        DrawNPCPixelArt(&g->npcs[i], g->menu_anim, walk_frame);
        DrawNPCNameplate(&g->npcs[i], (&g->npcs[i] == nearby), g->menu_anim); // Nameplate só no próximo
    }
}
