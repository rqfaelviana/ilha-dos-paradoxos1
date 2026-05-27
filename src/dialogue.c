#include "game.h"

// =============================================
// DIALOGUE DATABASE - O MISTÉRIO DA ILHA
// =============================================
// O mistério: Martim (morador) desapareceu.
//   - O pescador diz a verdade (Veraz)
//   - A sacerdotisa mente (Falaz) 
//   - O ferreiro alterna (Paradoxal)
//
// Solução: A sacerdotisa está mentindo sobre o alibi do ferreiro.
//          O ferreiro estava nos ruins quando Martim desapareceu.
//          O pescador confirma que viu a sacerdotisa no porto (não no templo).
// =============================================

void DialogueInit(GameState* g) {
    g->dialogue_db_count = 0;
    int i = 0;

    // ============================================================
    // PESCADOR (Veraz) - diálogos 0..3
    // ============================================================
    // Linha 0
    strcpy(g->dialogue_db[i].speaker, "Velho Pescador");
    strcpy(g->dialogue_db[i].text,
        "Ah... outro náufrago. A ilha sempre traz mais do que leva. "
        "Chamo-me Aldric. Pesco nestas águas há quarenta anos.");
    g->dialogue_db[i].next_index    = 1;
    g->dialogue_db[i].triggers_clue = false;
    i++;

    // Linha 1
    strcpy(g->dialogue_db[i].speaker, "Velho Pescador");
    strcpy(g->dialogue_db[i].text,
        "Martim desapareceu há três noites. Era um bom rapaz. "
        "Vi a Sacerdotisa Mira no porto nessa noite — não no templo como ela diz.");
    g->dialogue_db[i].next_index    = 2;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 0; // "P: A sacerdotisa estava no porto"
    i++;

    // Linha 2
    strcpy(g->dialogue_db[i].speaker, "Velho Pescador");
    strcpy(g->dialogue_db[i].text,
        "Se a sacerdotisa mentiu sobre onde estava... então o alibi que ela deu "
        "para o Ferreiro Kael também pode ser mentira. Pense nisso.");
    g->dialogue_db[i].next_index    = 3;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 1; // "P -> ¬Q"
    i++;

    // Linha 3
    strcpy(g->dialogue_db[i].speaker, "Velho Pescador");
    strcpy(g->dialogue_db[i].text,
        "Há símbolos nas ruínas antigas que falam sobre o Código do Farol. "
        "Dizem que apenas quem enxerga a lógica oculta pode ativá-lo. Tome cuidado.");
    g->dialogue_db[i].next_index    = -1; // end
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 2; // fragmento farol
    i++;

    // ============================================================
    // SACERDOTISA (Falaz) - diálogos 4..7
    // ============================================================
    // Linha 4
    strcpy(g->dialogue_db[i].speaker, "Sacerdotisa Mira");
    strcpy(g->dialogue_db[i].text,
        "Bem-vindo, náufrago. Esta ilha tem seus segredos... "
        "mas eu não tenho nada a esconder. Sirvo o farol há décadas.");
    g->dialogue_db[i].next_index    = 5;
    g->dialogue_db[i].triggers_clue = false;
    i++;

    // Linha 5 - MENTIRA: ela diz que estava no templo (mas estava no porto)
    strcpy(g->dialogue_db[i].speaker, "Sacerdotisa Mira");
    strcpy(g->dialogue_db[i].text,
        "Na noite do desaparecimento de Martim? Estava no templo, "
        "em oração. O Ferreiro Kael pode confirmar — ele me viu lá.");
    g->dialogue_db[i].next_index    = 6;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 3; // "¬P (mentira): sacerdotisa diz estar no templo"
    i++;

    // Linha 6 - MENTIRA: diz que o ferreiro é veraz
    strcpy(g->dialogue_db[i].speaker, "Sacerdotisa Mira");
    strcpy(g->dialogue_db[i].text,
        "Kael é um homem honesto. Confie nele. "
        "O pescador é quem mente — é velho e sua memória falha.");
    g->dialogue_db[i].next_index    = 7;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 4; // suspeita sobre pescador
    i++;

    // Linha 7
    strcpy(g->dialogue_db[i].speaker, "Sacerdotisa Mira");
    strcpy(g->dialogue_db[i].text,
        "O Código do Farol? Não existe tal coisa. "
        "São apenas lendas para assustar crianças e náufragos incautos como você.");
    g->dialogue_db[i].next_index    = -1;
    g->dialogue_db[i].triggers_clue = false;
    i++;

    // ============================================================
    // FERREIRO PARADOXAL - diálogos 8..12
    // ============================================================
    // Linha 8 (VERDADE - primeiro contato)
    strcpy(g->dialogue_db[i].speaker, "Ferreiro Kael");
    strcpy(g->dialogue_db[i].text,
        "Hmm. Você veio com perguntas. Eu sempre alterno entre o que é "
        "e o que parece ser. Esta fala — é verdade.");
    g->dialogue_db[i].next_index    = 9;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 5; // ferreiro é paradoxal
    i++;

    // Linha 9 (MENTIRA - segunda fala do ferreiro)
    strcpy(g->dialogue_db[i].speaker, "Ferreiro Kael");
    strcpy(g->dialogue_db[i].text,
        "Na noite que Martim sumiu, eu estava aqui na forja. "
        "A sacerdotisa pode confirmar — ela passou por aqui.");
    g->dialogue_db[i].next_index    = 10;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 6; // alibi do ferreiro (MENTIRA - ele estava nas ruínas)
    i++;

    // Linha 10 (VERDADE - terceira fala)
    strcpy(g->dialogue_db[i].speaker, "Ferreiro Kael");
    strcpy(g->dialogue_db[i].text,
        "...Escuta. Minha fala anterior foi... imprecisa. "
        "Estive nas ruínas naquela noite. Vi algo que não deveria ter visto.");
    g->dialogue_db[i].next_index    = 11;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 7; // Q: ferreiro estava nas ruínas (VERDADE)
    i++;

    // Linha 11 (MENTIRA - quarta fala)
    strcpy(g->dialogue_db[i].speaker, "Ferreiro Kael");
    strcpy(g->dialogue_db[i].text,
        "Martim? Não o vi. Nunca fomos próximos. "
        "Esta conversa não lhe será útil.");
    g->dialogue_db[i].next_index    = 12;
    g->dialogue_db[i].triggers_clue = false;
    i++;

    // Linha 12 (VERDADE - quinta fala)
    strcpy(g->dialogue_db[i].speaker, "Ferreiro Kael");
    strcpy(g->dialogue_db[i].text,
        "...Vi Martim nas ruínas. Ele encontrou algo no chão — "
        "um símbolo. Depois... a névoa o engoliu. Isso é tudo que sei.");
    g->dialogue_db[i].next_index    = -1;
    g->dialogue_db[i].triggers_clue = true;
    g->dialogue_db[i].clue_id       = 8; // Martim estava nas ruínas
    i++;

    g->dialogue_db_count = i;

    // ============================================================
    // INTRO DIALOGUE (uses separate index range, 50+)
    // ============================================================
    // Linha 50 - narração de intro
    strcpy(g->dialogue_db[50].speaker, "...");
    strcpy(g->dialogue_db[50].text,
        "Você acorda com o gosto de sal na boca. "
        "A praia sob seus pés é fria e úmida.");
    g->dialogue_db[50].next_index    = 51;
    g->dialogue_db[50].triggers_clue = false;

    strcpy(g->dialogue_db[51].speaker, "...");
    strcpy(g->dialogue_db[51].text,
        "Ao longe, um farol pisca através da névoa. "
        "Você não sabe como chegou aqui. Mas sabe que precisa sair.");
    g->dialogue_db[51].next_index    = 52;
    g->dialogue_db[51].triggers_clue = false;

    strcpy(g->dialogue_db[52].speaker, "Você");
    strcpy(g->dialogue_db[52].text,
        "Esta ilha... parece diferente. "
        "As pessoas aqui falam de um jeito estranho. Como se cada palavra escondesse outra.");
    g->dialogue_db[52].next_index    = 53;
    g->dialogue_db[52].triggers_clue = false;

    strcpy(g->dialogue_db[53].speaker, "...");
    strcpy(g->dialogue_db[53].text,
        "Para ativar o farol e chamar um navio, você precisa descobrir "
        "o Código do Farol — fragmentos de verdade espalhados pela ilha.");
    g->dialogue_db[53].next_index    = 54;
    g->dialogue_db[53].triggers_clue = false;

    strcpy(g->dialogue_db[54].speaker, "...");
    strcpy(g->dialogue_db[54].text,
        "Mas cuidado: nem todos dizem a verdade. "
        "Alguns mentem sempre. Outros alternam. Só a lógica pode guiá-lo.");
    g->dialogue_db[54].next_index    = -2; // -2 = go to game
    g->dialogue_db[54].triggers_clue = false;

    // ============================================================
    // CLUES DATABASE
    // ============================================================
    g->clue_count = 0;

    // Clue 0: P - sacerdotisa no porto
    AddClue(g,
        "O Pescador viu a Sacerdotisa no porto na noite do desaparecimento",
        "P", CLUE_OBSERVATION, 0, COL_VERAZ);
    g->clues[0].discovered = false;

    // Clue 1: P → ¬Q (se sacerdotisa mentiu sobre porto, alibi do ferreiro cai)
    AddClue(g,
        "Se P é verdade, então o alibi que a Sacerdotisa deu ao Ferreiro é falso",
        "P→¬Q", CLUE_DEDUCTION, -1, COL_UI_GOLD);
    g->clues[1].discovered = false;

    // Clue 2: Fragmento do farol
    AddClue(g,
        "[FRAGMENTO 1/3] Nas ruínas há um Código antigo ligado ao Farol",
        "F1", CLUE_PROPOSITION, -1, COL_LIGHTHOUSE);
    g->clues[2].discovered = false;

    // Clue 3: ¬P' (sacerdotisa disse estar no templo - mas é mentira)
    AddClue(g,
        "Sacerdotisa afirma: 'Estava no templo'. Contradiz observação do Pescador",
        "¬P'", CLUE_PROPOSITION, 1, COL_FALAZ);
    g->clues[3].discovered = false;

    // Clue 4: Sacerdotisa acusa pescador
    AddClue(g,
        "Sacerdotisa tenta desviar suspeita para o Pescador",
        "D", CLUE_OBSERVATION, 1, COL_UI_DIM);
    g->clues[4].discovered = false;

    // Clue 5: Ferreiro é paradoxal
    AddClue(g,
        "O Ferreiro admite alternar entre verdade e mentira",
        "K_PAR", CLUE_OBSERVATION, 2, COL_PARADOXAL);
    g->clues[5].discovered = false;

    // Clue 6: Alibi falso do ferreiro (ele disse que estava na forja)
    AddClue(g,
        "Ferreiro (fala ímpar/MENTIRA): 'Estava na forja na noite do crime'",
        "¬Q", CLUE_PROPOSITION, 2, COL_PARADOXAL);
    g->clues[6].discovered = false;

    // Clue 7: Q - ferreiro estava nas ruínas (verdade)
    AddClue(g,
        "Ferreiro (fala par/VERDADE): esteve nas Ruínas naquela noite",
        "Q", CLUE_OBSERVATION, 2, COL_PARADOXAL);
    g->clues[7].discovered = false;
    g->clues[7].confirmed = false;

    // Clue 8: Martim nas ruínas
    AddClue(g,
        "Ferreiro viu Martim encontrar um símbolo nas Ruínas antes de desaparecer",
        "R", CLUE_OBSERVATION, 2, COL_UI_ACCENT);
    g->clues[8].discovered = false;

    // Clue extra: Fragmento 2 (encontrado nas ruínas)
    AddClue(g,
        "[FRAGMENTO 2/3] O símbolo nas Ruínas é parte do Código do Farol",
        "F2", CLUE_PROPOSITION, -1, COL_LIGHTHOUSE);
    g->clues[9].discovered = false;

    // Clue extra: Fragmento 3 (dedução final)
    AddClue(g,
        "[FRAGMENTO 3/3] Com P, Q e R confirmados: o Código do Farol está completo",
        "F3", CLUE_PROPOSITION, -1, COL_LIGHTHOUSE);
    g->clues[10].discovered = false;

    // MELHORIA 4: Pistas de lore desbloqueadas pelas novas portas logicas
    // Clue 11: lore do Templo (porta nP_OR_R)
    AddClue(g,
        "[LORE] Anotacoes de Martim no Templo: 'O simbolo pulsa mais forte a cada noite...'",
        "LORE_T", CLUE_OBSERVATION, -1, (Color){120,100,180,255});
    g->clues[11].discovered = false;

    // Clue 12: lore da Forja (porta Q_IMPL_R)
    AddClue(g,
        "[LORE] Diario do Ferreiro: 'Vi Martim copiar o simbolo em papel. Devia ter o avisado.'",
        "LORE_F", CLUE_OBSERVATION, 2, COL_PARADOXAL);
    g->clues[12].discovered = false;

    g->clue_count = 13;

    // DEDUCTIONS
    g->deduction_count = 0;
    AddDeduction(g, "P", "&&", "¬P'",
        "CONT",
        "P (pescador viu sacerdotisa no porto) E ¬P' (sacerdotisa diz estar no templo)\n→ CONTRADIÇÃO: Sacerdotisa mente sobre seu alibi");

    AddDeduction(g, "P", "->", "¬Q_alibi",
        "¬Q_alibi",
        "Se P (sacerdotisa no porto), então o alibi que ela deu ao ferreiro é inválido\n→ Ferreiro pode ter estado nas ruínas");

    AddDeduction(g, "K_PAR", "&&", "Q",
        "Q_conf",
        "Ferreiro é Paradoxal: fala 1=verdade, fala 2=mentira, fala 3=VERDADE\n→ Q confirmado: Ferreiro estava nas Ruínas");

    AddDeduction(g, "Q", "&&", "R",
        "SOLVE",
        "Q (Ferreiro nas ruínas) E R (Martim visto nas ruínas)\n→ CONCLUSÃO: Martim desapareceu nas Ruínas Antigas\n∴ O Código do Farol está nas ruínas");
}

// =============================================================
// AddClue — registra uma nova pista no banco de dados do jogo
// =============================================================
// Cada pista tem um texto descritivo, um tag lógico (ex: "P", "¬P'"),
// um tipo, a fonte (qual NPC deu) e uma cor de identificação.
void AddClue(GameState* g, const char* text, const char* tag, ClueType type, int npc, Color col) {
    if (g->clue_count >= MAX_CLUES) return; // Evita estouro do array
    Clue* c = &g->clues[g->clue_count++];   // Aponta para o próximo slot e já incrementa o contador
    strncpy(c->text, text, MAX_TEXT_LEN-1); // Copia o texto descritivo
    strncpy(c->tag,  tag,  15);             // Copia o tag lógico (ex: "P", "Q", "¬P'")
    c->type       = type;                   // Tipo: CLUE_PROPOSITION, OBSERVATION ou DEDUCTION
    c->npc_source = npc;                    // Índice do NPC fonte (-1 = ambiente/dedução)
    c->color      = col;                    // Cor visual da pista no caderno
    c->discovered = false;                  // Começa oculta; só aparece quando o jogador a encontra
    c->confirmed  = false;                  // Será marcada como confirmada após dedução
    c->negated    = false;
}

// =============================================================
// AddDeduction — registra uma dedução lógica possível no jogo
// =============================================================
// Uma dedução tem duas premissas, um operador e uma conclusão.
// Começa trancada (unlocked=false) e é desbloqueada pelo jogador
// ao combinar as pistas corretas no Caderno de Deduções.
void AddDeduction(GameState* g, const char* p1, const char* op, const char* p2,
                  const char* conc, const char* text) {
    if (g->deduction_count >= MAX_DEDUCTIONS) return;
    Deduction* d = &g->deductions[g->deduction_count++];
    strncpy(d->premise1,   p1,   15);  // Primeira premissa (ex: "P")
    strncpy(d->operator,   op,   7);   // Operador lógico (ex: "&&" = ∧, "->" = →)
    strncpy(d->premise2,   p2,   15);  // Segunda premissa (ex: "¬P'")
    strncpy(d->conclusion, conc, 15);  // Conclusão (ex: "CONT" = contradição)
    strncpy(d->text,       text, MAX_TEXT_LEN-1); // Explicação em linguagem natural
    d->unlocked = false; // Trancada até o jogador realizar a dedução manualmente
}


// =============================================
// CLUE TRIGGER
// =============================================

// =============================================================
// TriggerClue — revela uma pista ao jogador durante um diálogo
// =============================================================
// Chamada automaticamente quando uma linha de diálogo que tem
// triggers_clue=true termina de ser exibida.
static void TriggerClue(GameState* g, int clue_id) {
    if (clue_id < 0 || clue_id >= g->clue_count) return; // ID inválido
    if (g->clues[clue_id].discovered) return;             // Pista já foi descoberta antes

    // MELHORIA 2: pistas com quiz — não revela diretamente
    if (NpcQuizNeeded(clue_id)) {
        NpcQuizSetup(g, clue_id);
        if (g->active_npc >= 0) g->npcs[g->active_npc].talked_to = true;
        return;
    }

    g->clues[clue_id].discovered = true; // Marca como descoberta

    // Algumas pistas são auto-confirmadas pela fonte do NPC:
    if (clue_id == 0) { // P: vem do Pescador (Veraz), que nunca mente → já confirmada
        g->clues[clue_id].confirmed = true;
    }
    if (clue_id == 7) { // Q: vem do Ferreiro em sua fala de verdade → já confirmada
        g->clues[clue_id].confirmed = true;
    }

    // Exibe notificação no canto da tela informando a nova pista
    char notif_body[256];
    snprintf(notif_body, 255, "[%s] %s", g->clues[clue_id].tag, g->clues[clue_id].text);
    ShowNotification(g, "PISTA DESCOBERTA", notif_body, g->clues[clue_id].color);

    // Fragmento 1 do Farol: descoberto na conversa com o Pescador
    if (clue_id == 2) {
        g->player.lighthouse_fragments++;
    }
}

// =============================================
// DIALOGUE CONTROL
// =============================================

// =============================================================
// StartDialogue — inicia a conversa com um NPC
// =============================================================
void StartDialogue(GameState* g, int npc_index) {
    if (npc_index < 0 || npc_index >= g->npc_count) return;
    NPC* npc = &g->npcs[npc_index];

    g->active_npc      = npc_index;           // Marca qual NPC está ativo
    g->dialogue_line   = npc->dialogue_start; // Começa na primeira linha do NPC
    g->dialogue_typing = true;                // Ativa o efeito typewriter
    g->type_timer      = 0;
    g->type_char       = 0;
    g->typed_text[0]   = '\0';
    g->prev_screen     = SCREEN_DIALOGUE;
    g->current_screen  = SCREEN_DIALOGUE;     // Muda para a tela de diálogo

    // Incrementa o contador de falas (usado no fallback do Paradoxal)
    npc->paradox_counter++;
    if (npc->type == NPC_PARADOXAL) {
        // O estado do Ferreiro (verdade/mentira) já é atualizado pelo ciclo dia/noite
        // em NPCsUpdate(). Aqui apenas sincroniza para a fala atual.
        npc->paradox_truth_now = NPCSaysTruth(npc, g);
    }
}

// =============================================================
// EndDialogue — encerra a conversa e volta ao jogo
// =============================================================
void EndDialogue(GameState* g) {
    g->active_npc      = -1;           // Sem NPC ativo
    g->dialogue_line   = -1;
    g->dialogue_typing = false;
    g->current_screen  = SCREEN_GAME;  // Volta para a exploração do mapa
    if (g->npcs[0].talked_to || g->npcs[1].talked_to || g->npcs[2].talked_to) {
        CheckPuzzleSolution(g); // Verifica se o puzzle avançou após a conversa
    }
}

// =============================================================
// DialogueUpdate — processa input durante a tela de diálogo
// =============================================================
void DialogueUpdate(GameState* g) {
    // MELHORIA 2: se quiz NPC ativo, só processa o quiz
    if (g->npc_quiz_active) {
        NpcQuizUpdate(g);
        return;
    }

    // A engine de diálogo é compartilhada com a intro (SCREEN_INTRO)
    bool is_intro = (g->current_screen == SCREEN_INTRO);
    if (!is_intro && g->current_screen != SCREEN_DIALOGUE) return;
    if (g->dialogue_line < 0) {
        if (is_intro) { g->current_screen = SCREEN_GAME; return; }
        EndDialogue(g);
        return;
    }

    DialogueLine* line = &g->dialogue_db[g->dialogue_line];
    const char* full_text = line->text;
    int full_len = (int)strlen(full_text);

    // --- Efeito typewriter: revela o texto caractere a caractere ---
    if (g->dialogue_typing) {
        g->type_timer += g->delta;
        float speed = 0.025f; // Segundos por caractere (0.025s = ~40 chars/segundo)
        while (g->type_timer >= speed && g->type_char < full_len) {
            g->type_timer -= speed;
            g->typed_text[g->type_char] = full_text[g->type_char]; // Revela próximo char
            g->type_char++;
            g->typed_text[g->type_char] = '\0'; // Mantém a string terminada
        }
        if (g->type_char >= full_len) {
            g->dialogue_typing = false; // Texto completo: para o typewriter
            if (line->triggers_clue) { // Se essa linha dá uma pista, revela ela
                TriggerClue(g, line->clue_id);
                if (g->active_npc >= 0) g->npcs[g->active_npc].talked_to = true;
            }
        }
        // ESPAÇO/ENTER/CLIQUE: pula o typewriter e exibe o texto completo imediatamente
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            memcpy(g->typed_text, full_text, full_len+1);
            g->type_char = full_len;
            g->dialogue_typing = false;
            if (line->triggers_clue) {
                TriggerClue(g, line->clue_id);
                if (g->active_npc >= 0) g->npcs[g->active_npc].talked_to = true;
            }
        }
    } else {
        // --- Texto completo exibido: aguarda o jogador avançar ---
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int next = line->next_index; // Próxima linha (-1 = fim do diálogo)
            if (next == -1) {
                if (is_intro) {
                    // Intro terminada: fade para o jogo
                    g->prev_screen   = SCREEN_GAME;
                    g->fading_out    = true;
                    g->dialogue_line = -1;
                } else {
                    EndDialogue(g); // Fim do diálogo normal
                }
            } else if (next == -2) {
                // Código legado: mesmo efeito que -1 para a intro
                g->prev_screen   = SCREEN_GAME;
                g->fading_out    = true;
                g->dialogue_line = -1;
            } else {
                // Avança para a próxima linha e reinicia o typewriter
                g->dialogue_line   = next;
                g->dialogue_typing = true;
                g->type_timer      = 0;
                g->type_char       = 0;
                g->typed_text[0]   = 0;
            }
        }
        // ESC: encerra o diálogo prematuramente (não funciona na intro)
        if (!is_intro && IsKeyPressed(KEY_ESCAPE)) {
            EndDialogue(g);
        }
    }
}

// =============================================================
// DialogueDraw — renderiza a caixa de diálogo na tela
// =============================================================
// Desenha o retângulo de fundo, o retrato do NPC, o nome,
// o badge de tipo (Veraz/Falaz/Paradoxal) e o texto com typewriter.
void DialogueDraw(GameState* g) {
    if (g->dialogue_line < 0) return; // Sem linha ativa: não desenha nada
    DialogueLine* line = &g->dialogue_db[g->dialogue_line];

    int W = GetScreenWidth();
    int H = GetScreenHeight();

    // --- Caixa de diálogo fixada na parte inferior da tela ---
    int box_h = 180;
    int box_y = H - box_h - 10; // 10px acima da borda inferior
    int box_x = 20;
    int box_w = W - 40;

    // Fundo semi-transparente escuro
    DrawRectangle(box_x, box_y, box_w, box_h, (Color){5,8,18,230});
    DrawRectangleLinesEx((Rectangle){(float)box_x,(float)box_y,(float)box_w,(float)box_h},
        2, COL_UI_BORDER);

    // --- Retrato do NPC (quadrado à esquerda) ---
    int port_size = 60; // Tamanho do retrato em pixels
    Color speaker_col = COL_UI_TEXT;
    if (g->active_npc >= 0) {
        speaker_col = g->npcs[g->active_npc].color; // Cor do NPC ativo (verde/vermelho/roxo)
    } else if (g->active_npc == -2) {
        speaker_col = (Color){150,130,200,255}; // Narrador da intro: lilás
    }

    // Fundo e borda do retrato
    DrawRectangle(box_x + 8, box_y + 8, port_size, port_size, (Color){10,15,30,255});
    DrawRectangleLinesEx((Rectangle){(float)(box_x+8),(float)(box_y+8),(float)port_size,(float)port_size},
        1, speaker_col);

    // Inicial do nome do falante centralizada no retrato
    char init[2] = {line->speaker[0], '\0'};
    DrawText(init, box_x + 8 + port_size/2 - 8, box_y + 8 + port_size/2 - 14, 28, speaker_col);

    // Nome do falante acima do texto
    DrawText(line->speaker, box_x + port_size + 20, box_y + 12, 18, speaker_col);

    // --- Badge de tipo do NPC (revelado progressivamente) ---
    // O tipo [VERAZ], [FALAZ] ou [PARADOXAL] só aparece quando
    // o jogador descobriu a pista que revela esse NPC.
    if (g->active_npc >= 0) {
        NPC* npc = &g->npcs[g->active_npc];
        if (npc->type_revealed) {
            // Tipo revelado: exibe com pulsação colorida
            const char* type_label = "";
            Color type_col = WHITE;
            switch (npc->type) {
                case NPC_VERAZ:     type_label = "[VERAZ]";     type_col = COL_VERAZ;     break;
                case NPC_FALAZ:     type_label = "[FALAZ]";     type_col = COL_FALAZ;     break;
                case NPC_PARADOXAL: type_label = "[PARADOXAL]"; type_col = COL_PARADOXAL; break;
            }
            float pulse = 0.8f + 0.2f * sinf(g->menu_anim * 4.0f); // Brilho oscilante
            Color reveal_col = type_col;
            reveal_col.a = (unsigned char)(255 * pulse);
            DrawText(type_label,
                box_x + port_size + 20 + MeasureText(npc->name, 18) + 12,
                box_y + 14, 14, reveal_col);
        } else {
            // Tipo ainda desconhecido: exibe "???" cinza
            DrawText("[???]",
                box_x + port_size + 20 + MeasureText(g->npcs[g->active_npc].name, 18) + 12,
                box_y + 14, 14, (Color){60,70,90,180});
        }
    }

    // Linha separadora entre nome e texto
    DrawRectangle(box_x + port_size + 18, box_y + 36, box_w - port_size - 30, 1, COL_UI_BORDER);

    // Texto do diálogo com quebra automática de linha (word wrap)
    // g->typed_text contém apenas os caracteres revelados até agora pelo typewriter
    DrawWrappedText(g->font_main, g->typed_text,
        box_x + port_size + 20, box_y + 44,
        box_w - port_size - 40, 16, 1, COL_UI_TEXT);

    // --- Prompt piscante "ESPAÇO para continuar" quando o texto está completo ---
    if (!g->dialogue_typing) {
        float pulse = 0.5f + 0.5f * sinf(g->menu_anim * 4.0f); // Pisca suavemente
        int pw = MeasureText("ESPAÇO para continuar", 13);
        DrawText("ESPAÇO para continuar",
            box_x + box_w - pw - 12, box_y + box_h - 22,
            13, (Color){120,150,200,(unsigned char)(200*pulse)});
        DrawText("▼", box_x + box_w - 18, box_y + box_h - 22,
            13, (Color){100,140,200,(unsigned char)(200*pulse)});
    }

    // --- Modo intro: sobrepõe uma caixa centralizada na tela ---
    // Na intro, o narrador fala antes do jogo começar; o layout é diferente.
    if (g->active_npc == -2) {
        int center_x = W/2;
        int center_y = H/2 - 80;

        DrawRectangle(0, 0, W, H, (Color){0,0,0,180}); // Escurece o fundo

        // Caixa centralizada para o texto da intro
        int qw = 600, qh = 200;
        int qx = center_x - qw/2;
        int qy = center_y - qh/2;

        DrawRectangle(qx, qy, qw, qh, (Color){8,10,20,240});
        DrawRectangleLinesEx((Rectangle){(float)qx,(float)qy,(float)qw,(float)qh},
            1, COL_UI_BORDER);

        DrawText(line->speaker, qx + 16, qy + 12, 16, (Color){150,130,200,255});
        DrawRectangle(qx+12, qy+32, qw-24, 1, COL_UI_BORDER);

        // Texto da narração centralizado
        DrawWrappedText(g->font_main, g->typed_text,
            qx+20, qy+42, qw-40, 17, 1, (Color){210,215,230,255});

        if (!g->dialogue_typing) {
            float p = 0.5f + 0.5f * sinf(g->menu_anim * 3.0f);
            DrawTextCentered(g->font_main, "[ ESPAÇO ]",
                qy + qh + 20, 14, 1, (Color){100,130,180,(unsigned char)(200*p)});
        }
    }

    // MELHORIA 2: overlay do quiz do NPC
    NpcQuizDraw(g);
}
