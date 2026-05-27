#include "game.h"

void DrawPixelBox(int x, int y, int w, int h, Color bg, Color border) {
    DrawRectangle(x, y, w, h, bg);
    DrawRectangleLinesEx((Rectangle){(float)x,(float)y,(float)w,(float)h}, 1, border);
}

void DrawWrappedText(Font font, const char* text, int x, int y, int max_w, int size, int spacing, Color col) {
    (void)font;
    char word[64];
    char line_buf[512];
    int  word_i = 0;
    int  cur_y  = y;
    int  i = 0;
    line_buf[0] = '\0';

    while (1) {
        char c = text[i++];
        if (c == ' ' || c == '\n' || c == '\0') {
            word[word_i] = '\0';
            word_i = 0;

            char test[512];
            if (strlen(line_buf) > 0)
                snprintf(test, sizeof(test), "%s %s", line_buf, word);
            else
                snprintf(test, sizeof(test), "%s", word);

            if (MeasureText(test, size) > max_w) {
                DrawText(line_buf, x, cur_y, size, col);
                cur_y += size + spacing + 2;
                snprintf(line_buf, sizeof(line_buf), "%s", word);
            } else {
                snprintf(line_buf, sizeof(line_buf), "%s", test);
            }

            if (c == '\n') {
                DrawText(line_buf, x, cur_y, size, col);
                cur_y += size + spacing + 2;
                line_buf[0] = '\0';
            }

            if (c == '\0') {
                DrawText(line_buf, x, cur_y, size, col);
                break;
            }
        } else {
            if (word_i < 63) word[word_i++] = c;
        }
    }
}

void DrawTextCentered(Font font, const char* text, int y, int size, int spacing, Color col) {
    (void)font; (void)spacing;
    int w = MeasureText(text, size);
    DrawText(text, GetScreenWidth()/2 - w/2, y, size, col);
}

void DrawTextCenteredX(Font font, const char* text, int cx, int y, int size, int spacing, Color col) {
    (void)font; (void)spacing;
    int w = MeasureText(text, size);
    DrawText(text, cx - w/2, y, size, col);
}

// =============================================
// HUD
// =============================================
void UIDrawHUD(GameState* g) {
    int W = GetScreenWidth();

    DrawRectangle(0, 0, W, 38, (Color){5,8,18,210});
    DrawRectangleLinesEx((Rectangle){0,0,(float)W,38}, 1, (Color){30,40,70,200});

    DrawText("Vila dos Paradoxos", 10, 10, 16, COL_UI_DIM);

    const char* time_labels[] = {"Amanhecer","Dia","Entardecer","Noite"};
    int tod = (int)(g->env.time_of_day / 6.0f) % 4;
    char time_str[48];
    bool kael_truth = (g->env.time_of_day >= 6.0f && g->env.time_of_day < 20.0f);
    snprintf(time_str, 47, "%02d:%02d  %s",
        (int)g->env.time_of_day,
        (int)((g->env.time_of_day - (int)g->env.time_of_day) * 60),
        time_labels[tod]);
    int tw = MeasureText(time_str, 15);
    Color time_col = kael_truth ? COL_UI_TEXT : (Color){160,120,220,255};
    DrawText(time_str, W/2 - tw/2, 11, 15, time_col);

    DrawText("Código do Farol:", W - 230, 10, 13, COL_UI_DIM);
    for (int i = 0; i < 3; i++) {
        Color fc = i < g->player.lighthouse_fragments ? COL_LIGHTHOUSE : (Color){40,40,55,255};
        DrawText(i < g->player.lighthouse_fragments ? "★" : "☆",
            W - 90 + i * 22, 8, 20, fc);
    }

    int bh = 36;
    int by = GetScreenHeight() - bh;
    DrawRectangle(0, by, W, bh, (Color){5,8,18,210});
    DrawRectangleLinesEx((Rectangle){0,(float)by,(float)W,(float)bh}, 1, (Color){30,40,70,200});

    DrawText("[E] Interagir", 10, by+10, 13, COL_UI_DIM);
    DrawText("[TAB] Caderno", 120, by+10, 13, COL_UI_DIM);
    DrawText("[WASD/↑↓←→] Mover", 240, by+10, 13, COL_UI_DIM);
    DrawText("[ESC] Pausa", 430, by+10, 13, COL_UI_DIM);

    if (g->mystery_solved) {
        float ms_p = 0.7f + 0.3f * sinf(g->menu_anim * 2.0f);
        DrawText("✓ MISTÉRIO SOLUCIONADO",
            W - 220, by+10, 13,
            (Color){(unsigned char)(80*ms_p),(unsigned char)(180*ms_p),(unsigned char)(80*ms_p),255});
    } else {
        int found = 0;
        for (int i = 0; i < g->clue_count; i++) {
            if (g->clues[i].discovered) found++;
        }
        char clue_str[32];
        snprintf(clue_str, 31, "Pistas: %d/%d", found, g->clue_count);
        DrawText(clue_str, W - 120, by+10, 13, COL_UI_DIM);
    }

    NPC* nearby = GetNearbyNPC(g);
    if (nearby && g->current_screen == SCREEN_GAME) {
        int hw = 260;
        int hx = W/2 - hw/2;
        int hy = GetScreenHeight() - bh - 32;
        DrawRectangle(hx, hy, hw, 28, (Color){8,12,22,220});
        DrawRectangleLinesEx((Rectangle){(float)hx,(float)hy,(float)hw,28.0f},
            1, nearby->color);
        char hint[96];
        snprintf(hint, sizeof(hint)-1, "[E] Falar com %s", nearby->name);
        int htw = MeasureText(hint, 14);
        DrawText(hint, hx + hw/2 - htw/2, hy+7, 14, nearby->color);
    }

    if (g->player.in_deduction_mode) {
        float p = 0.5f + 0.5f * sinf(g->menu_anim * 4.0f);
        DrawRectangleLinesEx((Rectangle){0,0,(float)W,(float)GetScreenHeight()},
            3, (Color){80,120,200,(unsigned char)(100*p)});
        DrawText("MODO DEDUÇÃO", W-140, 42, 14, (Color){100,150,255,(unsigned char)(200*p)});
    }
}

void ShowNotification(GameState* g, const char* title, const char* body, Color col) {
    g->notif.active = true;
    strncpy(g->notif.title, title, 63);
    strncpy(g->notif.body,  body,  511);
    g->notif.timer = 4.5f;
    g->notif.color = col;
}

void UIUpdateNotification(GameState* g) {
    if (!g->notif.active) return;
    g->notif.timer -= g->delta;
    if (g->notif.timer <= 0) {
        g->notif.active = false;
        g->notif.timer  = 0;
    }
}

void UIDrawNotification(GameState* g) {
    if (!g->notif.active) return;

    float t = g->notif.timer;
    float alpha = 1.0f;
    if (t < 0.5f) alpha = t / 0.5f;
    if (t > 4.0f) alpha = (4.5f - t) / 0.5f;
    if (alpha > 1.0f) alpha = 1.0f;

    int nw = 420, nh = 80;
    int nx = GetScreenWidth() - nw - 16;
    int ny = 50;

    float slide = (1.0f - alpha) * (nw + 20);
    nx += (int)slide;

    Color bg = (Color){8,12,22,220};
    Color border = g->notif.color;
    border.a = (unsigned char)(alpha * 255);

    DrawRectangle(nx, ny, nw, nh, bg);
    DrawRectangleLinesEx((Rectangle){(float)nx,(float)ny,(float)nw,(float)nh}, 2, border);

    DrawRectangle(nx, ny, 4, nh, border);

    DrawRectangle(nx+8, ny+8, 44, 44, (Color){border.r/3,border.g/3,border.b/3,200});
    DrawText("!", nx+20, ny+12, 26, border);

    DrawText(g->notif.title, nx+60, ny+10, 16,
        (Color){border.r,border.g,border.b,(unsigned char)(alpha*255)});

    char body_short[80];
    strncpy(body_short, g->notif.body, 79);
    body_short[79] = '\0';
    if ((int)strlen(g->notif.body) > 60) {
        body_short[57] = '.';
        body_short[58] = '.';
        body_short[59] = '.';
        body_short[60] = '\0';
    }
    DrawText(body_short, nx+60, ny+34, 12,
        (Color){180,190,210,(unsigned char)(alpha*200)});

    float bar_pct = g->notif.timer / 4.5f;
    DrawRectangle(nx+4, ny+nh-4, (int)((nw-8)*bar_pct), 3, border);
}
