#include "game.h"

// =============================================================
// ValidCombo — estrutura que define uma combinação lógica válida
// =============================================================
// Cada entrada na tabela descreve: "se o jogador combinar tag_a
// com tag_b usando o operador op, a conclusão 'result' é gerada."
typedef struct {
    const char* tag_a;   // Tag da primeira premissa (ex: "P")
    int         op;      // Operador: 0=∧(AND) 1=∨(OR) 2=→(IMPLICA) 3=¬(NEG)
    const char* tag_b;   // Tag da segunda premissa (ignorado quando op==3)
    const char* result;  // Tag da conclusão resultante
    int         ded_idx; // Índice em g->deductions[] a desbloquear (-1 = só confirma pista)
} ValidCombo;

// =============================================================
// VALID_COMBOS — tabela completa de deduções possíveis no jogo
// =============================================================
// op codes: 0=∧ (E)   1=∨ (OU)   2=→ (IMPLICA)   3=¬ (NÃO)
static const ValidCombo VALID_COMBOS[] = {
    // P ∧ ¬P' → CONT
    // Sacerdotisa no porto (P) E ela diz que estava no templo (¬P') = CONTRADIÇÃO
    { "P",     0, "nP'",  "CONT",     0 },

    // P → ¬Q_alibi
    // Se sacerdotisa estava no porto, o alibi que ela deu ao Ferreiro é inválido
    { "P",     2, "nP'",  "nQ_a",     1 },

    // K_PAR ∧ Q → Q_conf
    // Sabendo o padrão do Ferreiro (K_PAR) e que ele afirmou Q, Q está confirmado
    { "K_PAR", 0, "Q",    "Q_conf",   2 },

    // Q ∧ R → SOLVE
    // Ferreiro estava nas Ruínas (Q) E Martim foi visto lá (R) = SOLUÇÃO DO MISTÉRIO
    { "Q",     0, "R",    "SOLVE",    3 },

    // ¬P' ∨ R → nP_OR_R
    // Dedução que desbloqueia a Porta A do Templo (não conta como dedução no caderno)
    { "nP'",   1, "R",    "nP_OR_R", -1 },

    // Q → R — desbloqueia a Porta B da Forja
    { "Q",     2, "R",    "Q_IMPL_R",-1 },
};
static const int VALID_COMBO_COUNT = 6;

// Nomes dos operadores para exibição na interface
static const char* OP_SYMBOLS[] = { "∧", "∨", "→", "¬" };

// =============================================================
// TryManualDeduction — tenta realizar a dedução escolhida pelo jogador
// =============================================================
// Chamada quando o jogador clica em "DEDUZIR" ou pressiona ENTER
// no Caderno de Deduções. Verifica se a combinação (A op B) é válida.
static void TryManualDeduction(GameState* g) {
    int a  = g->deduce_slot_a; // Índice da pista A selecionada (-1 = vazio)
    int b  = g->deduce_slot_b; // Índice da pista B selecionada (-1 = vazio)
    int op = g->deduce_op;     // Operador escolhido (0=∧ 1=∨ 2=→ 3=¬)

    // Valida se as pistas existem e foram descobertas pelo jogador
    if (a < 0 || a >= g->clue_count || !g->clues[a].discovered) return;
    // Negação (op==3) usa só uma pista; os outros operadores precisam das duas
    if (op != 3 && (b < 0 || b >= g->clue_count || !g->clues[b].discovered)) return;

    const char* tag_a = g->clues[a].tag;
    const char* tag_b = (op == 3) ? "" : g->clues[b].tag;

    // Percorre a tabela procurando uma combinação que bata com a escolha do jogador
    for (int i = 0; i < VALID_COMBO_COUNT; i++) {
        const ValidCombo* c = &VALID_COMBOS[i];
        if (c->op != op) continue; // Operador deve corresponder

        bool match = false;
        if (op == 3) {
            // Negação usa apenas tag_a
            match = (strcmp(tag_a, c->tag_a) == 0);
        } else {
            if (op == 0 || op == 1) {
                // AND e OR são comutativos: A∧B = B∧A, então testa as duas ordens
                match = (strcmp(tag_a, c->tag_a) == 0 && strcmp(tag_b, c->tag_b) == 0)
                     || (strcmp(tag_b, c->tag_a) == 0 && strcmp(tag_a, c->tag_b) == 0);
            } else {
                // IMPLICA NÃO é comutativo: A→B ≠ B→A, testa só a ordem correta
                match = (strcmp(tag_a, c->tag_a) == 0 && strcmp(tag_b, c->tag_b) == 0);
            }
        }

        if (!match) continue; // Não bateu: testa a próxima combinação da tabela

        // ✓ COMBINAÇÃO VÁLIDA! Desbloqueia a dedução no caderno se houver uma associada
        if (c->ded_idx >= 0 && c->ded_idx < g->deduction_count) {
            if (!g->deductions[c->ded_idx].unlocked) {
                g->deductions[c->ded_idx].unlocked = true;
            }
        }

        // Marca como confirmada a pista de conclusão (busca pelo tag resultado)
        for (int ci = 0; ci < g->clue_count; ci++) {
            if (strcmp(g->clues[ci].tag, c->result) == 0) {
                g->clues[ci].confirmed = true;
                break;
            }
        }

        // MELHORIA 3: para CONT e SOLVE, intercala quiz antes de aplicar
        if (DedQuizNeeded(c->result)) {
            // Encontra o índice da pista de conclusão
            int clue_ci = -1;
            for (int ci = 0; ci < g->clue_count; ci++) {
                if (strcmp(g->clues[ci].tag, c->result) == 0) { clue_ci = ci; break; }
            }
            DedQuizSetup(g, c->result, c->ded_idx, clue_ci);
            // Não limpa os slots ainda — limpa após o quiz
            return;
        }

        // Para as outras deduções, aplica direto
        int clue_ci = -1;
        for (int ci = 0; ci < g->clue_count; ci++) {
            if (strcmp(g->clues[ci].tag, c->result) == 0) { clue_ci = ci; break; }
        }
        ApplyDeductionResult(g, c->result, c->ded_idx, clue_ci);

        // Limpa os slots após dedução bem-sucedida para nova tentativa
        g->deduce_slot_a = -1;
        g->deduce_slot_b = -1;

        CheckPuzzleSolution(g); // Verifica se o jogo foi totalmente concluído
        return;
    }

    // Nenhuma combinação válida encontrada: feedback de erro piscante
    g->deduce_error_timer = 1.2f;
    ShowNotification(g, "DEDUÇÃO INVÁLIDA",
        "Essa combinação não produz uma conclusão lógica.", COL_UI_RED);
}

// =============================================================
// CheckPuzzleSolution — verifica o estado global do puzzle
// =============================================================
// Chamada após cada dedução para checar se o jogo pode avançar
void CheckPuzzleSolution(GameState* g) {
    bool has_P = g->clues[0].discovered && g->clues[0].confirmed; // Sacerdotisa no porto
    bool has_Q = g->clues[7].discovered && g->clues[7].confirmed; // Ferreiro nas Ruínas
    bool has_R = g->clues[8].discovered;                          // Martim nas Ruínas

    // Detecta contradição P ∧ ¬P' automaticamente quando ambas pistas são descobertas
    bool has_nP = g->clues[3].discovered; // ¬P' = declaração da Sacerdotisa
    if (has_P && has_nP && !g->contradiction_active) {
        g->contradiction_active = true;
    }

    // Ativa o Farol quando o jogador já coletou os 3 fragmentos
    if (g->player.lighthouse_fragments >= 3 && !g->lighthouse_activated) {
        g->lighthouse_activated = true;
        g->clues[10].discovered = true;
        ShowNotification(g, "FAROL ATIVADO!",
            "O Código do Farol está completo. Um navio se aproxima...", COL_LIGHTHOUSE);
    }

    // Se o mistério foi resolvido e todas as 3 pistas principais estão confirmadas,
    // garante que o fragmento final seja concedido ao jogador
    if (g->mystery_solved && has_P && has_Q && has_R
        && g->player.lighthouse_fragments < 3) {
        g->player.lighthouse_fragments = 3;
        g->clues[10].discovered = true;
        ShowNotification(g, "FRAGMENTO FINAL",
            "O Código do Farol está completo!", COL_LIGHTHOUSE);
    }
}

// =============================================================
// ApplyDeductionResult — aplica os efeitos de uma dedução confirmada
// =============================================================
// Chamada tanto por TryManualDeduction (deduções sem quiz) quanto
// por DedQuizUpdate (após o jogador responder corretamente ao quiz).
void ApplyDeductionResult(GameState* g, const char* result, int ded_idx, int clue_ci) {
    // Desbloqueia a dedução no caderno
    if (ded_idx >= 0 && ded_idx < g->deduction_count) {
        if (!g->deductions[ded_idx].unlocked)
            g->deductions[ded_idx].unlocked = true;
    }
    // Confirma a pista de conclusão
    if (clue_ci >= 0 && clue_ci < g->clue_count)
        g->clues[clue_ci].confirmed = true;

    // Efeitos específicos por resultado
    if (strcmp(result, "CONT") == 0) {
        ShowNotification(g, "CONTRADICAO ENCONTRADA",
            "P ^ ¬P' — A Sacerdotisa mente sobre seu alibi!", COL_UI_RED);
        g->contradiction_active = true;
        g->screen_shake = 1.0f;
    } else if (strcmp(result, "SOLVE") == 0) {
        ShowNotification(g, "MISTERIO SOLUCIONADO!",
            "Martim desapareceu nas Ruinas. O Codigo do Farol esta la.", COL_UI_GOLD);
        g->mystery_solved = true;
        g->clues[7].confirmed = true;
        g->clues[9].discovered = true;
        g->player.lighthouse_fragments = 2;
        g->screen_shake = 1.5f;
    } else if (strcmp(result, "Q_conf") == 0) {
        ShowNotification(g, "PADRAO IDENTIFICADO",
            "Ferreiro Paradoxal: fala de dia = verdade confirmada.", COL_PARADOXAL);
        g->clues[7].confirmed = true;
    } else if (strcmp(result, "nP_OR_R") == 0) {
        ShowNotification(g, "DEDUCAO: ¬P' v R",
            "Acesse o Templo e resolva o puzzle da porta para entrar.", COL_UI_GOLD);
    } else if (strcmp(result, "Q_IMPL_R") == 0) {
        ShowNotification(g, "DEDUCAO: Q -> R",
            "Acesse a Forja e resolva o puzzle da porta para entrar.", COL_PARADOXAL);
    } else if (strcmp(result, "nQ_a") == 0) {
        ShowNotification(g, "DEDUCAO CONFIRMADA",
            "O alibi do Ferreiro dado pela Sacerdotisa e invalido.", COL_UI_GOLD);
    } else {
        if (ded_idx >= 0)
            ShowNotification(g, "DEDUCAO CONFIRMADA",
                g->deductions[ded_idx].text, COL_UI_GOLD);
    }

    // Limpa slots
    g->deduce_slot_a = -1;
    g->deduce_slot_b = -1;
    CheckPuzzleSolution(g);
}

// =============================================================
// DeductionUpdate — processa input do jogador no Caderno de Deduções
// =============================================================
void DeductionUpdate(GameState* g) {
    // MELHORIA 3: se quiz ativo, só processa o quiz
    if (g->ded_quiz_active) {
        DedQuizUpdate(g);
        return;
    }

    // Decrementa o timer do feedback de erro vermelho
    if (g->deduce_error_timer > 0) {
        g->deduce_error_timer -= g->delta;
        if (g->deduce_error_timer < 0) g->deduce_error_timer = 0;
    }

    // Scroll da lista de pistas com setas ou WASD
    if (!g->deduce_panel_focus) {
        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) g->deduction_scroll--;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) g->deduction_scroll++;
        if (g->deduction_scroll < 0) g->deduction_scroll = 0;
    }

    // --- Detecção de cliques do mouse ---
    int W = GetScreenWidth();
    int lp_x = 10, lp_y = 80, lp_w = W/2 - 20; // Painel esquerdo (lista de pistas)
    int cy = lp_y + 40;
    int scroll_px = g->deduction_scroll * 60; // Offset de scroll em pixels

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();

        // Cliques nas pistas: primeiro clique = slot A, segundo clique = slot B
        for (int i = 0; i < g->clue_count; i++) {
            if (!g->clues[i].discovered) continue;
            int iy = cy + i * 60 - scroll_px;
            Rectangle r = {(float)(lp_x+8), (float)iy, (float)(lp_w-16), 52};
            if (RectContains(r, mp)) {
                if (g->deduce_slot_a == -1 || g->deduce_slot_a == i) {
                    // Alterna slot A (clica de novo = deseleciona)
                    g->deduce_slot_a = (g->deduce_slot_a == i) ? -1 : i;
                } else if (g->deduce_slot_b == -1 || g->deduce_slot_b == i) {
                    // Alterna slot B
                    g->deduce_slot_b = (g->deduce_slot_b == i) ? -1 : i;
                } else {
                    g->deduce_slot_b = i; // Ambos cheios: substitui slot B
                }
                break;
            }
        }

        // Cliques nos botões de operador (∧ ∨ → ¬)
        int rp_x = W/2 + 10, rp_y = 80, rp_w = W/2 - 20;
        int btn_y = rp_y + 170;
        for (int op = 0; op < 4; op++) {
            int btn_x = rp_x + 12 + op * ((rp_w-24)/4);
            int btn_w = (rp_w-24)/4 - 4;
            Rectangle br = {(float)btn_x, (float)btn_y, (float)btn_w, 28};
            if (RectContains(br, mp)) {
                g->deduce_op = op; // Seleciona o operador clicado
                break;
            }
        }

        // Clique no botão DEDUZIR
        int deduce_btn_y = rp_y + 215;
        Rectangle db = {(float)(rp_x+12), (float)deduce_btn_y, (float)(rp_w-24), 32};
        if (RectContains(db, mp)) {
            TryManualDeduction(g); // Tenta realizar a dedução
        }

        // Clique no botão LIMPAR (reseta os slots)
        Rectangle cb = {(float)(rp_x + rp_w/2 + 4), (float)deduce_btn_y, (float)(rp_w/2-16), 32};
        if (RectContains(cb, mp)) {
            g->deduce_slot_a = -1;
            g->deduce_slot_b = -1;
        }
    }

    // Atalhos de teclado
    if (IsKeyPressed(KEY_ONE))   g->deduce_op = 0; // Tecla 1 = ∧ (AND)
    if (IsKeyPressed(KEY_TWO))   g->deduce_op = 1; // Tecla 2 = ∨ (OR)
    if (IsKeyPressed(KEY_THREE)) g->deduce_op = 2; // Tecla 3 = → (IMPLICA)
    if (IsKeyPressed(KEY_FOUR))  g->deduce_op = 3; // Tecla 4 = ¬ (NEG)
    if (IsKeyPressed(KEY_ENTER)) TryManualDeduction(g); // ENTER = deduzir
    if (IsKeyPressed(KEY_C))     { g->deduce_slot_a = -1; g->deduce_slot_b = -1; } // C = limpar

    // Fechar o caderno e voltar ao jogo
    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_J) ||
        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
        g->current_screen = SCREEN_GAME;
    }
}

// =============================================================
// Helpers de desenho (funções internas de renderização)
// =============================================================

// Desenha um símbolo lógico (tag) com fundo escuro e borda colorida
static void DrawLogicSymbol(const char* tag, int x, int y, Color col) {
    int tw = MeasureText(tag, 14);
    DrawRectangle(x-2, y-2, tw+8, 18, (Color){col.r/4,col.g/4,col.b/4,200});
    DrawRectangleLinesEx((Rectangle){(float)(x-2),(float)(y-2),(float)(tw+8),18.0f}, 1, col);
    DrawText(tag, x+2, y, 14, col);
}

// Desenha um nó de dedução (premissa ou conclusão) na visualização do caderno
static void DrawDeductionNode(const char* label, int x, int y, Color col, bool filled) {
    int w = MeasureText(label, 13) + 14;
    if (filled) DrawRectangle(x, y, w, 24, (Color){col.r/3,col.g/3,col.b/3,255});
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,24.0f}, 1, col);
    DrawText(label, x+7, y+5, 13, col);
}

// Desenha um slot de seleção de pista (A ou B) no painel combinador
static void DrawDeduceSlot(GameState* g, int slot_idx, int x, int y, int w,
                            const char* label, float t) {
    bool empty = (slot_idx < 0 || slot_idx >= g->clue_count);
    Color border = empty ? (Color){40,50,70,255} : COL_UI_ACCENT;

    // Pisca vermelho se a última tentativa foi inválida
    if (g->deduce_error_timer > 0) {
        float blink = sinf(g->deduce_error_timer * 20.0f);
        if (blink > 0) border = COL_UI_RED;
    }

    DrawRectangle(x, y, w, 36, (Color){8,12,24,220});
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,36.0f}, 1, border);
    DrawText(label, x+6, y+2, 10, COL_UI_DIM);

    if (!empty) {
        Clue* c = &g->clues[slot_idx];
        // Badge colorido com o tag lógico da pista
        DrawRectangle(x+4, y+14, MeasureText(c->tag,12)+6, 16, (Color){c->color.r/4,c->color.g/4,c->color.b/4,200});
        DrawText(c->tag, x+7, y+15, 12, c->color);
        // Texto truncado da pista (máx 28 caracteres)
        char trunc[32];
        strncpy(trunc, c->text, 28); trunc[28]=0;
        if (strlen(c->text)>28) { trunc[25]='.'; trunc[26]='.'; trunc[27]='.'; trunc[28]=0; }
        DrawText(trunc, x+MeasureText(c->tag,12)+16, y+17, 10, COL_UI_DIM);
    } else {
        // Instrução piscante quando o slot está vazio
        float p = 0.5f + 0.5f*sinf(t*2.0f);
        DrawText("clique numa pista", x+8, y+16, 10, (Color){50,60,80,(unsigned char)(180*p)});
    }
}

// =============================================================
// DeductionDraw — renderiza a tela do Caderno de Deduções
// =============================================================
void DeductionDraw(GameState* g) {
    int W = GetScreenWidth();
    int H = GetScreenHeight();
    float t = g->menu_anim; // Timer global usado para animações

    // --- Fundo com efeito de scanlines (linhas horizontais alternadas) ---
    DrawRectangle(0, 0, W, H, (Color){4,6,14,255});
    for (int y = 0; y < H; y += 3)
        DrawRectangle(0, y, W, 1, (Color){0,0,0,40}); // Linha escura a cada 3px

    // --- Barra de título ---
    DrawRectangle(0, 0, W, 50, (Color){8,12,24,255});
    DrawRectangleLinesEx((Rectangle){0,0,(float)W,50}, 1, COL_UI_BORDER);
    DrawText("CADERNO DE DEDUÇÃO", 16, 14, 22, COL_UI_ACCENT);
    DrawText("TAB/ESC fechar  ENTER deduzir  C limpar  1-4 operador",
        W - 440, 17, 12, COL_UI_DIM);

    // Símbolos lógicos flutuantes como decoração da barra de título
    const char* syms[] = {"∧","∨","¬","→","↔","∴","⊥","∀"};
    for (int i = 0; i < 8; i++) {
        float fx = 50.0f + i*(W/8.0f) + sinf(t*0.5f+i)*20.0f;
        float fy = 55.0f + sinf(t*0.8f+i*0.7f)*8.0f;
        DrawText(syms[i], (int)fx, (int)fy, 16,
            (Color){60,80,120,(unsigned char)(80+40*sinf(t+i))});
    }

    // === PAINEL ESQUERDO: lista de pistas coletadas ===
    int lp_x = 10, lp_y = 80, lp_w = W/2 - 20, lp_h = H - 100;
    DrawRectangle(lp_x, lp_y, lp_w, lp_h, (Color){6,9,18,220});
    DrawRectangleLinesEx((Rectangle){(float)lp_x,(float)lp_y,(float)lp_w,(float)lp_h},
        1, COL_UI_BORDER);
    DrawText("PISTAS COLETADAS", lp_x+12, lp_y+10, 16, COL_UI_ACCENT);
    DrawRectangle(lp_x+10, lp_y+30, lp_w-20, 1, COL_UI_BORDER);

    // Contador "X/Y descobertas"
    int discovered_count = 0;
    for (int i = 0; i < g->clue_count; i++)
        if (g->clues[i].discovered) discovered_count++;
    char cnt[32];
    snprintf(cnt, 31, "%d/%d descobertas", discovered_count, g->clue_count);
    DrawText(cnt, lp_x+lp_w-MeasureText(cnt,13)-12, lp_y+12, 13, COL_UI_DIM);

    // Detecta se P e ¬P' estão simultaneamente descobertas (para highlight de contradição)
    int clue_P_idx   = -1;
    int clue_nP_idx  = -1;
    for (int i = 0; i < g->clue_count; i++) {
        if (strcmp(g->clues[i].tag, "P")   == 0 && g->clues[i].discovered) clue_P_idx  = i;
        if (strcmp(g->clues[i].tag, "nP'") == 0 && g->clues[i].discovered) clue_nP_idx = i;
    }
    bool show_contradiction = (clue_P_idx >= 0 && clue_nP_idx >= 0);

    // Lista de pistas com scroll
    int cy = lp_y + 40;
    int scroll_px = g->deduction_scroll * 60;
    for (int i = 0; i < g->clue_count; i++) {
        Clue* c = &g->clues[i];
        int iy = cy + i*60 - scroll_px; // Posição Y com scroll aplicado
        if (iy < lp_y+35 || iy > lp_y+lp_h-20) continue; // Fora da área visível: pula

        bool is_slot_a = (g->deduce_slot_a == i);
        bool is_slot_b = (g->deduce_slot_b == i);

        // Pista ainda não descoberta: exibe como "???"
        if (!c->discovered) {
            DrawRectangle(lp_x+8, iy, lp_w-16, 52, (Color){8,10,20,180});
            DrawRectangleLinesEx((Rectangle){(float)(lp_x+8),(float)iy,(float)(lp_w-16),52.0f},
                1, (Color){30,40,60,255});
            DrawText("???", lp_x+16, iy+8, 14, (Color){40,50,70,255});
            DrawText("Pista não descoberta", lp_x+16, iy+26, 12, (Color){40,50,70,255});
            continue;
        }

        // Cores da pista variam conforme seleção e contradição
        bool is_contradiction_member = show_contradiction &&
            (i == clue_P_idx || i == clue_nP_idx);
        Color border = c->color;
        Color bg     = (Color){10,14,28,200};

        if (is_slot_a || is_slot_b) {
            bg     = (Color){20,40,80,220}; // Azul quando selecionada
            border = COL_UI_ACCENT;
        }
        if (is_contradiction_member) {
            // Pisca em vermelho quando faz par de contradição com outra pista
            float blink = 0.6f + 0.4f*sinf(t*6.0f);
            border = (Color){(unsigned char)(200*blink),50,50,255};
            bg     = (Color){(unsigned char)(40*blink),10,10,200};
        }
        if (g->deduce_error_timer > 0 && (is_slot_a || is_slot_b)) {
            float eb = sinf(g->deduce_error_timer*20.0f);
            if (eb > 0) border = COL_UI_RED;
        }

        DrawRectangle(lp_x+8, iy, lp_w-16, 52, bg);
        DrawRectangleLinesEx((Rectangle){(float)(lp_x+8),(float)iy,(float)(lp_w-16),52.0f},
            (is_slot_a||is_slot_b||is_contradiction_member) ? 2 : 1, border);

        if (is_slot_a) DrawText("A", lp_x+lp_w-46, iy+6, 14, COL_UI_GOLD);  // Indicador slot A
        if (is_slot_b) DrawText("B", lp_x+lp_w-46, iy+6, 14, (Color){100,200,255,255}); // slot B

        // Símbolo ⊥ (falsum/contradição) entre P e ¬P' quando contradição detectada
        if (is_contradiction_member && show_contradiction && i == clue_nP_idx) {
            float bsz = 0.8f + 0.2f*sinf(t*6.0f);
            DrawText("⊥", lp_x + lp_w - 26, iy - 14, (int)(16*bsz), COL_UI_RED);
        }

        DrawLogicSymbol(c->tag, lp_x+14, iy+6, c->color); // Badge do tag lógico
        if (c->confirmed) DrawText("✓", lp_x+lp_w-30, iy+8, 16, COL_UI_GREEN); // Confirmada

        const char* ticons[] = {"[P]","[O]","[D]"}; // Tipo: Proposição, Observação, Dedução
        DrawText(ticons[c->type], lp_x+lp_w-62, iy+28, 11, COL_UI_DIM);

        // Texto da pista truncado a 56 caracteres
        char disp[60];
        strncpy(disp, c->text, 56); disp[56]=0;
        if ((int)strlen(c->text)>56) { disp[53]='.'; disp[54]='.'; disp[55]='.'; disp[56]=0; }
        DrawText(disp, lp_x+14, iy+28, 12, COL_UI_TEXT);

        // Fonte da pista (qual NPC a deu)
        if (c->npc_source >= 0 && c->npc_source < g->npc_count) {
            char src[64];
            snprintf(src, 63, "Fonte: %s", g->npcs[c->npc_source].name);
            DrawText(src, lp_x+14, iy+42, 10, c->color);
        }
    }

    // Banner de contradição piscante no topo do painel esquerdo
    if (show_contradiction) {
        int cx2 = lp_x + lp_w/2;
        float blink = 0.6f + 0.4f*sinf(t*4.0f);
        DrawRectangle(lp_x+8, lp_y+33, lp_w-16, 20,
            (Color){(unsigned char)(60*blink),10,10,200});
        DrawRectangleLinesEx((Rectangle){(float)(lp_x+8),(float)(lp_y+33),(float)(lp_w-16),20.0f},
            1, (Color){(unsigned char)(200*blink),50,50,255});
        int ctw = MeasureText("⊥ CONTRADIÇÃO DETECTADA: P ∧ ¬P'", 12);
        DrawText("⊥ CONTRADIÇÃO DETECTADA: P ∧ ¬P'",
            cx2 - ctw/2, lp_y+36, 12,
            (Color){255,(unsigned char)(100*blink),(unsigned char)(100*blink),255});
    }

    // Indicador de scroll lateral
    if (g->clue_count * 60 > lp_h - 50) {
        float pct = (g->clue_count > 5) ? (float)g->deduction_scroll/(g->clue_count-5.0f) : 0;
        int sh = (int)(lp_h * 0.3f);
        int sy = lp_y + (int)((lp_h - sh) * pct);
        DrawRectangle(lp_x+lp_w-6, sy, 4, sh, COL_UI_BORDER);
    }

    // === PAINEL DIREITO SUPERIOR: combinador de deduções ===
    int rp_x = W/2+10, rp_y = 80, rp_w = W/2-20;
    int combiner_h = 270;

    DrawRectangle(rp_x, rp_y, rp_w, combiner_h, (Color){6,9,18,220});
    DrawRectangleLinesEx((Rectangle){(float)rp_x,(float)rp_y,(float)rp_w,(float)combiner_h},
        1, COL_UI_BORDER);
    DrawText("CONSTRUIR DEDUÇÃO", rp_x+12, rp_y+10, 16, COL_UI_GOLD);
    DrawRectangle(rp_x+10, rp_y+30, rp_w-20, 1, COL_UI_BORDER);
    DrawText("Clique em 2 pistas + operador + DEDUZIR", rp_x+12, rp_y+35, 11, COL_UI_DIM);

    // Slots A e B lado a lado
    int slot_y = rp_y + 52;
    int half_w = (rp_w - 32) / 2;
    DrawDeduceSlot(g, g->deduce_slot_a, rp_x+12,        slot_y, half_w, "PISTA A", t);
    DrawDeduceSlot(g, g->deduce_slot_b, rp_x+16+half_w, slot_y, half_w, "PISTA B", t);

    // Operador selecionado centralizado entre os slots
    DrawTextCenteredX(g->font_main, OP_SYMBOLS[g->deduce_op],
        rp_x + rp_w/2, slot_y + 10, 18, 1, COL_UI_ACCENT);

    // Botões dos 4 operadores
    int btn_y = rp_y + 170;
    DrawText("OPERADOR:", rp_x+12, btn_y-14, 11, COL_UI_DIM);
    for (int op = 0; op < 4; op++) {
        int btn_x = rp_x + 12 + op * ((rp_w-24)/4);
        int btn_w = (rp_w-24)/4 - 4;
        bool sel = (g->deduce_op == op); // Este botão está selecionado?
        Color bc = sel ? COL_UI_GOLD : (Color){30,40,60,255};
        Color tc = sel ? COL_UI_GOLD : COL_UI_DIM;
        DrawRectangle(btn_x, btn_y, btn_w, 28, sel ? (Color){20,30,10,220} : (Color){8,12,20,200});
        DrawRectangleLinesEx((Rectangle){(float)btn_x,(float)btn_y,(float)btn_w,28.0f},
            sel ? 2 : 1, bc);
        int sw = MeasureText(OP_SYMBOLS[op], 16);
        DrawText(OP_SYMBOLS[op], btn_x + btn_w/2 - sw/2, btn_y+6, 16, tc);
    }

    // Botão DEDUZIR (fica verde quando há pistas válidas; vermelho se erro)
    int deduce_y = rp_y + 215;
    bool has_error = (g->deduce_error_timer > 0);
    float ep = has_error ? (0.5f + 0.5f*sinf(g->deduce_error_timer*20.0f)) : 1.0f;
    Color deduce_col    = has_error ? (Color){(unsigned char)(180*ep),40,40,255} : (Color){40,100,40,220};
    Color deduce_border = has_error ? COL_UI_RED : COL_UI_GREEN;
    bool can_deduce = (g->deduce_slot_a >= 0) && (g->deduce_op == 3 || g->deduce_slot_b >= 0);

    DrawRectangle(rp_x+12, deduce_y, rp_w-24, 32, can_deduce ? deduce_col : (Color){15,20,35,200});
    DrawRectangleLinesEx((Rectangle){(float)(rp_x+12),(float)deduce_y,(float)(rp_w-24),32.0f},
        2, can_deduce ? deduce_border : (Color){30,40,60,255});
    const char* deduce_txt = has_error ? "COMBINAÇÃO INVÁLIDA" : "DEDUZIR";
    int dtw = MeasureText(deduce_txt, 16);
    DrawText(deduce_txt, rp_x + rp_w/2 - dtw/2, deduce_y+8, 16,
        can_deduce ? WHITE : COL_UI_DIM);

    // === PAINEL DIREITO INFERIOR: deduções já confirmadas ===
    int dp_y = rp_y + combiner_h + 8;
    int dp_h = H - dp_y - 30;
    DrawRectangle(rp_x, dp_y, rp_w, dp_h, (Color){6,9,18,220});
    DrawRectangleLinesEx((Rectangle){(float)rp_x,(float)dp_y,(float)rp_w,(float)dp_h},
        1, COL_UI_BORDER);
    DrawText("DEDUÇÕES CONFIRMADAS", rp_x+12, dp_y+10, 15, COL_UI_GOLD);
    DrawRectangle(rp_x+10, dp_y+28, rp_w-20, 1, COL_UI_BORDER);

    int dy = dp_y + 36;
    for (int i = 0; i < g->deduction_count; i++) {
        Deduction* d = &g->deductions[i];
        if (dy > dp_y + dp_h - 8) break;

        // Dedução ainda não realizada: exibe placeholder cinza
        if (!d->unlocked) {
            DrawRectangle(rp_x+8, dy, rp_w-16, 32, (Color){8,10,18,160});
            DrawRectangleLinesEx((Rectangle){(float)(rp_x+8),(float)dy,(float)(rp_w-16),32.0f},
                1, (Color){20,30,50,255});
            DrawText("[ aguardando dedução manual ]", rp_x+16, dy+9, 12, (Color){35,45,65,255});
            dy += 38; continue;
        }

        // Cor da dedução: dourado padrão, verde para solução, vermelho para contradição
        Color dc = COL_UI_GOLD;
        if (strcmp(d->conclusion,"SOLVE")==0) dc = COL_UI_GREEN;
        if (strcmp(d->conclusion,"CONT") ==0) dc = COL_UI_RED;

        float pulse = 0.85f + 0.15f*sinf(t*2.0f+i); // Pulso suave
        DrawRectangle(rp_x+8, dy, rp_w-16, 44, (Color){12,18,30,220});
        DrawRectangleLinesEx((Rectangle){(float)(rp_x+8),(float)dy,(float)(rp_w-16),44.0f},
            1, (Color){(unsigned char)(dc.r*pulse),(unsigned char)(dc.g*pulse),(unsigned char)(dc.b*pulse),255});

        // Visualização da fórmula: PREMISSA1 [operador] PREMISSA2 ∴ CONCLUSÃO
        int fx = rp_x + 14;
        DrawDeductionNode(d->premise1, fx, dy+4, COL_UI_ACCENT, true);
        fx += MeasureText(d->premise1,13) + 16;
        DrawText(d->operator[0]!=0 ? d->operator : "∧", fx, dy+6, 13, COL_UI_DIM);
        fx += MeasureText(d->operator,13) + 8;
        if (d->premise2[0]) DrawDeductionNode(d->premise2, fx, dy+4, COL_UI_ACCENT, true);
        DrawText("∴", rp_x+14, dy+26, 13, dc); // ∴ = "portanto"
        DrawDeductionNode(d->conclusion, rp_x+28, dy+24, dc, false);

        dy += 50;
    }

    // Dica de controles na barra inferior
    DrawText("Clique nas pistas para selecionar  |  1-4 Operador  |  ENTER Deduzir  |  C Limpar",
        W/2 - MeasureText("Clique nas pistas para selecionar  |  1-4 Operador  |  ENTER Deduzir  |  C Limpar",11)/2,
        H - 18, 11, COL_UI_DIM);

    // MELHORIA 3: overlay do quiz de dedução
    DedQuizDraw(g);
}
