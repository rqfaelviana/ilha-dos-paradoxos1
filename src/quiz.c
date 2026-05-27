#include "game.h"

// ================================================================
// quiz.c — MELHORIAS 1, 2 e 3
//
// Melhoria 1: TruthTable — puzzle de tabela-verdade nas portas
// Melhoria 2: NpcQuiz    — NPC faz pergunta antes de revelar pista
// Melhoria 3: DedQuiz    — quiz antes de confirmar dedução crítica
// ================================================================

// ================================================================
// HELPERS DE DESENHO
// ================================================================

static void DrawQuizBox(int x, int y, int w, int h, const char* title, Color accent) {
    // Fundo semitransparente escuro
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0,160});
    // Caixa principal
    DrawRectangle(x, y, w, h, (Color){8,10,22,245});
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,(float)h}, 2, accent);
    // Barra de título
    DrawRectangle(x, y, w, 36, (Color){accent.r/5,accent.g/5,accent.b/5,220});
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,36}, 1, accent);
    int tw = MeasureText(title, 18);
    DrawText(title, x + w/2 - tw/2, y + 9, 18, accent);
}

static void DrawOptionButton(int x, int y, int w, int h,
                            fconst char* text, bool selected, bool answered,
                            bool correct, bool wrong, float t) {
    Color bg     = (Color){12,16,30,220};
    Color border = (Color){40,55,85,255};
    Color tc     = COL_UI_TEXT;

    if (selected && !answered) {
        bg     = (Color){20,40,80,230};
        border = COL_UI_ACCENT;
        tc     = WHITE;
    }
    if (answered && correct) {
        float p = 0.7f + 0.3f*sinf(t*4.0f);
        bg     = (Color){10,(unsigned char)(80*p),20,230};
        border = COL_UI_GREEN;
        tc     = COL_UI_GREEN;
    }
    if (answered && wrong && selected) {
        float p = 0.7f + 0.3f*sinf(t*8.0f);
        bg     = (Color){(unsigned char)(80*p),10,10,230};
        border = COL_UI_RED;
        tc     = COL_UI_RED;
    }

    DrawRectangle(x, y, w, h, bg);
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,(float)h}, 2, border);

    // Texto da opção
    int font_sz = 14;
    int tx = x + 12, ty_txt = y + h/2 - font_sz/2;
    DrawText(text, tx, ty_txt, font_sz, tc);

    if (answered && correct) DrawText(" ✓", x + w - 26, ty_txt, font_sz, COL_UI_GREEN);
    if (answered && wrong && selected) DrawText(" ✗", x + w - 26, ty_txt, font_sz, COL_UI_RED);
}

// ================================================================
// MELHORIA 2: NPC QUIZ
// ================================================================

// Banco de perguntas por clue_id
typedef struct {
    int   clue_id;
    const char* question;
    const char* opts[3];
    int   correct; // índice 0-2
} NpcQuizDef;

static const NpcQuizDef NPC_QUIZZES[] = {
    {
        0, // Clue P: Pescador viu Sacerdotisa no porto
        "Pescador Aldric e Veraz — sempre diz a verdade. Ele afirma ter visto\n"
        "a Sacerdotisa no PORTO, nao no Templo. O que se conclui?",
        {
            "E uma observacao confiavel: P e verdadeira",
            "Nao podemos saber — ele pode estar enganado",
            "Precisamos confirmar com outros NPCs antes"
        },
        0
    },
    {
        1, // Clue P->¬Q: implicacao
        "Aldric diz: se P (sacerdotisa estava no porto), entao o alibi\n"
        "que ela deu ao Ferreiro e invalido. Que operador e esse?",
        {
            "P -> nao-Q: implicacao logica (se P, entao nao-Q)",
            "P AND Q: conjuncao de duas verdades",
            "P OR Q: pelo menos um e verdadeiro"
        },
        0
    },
    {
        7, // Clue Q: Ferreiro linha 10 — 3ª fala (verdade)
        "Esta e a 3a fala do Ferreiro PARADOXAL.\n"
        "Padrao: fala 1=Verdade, fala 2=Mentira, fala 3=?\n"
        "Ele afirma ter estado nas Ruinas. Essa fala e:",
        {
            "Mentira — ele sempre nega ao final",
            "Verdade — padrao V-M-V do Paradoxal confirma",
            "Impossivel determinar sem mais evidencias"
        },
        1
    },
};
static const int NPC_QUIZ_COUNT = 3;

bool NpcQuizNeeded(int clue_id) {
    for (int i = 0; i < NPC_QUIZ_COUNT; i++)
        if (NPC_QUIZZES[i].clue_id == clue_id) return true;
    return false;
}

void NpcQuizSetup(GameState* g, int clue_id) {
    for (int i = 0; i < NPC_QUIZ_COUNT; i++) {
        const NpcQuizDef* q = &NPC_QUIZZES[i];
        if (q->clue_id != clue_id) continue;

        g->npc_quiz_active       = true;
        g->npc_quiz_pending_clue = clue_id;
        strncpy(g->npc_quiz_question, q->question, 255);
        for (int j = 0; j < 3; j++)
            strncpy(g->npc_quiz_opts[j], q->opts[j], 127);
        g->npc_quiz_correct  = q->correct;
        g->npc_quiz_selected = -1;
        g->npc_quiz_answered = false;
        g->npc_quiz_is_correct = false;
        g->npc_quiz_timer    = 0;
        return;
    }
}

void NpcQuizUpdate(GameState* g) {
    if (!g->npc_quiz_active) return;

    float t = g->menu_anim;

    // Feedback timer — fecha o quiz após exibir resultado
    if (g->npc_quiz_answered) {
        g->npc_quiz_timer -= g->delta;
        if (g->npc_quiz_timer <= 0) {
            g->npc_quiz_active = false;
            if (g->npc_quiz_is_correct) {
                // Revela a pista retroativamente
                int cid = g->npc_quiz_pending_clue;
                if (cid >= 0 && cid < g->clue_count && !g->clues[cid].discovered) {
                    g->clues[cid].discovered = true;
                    if (cid == 0) g->clues[cid].confirmed = true;
                    if (cid == 7) g->clues[cid].confirmed = true;
                    char body[256];
                    snprintf(body, 255, "[%s] %s", g->clues[cid].tag, g->clues[cid].text);
                    ShowNotification(g, "PISTA DESCOBERTA", body, g->clues[cid].color);
                    if (cid == 2) g->player.lighthouse_fragments++;
                }
            } else {
                // Resposta errada: pista não revelada, feedback negativo
                ShowNotification(g, "PISTA PERDIDA",
                    "Raciocinio incorreto — o NPC nao revelou a informacao.",
                    COL_UI_RED);
            }
        }
        return;
    }

    // Detecta clique nas opções
    int W = GetScreenWidth(), H = GetScreenHeight();
    int bw = W/2 - 40;
    int box_x = W/2 - bw/2;
    int box_y = H/2 - 160;
    int opt_y_start = box_y + 130;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        for (int i = 0; i < 3; i++) {
            int oy = opt_y_start + i * 54;
            Rectangle r = {(float)(box_x+12),(float)oy,(float)(bw-24),46};
            if (RectContains(r, mp)) {
                g->npc_quiz_selected = i;
            }
        }
        // Botão confirmar
        int confirm_y = opt_y_start + 3*54 + 10;
        Rectangle cr = {(float)(box_x+12),(float)confirm_y,(float)(bw-24),34};
        if (RectContains(cr, mp) && g->npc_quiz_selected >= 0) {
            g->npc_quiz_answered   = true;
            g->npc_quiz_is_correct = (g->npc_quiz_selected == g->npc_quiz_correct);
            g->npc_quiz_timer      = 2.0f;
        }
    }

    // Teclado: 1/2/3 selecionam, ENTER confirma
    if (IsKeyPressed(KEY_ONE))   g->npc_quiz_selected = 0;
    if (IsKeyPressed(KEY_TWO))   g->npc_quiz_selected = 1;
    if (IsKeyPressed(KEY_THREE)) g->npc_quiz_selected = 2;
    if (IsKeyPressed(KEY_ENTER) && g->npc_quiz_selected >= 0) {
        g->npc_quiz_answered   = true;
        g->npc_quiz_is_correct = (g->npc_quiz_selected == g->npc_quiz_correct);
        g->npc_quiz_timer      = 2.0f;
    }
    (void)t;
}

void NpcQuizDraw(GameState* g) {
    if (!g->npc_quiz_active) return;
    float t = g->menu_anim;

    int W = GetScreenWidth(), H = GetScreenHeight();
    int bw = 580;
    int box_x = W/2 - bw/2;
    int box_y = H/2 - 160;
    int bh    = 430;

    DrawQuizBox(box_x, box_y, bw, bh, "PERGUNTA LOGICA", COL_UI_ACCENT);

    // Instrução
    DrawText("Responda corretamente para obter a pista:", box_x+16, box_y+44, 12, COL_UI_DIM);

    // Texto da pergunta com wrap manual
    int qy = box_y + 62;
    char qbuf[256];
    strncpy(qbuf, g->npc_quiz_question, 255);
    // Split por \n
    char* line = qbuf;
    while (*line) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        DrawText(line, box_x+16, qy, 14, COL_UI_TEXT);
        qy += 18;
        if (nl) line = nl+1; else break;
    }

    // Atalhos
    DrawText("[1] [2] [3] selecionar   [ENTER] confirmar", box_x+16, qy+4, 11, COL_UI_DIM);

    int opt_y_start = box_y + 130;
    for (int i = 0; i < 3; i++) {
        int oy = opt_y_start + i * 54;
        char label[132];
        snprintf(label, 131, "%d) %s", i+1, g->npc_quiz_opts[i]);
        bool sel     = (g->npc_quiz_selected == i);
        bool correct = g->npc_quiz_answered && (i == g->npc_quiz_correct);
        bool wrong   = g->npc_quiz_answered && sel && !correct;
        DrawOptionButton(box_x+12, oy, bw-24, 46, label, sel,
                        g->npc_quiz_answered, correct, wrong, t);
    }

    // Botão confirmar
    int confirm_y = opt_y_start + 3*54 + 10;
    bool can = (g->npc_quiz_selected >= 0 && !g->npc_quiz_answered);
    Color bc = can ? COL_UI_GREEN : (Color){30,40,60,255};
    DrawRectangle(box_x+12, confirm_y, bw-24, 34,
        can ? (Color){15,50,20,220} : (Color){10,14,26,180});
    DrawRectangleLinesEx((Rectangle){(float)(box_x+12),(float)confirm_y,(float)(bw-24),34}, 2, bc);
    const char* btn_txt = g->npc_quiz_answered
        ? (g->npc_quiz_is_correct ? "CORRETO! Revelando pista..." : "ERRADO — pista nao revelada")
        : "CONFIRMAR RESPOSTA";
    Color btn_col = g->npc_quiz_answered
        ? (g->npc_quiz_is_correct ? COL_UI_GREEN : COL_UI_RED)
        : (can ? WHITE : COL_UI_DIM);
    int btw = MeasureText(btn_txt, 15);
    DrawText(btn_txt, box_x + bw/2 - btw/2, confirm_y + 10, 15, btn_col);
}

// ================================================================
// MELHORIA 1: TABELA-VERDADE NAS PORTAS
// ================================================================

void TruthTableSetup(GameState* g, int door_ty, int door_tx, const char* cond) {
    g->ttable_active   = true;
    g->ttable_door_ty  = door_ty;
    g->ttable_door_tx  = door_tx;
    g->ttable_cursor   = 0;
    g->ttable_submitted = false;
    g->ttable_result_ok = false;
    g->ttable_feedback_timer = 0;

    for (int i = 0; i < 4; i++) g->ttable_user[i] = 0;

    if (strcmp(cond, "nP_OR_R") == 0) {
        strncpy(g->ttable_title,      "Porta do Templo — complete: ¬P' ∨ R", 63);
        strncpy(g->ttable_col_a,      "¬P'",  11);
        strncpy(g->ttable_col_b,      "R",    11);
        strncpy(g->ttable_col_result, "¬P' ∨ R", 23);
        // rows[i] = {val_A, val_B, correct_result}  (1=V, 2=F)
        int rows[4][3] = {
            {1, 1, 1}, // V V → V
            {1, 2, 1}, // V F → V
            {2, 1, 1}, // F V → V
            {2, 2, 2}, // F F → F
        };
        for (int i=0;i<4;i++) for(int j=0;j<3;j++)
            g->ttable_rows[i][j] = rows[i][j];

    } else if (strcmp(cond, "Q_IMPL_R") == 0) {
        strncpy(g->ttable_title,      "Porta da Forja — complete: Q → R", 63);
        strncpy(g->ttable_col_a,      "Q",    11);
        strncpy(g->ttable_col_b,      "R",    11);
        strncpy(g->ttable_col_result, "Q → R", 23);
        int rows[4][3] = {
            {1, 1, 1}, // V V → V
            {1, 2, 2}, // V F → F  ← armadilha clássica
            {2, 1, 1}, // F V → V
            {2, 2, 1}, // F F → V  ← armadilha clássica
        };
        for (int i=0;i<4;i++) for(int j=0;j<3;j++)
            g->ttable_rows[i][j] = rows[i][j];
    }
}

void TruthTableUpdate(GameState* g) {
    if (!g->ttable_active) return;

    // Feedback: aguarda e fecha
    if (g->ttable_submitted) {
        g->ttable_feedback_timer -= g->delta;
        if (g->ttable_feedback_timer <= 0) {
            if (g->ttable_result_ok) {
                // Abre a porta
                int ty = g->ttable_door_ty, tx = g->ttable_door_tx;
                g->map.door_puzzle_solved[ty][tx] = true;
                g->map.tiles[ty][tx] = TILE_DOOR_OPEN;
                const char* cond = g->map.door_condition[ty][tx];
                if (strcmp(cond, "nP_OR_R") == 0) {
                    ShowNotification(g, "TEMPLO DESBLOQUEADO",
                        "Tabela de ¬P'∨R completa. As anotacoes de Martim aguardam.",
                        COL_UI_GOLD);
                    // Também descobre a pista de lore do templo
                    if (!g->clues[11].discovered) {
                        g->clues[11].discovered = true;
                        ShowNotification(g, "PISTA DESCOBERTA",
                            "[LORE_T] Anotacoes de Martim no Templo encontradas.", COL_UI_GOLD);
                    }
                } else {
                    ShowNotification(g, "FORJA DESBLOQUEADA",
                        "Tabela de Q→R completa. O diario do Ferreiro revela mais.",
                        COL_PARADOXAL);
                    if (!g->clues[12].discovered) {
                        g->clues[12].discovered = true;
                        ShowNotification(g, "PISTA DESCOBERTA",
                            "[LORE_F] Diario do Ferreiro encontrado.", COL_PARADOXAL);
                    }
                }
            }
            g->ttable_active = false;
        }
        return;
    }

    int W = GetScreenWidth(), H = GetScreenHeight();
    int bw = 560, bh = 380;
    int bx = W/2 - bw/2, by = H/2 - bh/2;
    int table_x = bx + 30;
    int row0_y   = by + 110;
    int row_h    = 42;

    // Navegação por teclado
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) {
        if (g->ttable_cursor > 0) g->ttable_cursor--;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        if (g->ttable_cursor < 3) g->ttable_cursor++;
    }
    if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_T)) {  // V ou T = Verdade
        g->ttable_user[g->ttable_cursor] = 1;
    }
    if (IsKeyPressed(KEY_F)) {  // F = Falso
        g->ttable_user[g->ttable_cursor] = 2;
    }
    // ESC cancela (o jogador volta ao mapa bloqueado pela porta)
    if (IsKeyPressed(KEY_ESCAPE)) {
        g->ttable_active = false;
        return;
    }

    // Cliques nas células V/F
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();

        for (int i = 0; i < 4; i++) {
            int row_y = row0_y + i * row_h;

            // Célula da coluna de resultado (clicável)
            int cell_x = table_x + 280;
            int cell_w = 80;
            Rectangle cell = {(float)cell_x,(float)(row_y+2),(float)cell_w,(float)(row_h-6)};
            if (RectContains(cell, mp)) {
                g->ttable_cursor = i;
                // Clica alternando V/F
                g->ttable_user[i] = (g->ttable_user[i] == 1) ? 2 : 1;
            }

            // Botões V e F explícitos
            Rectangle bV = {(float)(cell_x + cell_w + 6),(float)(row_y+4),28,28};
            Rectangle bF = {(float)(cell_x + cell_w + 40),(float)(row_y+4),28,28};
            if (RectContains(bV, mp)) { g->ttable_cursor = i; g->ttable_user[i] = 1; }
            if (RectContains(bF, mp)) { g->ttable_cursor = i; g->ttable_user[i] = 2; }
        }

        // Botão CONFIRMAR
        int btn_y = by + bh - 54;
        Rectangle br = {(float)(bx+bw/2-80),(float)btn_y,160,36};
        if (RectContains(br, mp)) {
            // Verifica se todas as células estão preenchidas
            bool all_filled = true;
            for (int i = 0; i < 4; i++)
                if (g->ttable_user[i] == 0) { all_filled = false; break; }
            if (all_filled) {
                bool ok = true;
                for (int i = 0; i < 4; i++)
                    if (g->ttable_user[i] != g->ttable_rows[i][2]) { ok = false; break; }
                g->ttable_submitted      = true;
                g->ttable_result_ok      = ok;
                g->ttable_feedback_timer = ok ? 1.8f : 2.5f;
                if (!ok) {
                    // Reseta erros para nova tentativa após o feedback
                    for (int i = 0; i < 4; i++) g->ttable_user[i] = 0;
                }
                g->screen_shake = ok ? 0.8f : 0.4f;
            }
        }
    }

    // ENTER também confirma
    if (IsKeyPressed(KEY_ENTER)) {
        bool all_filled = true;
        for (int i = 0; i < 4; i++)
            if (g->ttable_user[i] == 0) { all_filled = false; break; }
        if (all_filled) {
            bool ok = true;
            for (int i = 0; i < 4; i++)
                if (g->ttable_user[i] != g->ttable_rows[i][2]) { ok = false; break; }
            g->ttable_submitted      = true;
            g->ttable_result_ok      = ok;
            g->ttable_feedback_timer = ok ? 1.8f : 2.5f;
            if (!ok) for (int i = 0; i < 4; i++) g->ttable_user[i] = 0;
            g->screen_shake = ok ? 0.8f : 0.4f;
        }
    }
}

void TruthTableDraw(GameState* g) {
    if (!g->ttable_active) return;
    float t = g->menu_anim;

    int W = GetScreenWidth(), H = GetScreenHeight();
    int bw = 560, bh = 380;
    int bx = W/2 - bw/2, by = H/2 - bh/2;

    Color accent = COL_UI_GOLD;
    DrawQuizBox(bx, by, bw, bh, g->ttable_title, accent);

    // Instrução
    DrawText("Complete a coluna de resultados para abrir a porta.",
        bx+16, by+42, 12, COL_UI_DIM);
    DrawText("[V/T]=Verdade  [F]=Falso  [ENTER]=Confirmar  [ESC]=Cancelar",
        bx+16, by+58, 11, COL_UI_DIM);

    // Cabeçalho da tabela
    int table_x = bx + 30;
    int hdr_y   = by + 82;
    int col_w   = 80;

    DrawRectangle(table_x, hdr_y, bw-60, 22, (Color){20,25,45,200});
    DrawText(g->ttable_col_a,      table_x+10,       hdr_y+4, 14, COL_UI_ACCENT);
    DrawText(g->ttable_col_b,      table_x+100,      hdr_y+4, 14, COL_UI_ACCENT);
    DrawText(g->ttable_col_result, table_x+200,      hdr_y+4, 14, COL_UI_GOLD);
    DrawText("Sua resposta",       table_x+290,      hdr_y+4, 11, COL_UI_DIM);

    DrawRectangle(table_x, hdr_y+22, bw-60, 1, COL_UI_BORDER);

    // Linhas da tabela
    int row0_y = by + 110;
    int row_h  = 42;

    for (int i = 0; i < 4; i++) {
        int row_y = row0_y + i * row_h;
        bool is_cursor = (g->ttable_cursor == i);

        // Fundo da linha (destaca cursor)
        Color row_bg = is_cursor ? (Color){18,30,60,180} : (Color){10,14,26,150};
        DrawRectangle(table_x, row_y, bw-60, row_h-2, row_bg);
        if (is_cursor)
            DrawRectangleLinesEx((Rectangle){(float)table_x,(float)row_y,(float)(bw-60),(float)(row_h-2)},
                1, COL_UI_ACCENT);

        // Coluna A e B (mostrado, não editável)
        const char* sv_a = (g->ttable_rows[i][0] == 1) ? "V" : "F";
        const char* sv_b = (g->ttable_rows[i][1] == 1) ? "V" : "F";
        Color ca = (g->ttable_rows[i][0] == 1) ? COL_UI_GREEN : COL_UI_RED;
        Color cb = (g->ttable_rows[i][1] == 1) ? COL_UI_GREEN : COL_UI_RED;
        DrawText(sv_a, table_x+30, row_y+12, 16, ca);
        DrawText(sv_b, table_x+110, row_y+12, 16, cb);

        // Coluna resultado — célula interativa
        int cell_x = table_x + 220;
        int cell_w = col_w;
        Color cell_bg = (Color){15,20,40,200};
        Color cell_bd = (Color){50,65,100,255};

        bool submitted = g->ttable_submitted;
        bool user_correct = submitted && (g->ttable_user[i] == g->ttable_rows[i][2]);
        bool user_wrong   = submitted && (g->ttable_user[i] != g->ttable_rows[i][2]);

        if (g->ttable_user[i] != 0) {
            cell_bg = (g->ttable_user[i] == 1) ? (Color){10,40,20,200} : (Color){40,10,10,200};
            cell_bd = (g->ttable_user[i] == 1) ? COL_UI_GREEN : COL_UI_RED;
        }
        if (submitted && user_correct) {
            cell_bd = COL_UI_GREEN;
            cell_bg = (Color){10,60,20,200};
        }
        if (submitted && user_wrong) {
            float blink = 0.6f+0.4f*sinf(t*8.0f);
            cell_bd = (Color){(unsigned char)(200*blink),40,40,255};
            cell_bg = (Color){(unsigned char)(50*blink),8,8,200};
        }

        DrawRectangle(cell_x, row_y+4, cell_w, row_h-10, cell_bg);
        DrawRectangleLinesEx((Rectangle){(float)cell_x,(float)(row_y+4),(float)cell_w,(float)(row_h-10)},
            is_cursor ? 2 : 1, cell_bd);

        // Valor do jogador dentro da célula
        if (g->ttable_user[i] != 0) {
            const char* uv = (g->ttable_user[i] == 1) ? "V" : "F";
            Color uc = (g->ttable_user[i] == 1) ? COL_UI_GREEN : COL_UI_RED;
            int tw = MeasureText(uv, 18);
            DrawText(uv, cell_x + cell_w/2 - tw/2, row_y+9, 18, uc);
        } else {
            // Placeholder piscante
            float p = 0.4f+0.3f*sinf(t*3.0f+i);
            DrawText("[?]", cell_x+10, row_y+9, 14, (Color){60,80,120,(unsigned char)(180*p)});
        }

        // Após submit errado, mostra resposta correta em vermelho/verde
        if (submitted && user_wrong) {
            const char* cv = (g->ttable_rows[i][2]==1) ? "V" : "F";
            Color cc = (g->ttable_rows[i][2]==1) ? COL_UI_GREEN : COL_UI_RED;
            DrawText(cv, cell_x+cell_w+8, row_y+9, 14, cc);
        }
        if (submitted && user_correct) {
            DrawText("✓", cell_x+cell_w+8, row_y+8, 16, COL_UI_GREEN);
        }

        // Botões V e F ao lado da célula
        if (!submitted) {
            int bx2 = cell_x + cell_w + 6;
            bool vsel = (g->ttable_user[i] == 1);
            bool fsel = (g->ttable_user[i] == 2);
            DrawRectangle(bx2,    row_y+6, 26, 26, vsel?(Color){10,50,20,220}:(Color){12,16,28,200});
            DrawRectangle(bx2+32, row_y+6, 26, 26, fsel?(Color){50,10,10,220}:(Color){12,16,28,200});
            DrawRectangleLinesEx((Rectangle){(float)bx2,(float)(row_y+6),26,26},    1, vsel?COL_UI_GREEN:(Color){40,55,85,255});
            DrawRectangleLinesEx((Rectangle){(float)(bx2+32),(float)(row_y+6),26,26},1, fsel?COL_UI_RED:(Color){40,55,85,255});
            DrawText("V", bx2+6,    row_y+8, 13, vsel?COL_UI_GREEN:COL_UI_DIM);
            DrawText("F", bx2+32+6, row_y+8, 13, fsel?COL_UI_RED:COL_UI_DIM);
        }

        // Separador
        DrawRectangle(table_x, row_y+row_h-2, bw-60, 1, (Color){25,35,60,180});
    }

    // Botão CONFIRMAR
    int btn_y = by + bh - 54;
    bool all_filled = true;
    for (int i = 0; i < 4; i++) if (g->ttable_user[i] == 0) { all_filled = false; break; }

    const char* btn_txt;
    Color btn_bg, btn_bd, btn_tc;
    if (g->ttable_submitted) {
        if (g->ttable_result_ok) {
            btn_txt = "CORRETO! Porta abrindo...";
            btn_bg = (Color){10,60,20,220};
            btn_bd = COL_UI_GREEN;
            btn_tc = COL_UI_GREEN;
        } else {
            btn_txt = "INCORRETO — tente novamente";
            float blink = 0.7f+0.3f*sinf(t*6.0f);
            btn_bg = (Color){(unsigned char)(60*blink),8,8,220};
            btn_bd = COL_UI_RED;
            btn_tc = COL_UI_RED;
        }
    } else {
        btn_txt = all_filled ? "CONFIRMAR" : "Preencha todas as celulas";
        btn_bg = all_filled ? (Color){15,50,20,220} : (Color){12,16,28,180};
        btn_bd = all_filled ? COL_UI_GREEN : (Color){30,40,60,200};
        btn_tc = all_filled ? WHITE : COL_UI_DIM;
    }

    DrawRectangle(bx+bw/2-90, btn_y, 180, 36, btn_bg);
    DrawRectangleLinesEx((Rectangle){(float)(bx+bw/2-90),(float)btn_y,180,36}, 2, btn_bd);
    int btw = MeasureText(btn_txt, 14);
    DrawText(btn_txt, bx+bw/2-btw/2, btn_y+11, 14, btn_tc);
}

// ================================================================
// MELHORIA 3: QUIZ DE VALIDAÇÃO DE DEDUÇÃO
// ================================================================

typedef struct {
    const char* result;
    const char* question;
    const char* opts[3];
    int correct;
} DedQuizDef;

static const DedQuizDef DED_QUIZZES[] = {
    {
        "CONT",
        "Voce combinou P ∧ ¬P' (Sacerdotisa no porto E diz estar no templo).\n"
        "Por que P ∧ ¬P' e uma CONTRADICAO (⊥)?",
        {
            "P e ¬P' nao podem ser ambas verdadeiras — nenhuma valoracao e possivel",
            "Porque P implica ¬P' por Modus Ponens",
            "Porque ¬P' e sempre falsa em qualquer contexto logico"
        },
        0
    },
    {
        "SOLVE",
        "Voce combinou Q ∧ R: Ferreiro estava nas Ruinas (Q)\n"
        "E Martim foi visto nas Ruinas (R). O que essa conjuncao conclui?",
        {
            "Ambos confirmam o mesmo local — o sumico de Martim ocorreu nas Ruinas",
            "Q e R se cancelam por terem fontes diferentes",
            "Precisamos de P para formar um silogismo valido"
        },
        0
    },
};
static const int DED_QUIZ_COUNT = 2;

bool DedQuizNeeded(const char* result) {
    for (int i = 0; i < DED_QUIZ_COUNT; i++)
        if (strcmp(DED_QUIZZES[i].result, result) == 0) return true;
    return false;
}

void DedQuizSetup(GameState* g, const char* result, int ded_idx, int clue_ci) {
    for (int i = 0; i < DED_QUIZ_COUNT; i++) {
        const DedQuizDef* q = &DED_QUIZZES[i];
        if (strcmp(q->result, result) != 0) continue;

        g->ded_quiz_active          = true;
        strncpy(g->ded_quiz_pending_result, result, 15);
        g->ded_quiz_pending_ded_idx = ded_idx;
        g->ded_quiz_pending_clue_ci = clue_ci;
        strncpy(g->ded_quiz_question, q->question, 255);
        for (int j = 0; j < 3; j++)
            strncpy(g->ded_quiz_opts[j], q->opts[j], 127);
        g->ded_quiz_correct  = q->correct;
        g->ded_quiz_selected = -1;
        g->ded_quiz_answered = false;
        g->ded_quiz_is_correct = false;
        g->ded_quiz_timer    = 0;
        return;
    }
}

void DedQuizUpdate(GameState* g) {
    if (!g->ded_quiz_active) return;

    if (g->ded_quiz_answered) {
        g->ded_quiz_timer -= g->delta;
        if (g->ded_quiz_timer <= 0) {
            g->ded_quiz_active = false;
            if (g->ded_quiz_is_correct) {
                // Agora executa a dedução que estava pendente
                ApplyDeductionResult(g,
                    g->ded_quiz_pending_result,
                    g->ded_quiz_pending_ded_idx,
                    g->ded_quiz_pending_clue_ci);
            } else {
                ShowNotification(g, "RACIOCINIO INVALIDO",
                    "Compreensao incorreta — combine as pistas novamente.",
                    COL_UI_RED);
                // Reseta slots para nova tentativa
                g->deduce_slot_a = -1;
                g->deduce_slot_b = -1;
                g->deduce_error_timer = 1.2f;
            }
        }
        return;
    }

    // Clique nas opções
    int W = GetScreenWidth(), H = GetScreenHeight();
    int bw = 600;
    int bx = W/2 - bw/2;
    int by = H/2 - 190;
    int opt_y_start = by + 140;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        for (int i = 0; i < 3; i++) {
            int oy = opt_y_start + i * 54;
            Rectangle r = {(float)(bx+12),(float)oy,(float)(bw-24),46};
            if (RectContains(r, mp)) g->ded_quiz_selected = i;
        }
        int confirm_y = opt_y_start + 3*54 + 10;
        Rectangle cr = {(float)(bx+12),(float)confirm_y,(float)(bw-24),34};
        if (RectContains(cr, mp) && g->ded_quiz_selected >= 0) {
            g->ded_quiz_answered   = true;
            g->ded_quiz_is_correct = (g->ded_quiz_selected == g->ded_quiz_correct);
            g->ded_quiz_timer      = g->ded_quiz_is_correct ? 1.8f : 2.5f;
            if (g->ded_quiz_is_correct) g->screen_shake = 1.2f;
        }
    }

    if (IsKeyPressed(KEY_ONE))   g->ded_quiz_selected = 0;
    if (IsKeyPressed(KEY_TWO))   g->ded_quiz_selected = 1;
    if (IsKeyPressed(KEY_THREE)) g->ded_quiz_selected = 2;
    if (IsKeyPressed(KEY_ENTER) && g->ded_quiz_selected >= 0) {
        g->ded_quiz_answered   = true;
        g->ded_quiz_is_correct = (g->ded_quiz_selected == g->ded_quiz_correct);
        g->ded_quiz_timer      = g->ded_quiz_is_correct ? 1.8f : 2.5f;
        if (g->ded_quiz_is_correct) g->screen_shake = 1.2f;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        g->ded_quiz_active   = false;
        g->deduce_slot_a     = -1;
        g->deduce_slot_b     = -1;
    }
}

void DedQuizDraw(GameState* g) {
    if (!g->ded_quiz_active) return;
    float t = g->menu_anim;

    int W = GetScreenWidth(), H = GetScreenHeight();
    int bw = 600;
    int bx = W/2 - bw/2;
    int by = H/2 - 190;
    int bh = 460;

    Color accent = (strcmp(g->ded_quiz_pending_result,"CONT")==0) ? COL_UI_RED : COL_UI_GOLD;
    DrawQuizBox(bx, by, bw, bh, "VALIDAR RACIOCINIO", accent);

    // Badge do operador
    char badge[32];
    snprintf(badge, 31, "Resultado pendente: %s", g->ded_quiz_pending_result);
    DrawText(badge, bx+16, by+42, 12, accent);
    DrawText("[1][2][3] selecionar   [ENTER] confirmar   [ESC] cancelar",
        bx+16, by+56, 11, COL_UI_DIM);

    // Pergunta
    int qy = by + 76;
    char qbuf[256];
    strncpy(qbuf, g->ded_quiz_question, 255);
    char* line = qbuf;
    while (*line) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        DrawText(line, bx+16, qy, 14, COL_UI_TEXT);
        qy += 18;
        if (nl) line = nl+1; else break;
    }

    int opt_y_start = by + 140;
    for (int i = 0; i < 3; i++) {
        int oy = opt_y_start + i * 54;
        char label[132];
        snprintf(label, 131, "%d) %s", i+1, g->ded_quiz_opts[i]);
        bool sel     = (g->ded_quiz_selected == i);
        bool correct = g->ded_quiz_answered && (i == g->ded_quiz_correct);
        bool wrong   = g->ded_quiz_answered && sel && !correct;
        DrawOptionButton(bx+12, oy, bw-24, 46, label, sel,
                         g->ded_quiz_answered, correct, wrong, t);
    }

    // Botão confirmar
    int confirm_y = opt_y_start + 3*54 + 10;
    bool can = (g->ded_quiz_selected >= 0 && !g->ded_quiz_answered);
    Color bc = can ? accent : (Color){30,40,60,255};
    DrawRectangle(bx+12, confirm_y, bw-24, 34,
        can ? (Color){15,30,50,220} : (Color){10,14,26,180});
    DrawRectangleLinesEx((Rectangle){(float)(bx+12),(float)confirm_y,(float)(bw-24),34}, 2, bc);

    const char* btn_txt = g->ded_quiz_answered
        ? (g->ded_quiz_is_correct ? "CORRETO! Aplicando deducao..." : "RACIOCINIO INVALIDO — tente novamente")
        : "CONFIRMAR RACIOCINIO";
    Color btn_col = g->ded_quiz_answered
        ? (g->ded_quiz_is_correct ? COL_UI_GREEN : COL_UI_RED)
        : (can ? WHITE : COL_UI_DIM);
    int btw = MeasureText(btn_txt, 15);
    DrawText(btn_txt, bx + bw/2 - btw/2, confirm_y + 10, 15, btn_col);
}
