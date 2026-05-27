#include "game.h"

static float star_x[80], star_y[80], star_br[80];
static float wave_timer = 0;
static bool  stars_init = false;

static void InitMenuStars() {
    if (stars_init) return;
    stars_init = true;
    for (int i = 0; i < 80; i++) {
        star_x[i] = (float)GetRandomValue(0, SCREEN_WIDTH);
        star_y[i] = (float)GetRandomValue(0, SCREEN_HEIGHT * 2/3);
        star_br[i] = (float)GetRandomValue(40, 255) / 255.0f;
    }
}

static void DrawMenuStars(float t) {
    for (int i = 0; i < 80; i++) {
        float br = star_br[i] * (0.6f + 0.4f * sinf(t * 1.2f + i * 0.7f));
        int sz = (i % 5 == 0) ? 2 : 1;
        DrawRectangle((int)star_x[i], (int)star_y[i], sz, sz,
            (Color){(unsigned char)(200*br),(unsigned char)(210*br),(unsigned char)(255*br),255});
    }
}

static void DrawLighthouse(int cx, int y, float t) {
    DrawRectangle(cx-8, y-80, 16, 80, (Color){70,72,82,255});
    DrawRectangle(cx-6, y-80, 12, 80, (Color){80,82,92,255});
    for (int i = 0; i < 3; i++) {
        DrawRectangle(cx-6, y-20-i*20, 12, 8, (Color){160,80,80,255});
    }
    DrawRectangle(cx-10, y-92, 20, 14, (Color){90,92,102,255});
    DrawRectangle(cx-12, y-80, 24, 4, (Color){70,72,82,255});
    float pulse = 0.6f + 0.4f * sinf(t * 2.0f);
    float rot = fmodf(t * 60.0f, 360.0f);
    float ang1 = (rot - 20) * DEG2RAD;
    float ang2 = (rot + 20) * DEG2RAD;
    Vector2 tip = {(float)cx, (float)(y-85)};
    Vector2 b1 = {tip.x + cosf(ang1)*180, tip.y + sinf(ang1)*60};
    Vector2 b2 = {tip.x + cosf(ang2)*180, tip.y + sinf(ang2)*60};
    DrawTriangle(tip, b1, b2, (Color){255,220,100,(unsigned char)(40*pulse)});
    DrawCircle(cx, y-85, 10, (Color){255,220,100,(unsigned char)(200*pulse)});
    DrawCircle(cx, y-85, 18, (Color){255,200,80,(unsigned char)(80*pulse)});
    DrawRectangle(cx-16, y, 32, 12, (Color){60,62,72,255});
}

static void DrawIslandSilhouette(float t) {
    int W = SCREEN_WIDTH;
    int H = SCREEN_HEIGHT;
    for (int x = 0; x < W; x++) {
        float wave = sinf(x * 0.02f + t) * 3.0f + sinf(x * 0.04f - t * 0.7f) * 2.0f;
        int wy = H * 3/4 + (int)wave;
        DrawRectangle(x, wy, 1, H - wy, (Color){10,22,55,255});
        if (x % 20 == 0) {
            float sh = sinf(x * 0.1f + t * 3.0f);
            if (sh > 0.6f) DrawRectangle(x, wy+2, 3, 1, (Color){80,120,180,100});
        }
    }
    for (int x = 0; x < W; x++) {
        float h1 = sinf(x * 0.005f) * 60.0f + 40.0f;
        float h2 = sinf(x * 0.008f + 1.0f) * 40.0f + 20.0f;
        int top = H * 3/4 - (int)(h1 + h2);
        DrawRectangle(x, top, 1, H*3/4 - top, (Color){15,22,18,255});
    }
    int trees[] = {80, 160, 240, 350, 450, 600, 700, 820, 950, 1100, 1200};
    for (int i = 0; i < 11; i++) {
        int tx = trees[i];
        float h1 = sinf(tx * 0.005f) * 60.0f + 40.0f;
        float h2 = sinf(tx * 0.008f + 1.0f) * 40.0f + 20.0f;
        int base = H * 3/4 - (int)(h1 + h2);
        DrawRectangle(tx-2, base-40, 4, 40, (Color){20,18,15,255});
        DrawTriangle(
            (Vector2){(float)tx, (float)(base-80)},
            (Vector2){(float)(tx-18), (float)(base-10)},
            (Vector2){(float)(tx+18), (float)(base-10)},
            (Color){12,25,14,255}
        );
        DrawTriangle(
            (Vector2){(float)tx, (float)(base-65)},
            (Vector2){(float)(tx-14), (float)(base-20)},
            (Vector2){(float)(tx+14), (float)(base-20)},
            (Color){16,32,18,255}
        );
    }
}

void MenuUpdate(GameState* g) {
    InitMenuStars();
    wave_timer += g->delta;
    g->menu_anim += g->delta;

    int max_items = g->has_save ? 3 : 2; 

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        g->menu_selection--;
        if (g->menu_selection < 0) g->menu_selection = max_items - 1;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        g->menu_selection++;
        if (g->menu_selection >= max_items) g->menu_selection = 0;
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        int actual = g->menu_selection;
        if (!g->has_save) {
            if (actual == 0) { 
                g->is_new_game = true;
                g->prev_screen = SCREEN_INTRO;
                g->fading_out  = true;
            } else { 
                CloseWindow();
            }
        } else {
            if (actual == 0) { 
                remove("savegame.dat");
                g->is_new_game = true;
                g->prev_screen = SCREEN_INTRO;
                g->fading_out  = true;
            } else if (actual == 1) { 
                g->prev_screen = SCREEN_GAME;
                g->fading_out  = true;
            } else { 
                CloseWindow();
            }
        }
    }
}

void MenuDraw(GameState* g) {
    float t = g->menu_anim;
    int W = SCREEN_WIDTH;
    int H = SCREEN_HEIGHT;

    // Gradient sky
    for (int y = 0; y < H * 3/4; y++) {
        float p = (float)y / (H * 3/4);
        Color top = {5, 8, 20, 255};
        Color bot = {15, 25, 50, 255};
        Color c = ColorLerp(top, bot, p);
        DrawRectangle(0, y, W, 1, c);
    }

    DrawMenuStars(t);
    DrawIslandSilhouette(t);

    // Lighthouse on left side
    DrawLighthouse(200, H*3/4, t);

    // Fog overlay at horizon
    for (int x = 0; x < W; x++) {
        float fog = 0.4f + 0.2f * sinf(x * 0.01f + t * 0.3f);
        DrawRectangle(x, H*3/4 - 40, 1, 50,
            (Color){20,30,50,(unsigned char)(fog*160)});
    }

    // Vignette
    for (int r = 0; r < 200; r++) {
        float a = (float)r / 200.0f;
        DrawRectangleLinesEx((Rectangle){(float)r,(float)r,(float)(W-r*2),(float)(H-r*2)},
            1, (Color){0,0,0,(unsigned char)((1.0f-a)*120)});
    }

    // Title panel
    float title_y = 60.0f + sinf(t * 0.8f) * 4.0f;

    // Title glow
    for (int i = 3; i > 0; i--) {
        DrawTextCentered(g->font_main, "ILHA DOS PARADOXOS",
            (int)title_y + i, 56, 3, (Color){60,100,200,(unsigned char)(40/i)});
    }
    DrawTextCentered(g->font_main, "ILHA DOS PARADOXOS", (int)title_y, 56, 3, COL_UI_ACCENT);

    // Subtitle
    DrawTextCentered(g->font_main, "Um mistério de lógica e dedução",
        (int)title_y + 70, 18, 1, (Color){130,150,190,200});

    // Decorative line
    int lw = 300;
    int lx = W/2 - lw/2;
    int ly = (int)title_y + 100;
    DrawRectangle(lx, ly, lw, 1, (Color){60,80,120,200});
    DrawRectangle(W/2-3, ly-4, 6, 9, COL_UI_ACCENT);

    // Menu items
    const char* items_with_save[]    = {"INICIAR NOVO JOGO", "CONTINUAR HISTÓRIA", "SAIR DO JOGO"};
    const char* items_without_save[] = {"INICIAR JOGO", "SAIR DO JOGO"};
    const char** items = g->has_save ? items_with_save : items_without_save;
    int   item_count   = g->has_save ? 3 : 2;

    int menu_start_y = H/2 - (item_count * 55) / 2 + 40;

    for (int i = 0; i < item_count; i++) {
        bool sel = (g->menu_selection == i);
        int  iy  = menu_start_y + i * 55;
        float pulse = sel ? (0.85f + 0.15f * sinf(t * 3.0f)) : 1.0f;

        // Selection highlight box
        if (sel) {
            int bw = 320;
            int bx = W/2 - bw/2;
            DrawRectangle(bx, iy-8, bw, 38, (Color){20,40,80,180});
            DrawRectangleLinesEx((Rectangle){(float)bx,(float)(iy-8),(float)bw,38.0f},
                1, (Color){80,120,200,(unsigned char)(180*pulse)});
            // Arrow indicators
            DrawText("<", bx + 8, iy + 2, 20, COL_UI_ACCENT);
            DrawText(">", bx + bw - 22, iy + 2, 20, COL_UI_ACCENT);
        }

        Color col = sel
            ? (Color){220,235,255,(unsigned char)(255*pulse)}
            : (Color){120,140,170,200};

        DrawTextCentered(g->font_main, items[i], iy, sel ? 22 : 20, 1, col);
    }

    // Nav hints
    DrawTextCentered(g->font_main, "↑↓ Navegar    ENTER Confirmar",
        H - 40, 15, 1, (Color){80,100,140,180});

    // Version
    DrawText("v0.1 - Slice Vertical", 10, H-22, 14, (Color){50,60,80,180});

    // Mysterious quote at bottom
    float qa = 0.5f + 0.5f * sinf(t * 0.5f);
    DrawTextCentered(g->font_main, "\"A lógica é a linguagem da ilha...\"",
        H - 80, 14, 1, (Color){80,100,140,(unsigned char)(180*qa)});
}
