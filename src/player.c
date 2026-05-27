#include "game.h"

void PlayerInit(GameState* g) {
    g->player.pos    = (Vector2){10 * TILE_SIZE, 13 * TILE_SIZE};
    g->player.speed  = PLAYER_SPEED;
    g->player.facing = 0;
    g->player.anim_frame = 0;
    g->player.anim_timer = 0;
    g->player.moving     = false;
    g->player.in_deduction_mode = false;
    g->player.has_lighthouse_code = false;
    g->player.lighthouse_fragments = 0;
}

void PlayerUpdate(GameState* g) {
    if (g->current_screen != SCREEN_GAME) return;

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_J)) {
        g->current_screen = SCREEN_DEDUCTION;
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P)) {
        g->current_screen = SCREEN_PAUSE;
        return;
    }
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
        NPC* nearby = GetNearbyNPC(g);
        if (nearby) {
            int idx = (int)(nearby - g->npcs);
            StartDialogue(g, idx);
            return;
        }
    }

    Vector2 vel = {0, 0};
    bool moving = false;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { vel.y = -g->player.speed; g->player.facing = 1; moving = true; }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))   { vel.y =  g->player.speed; g->player.facing = 0; moving = true; }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))   { vel.x = -g->player.speed; g->player.facing = 2; moving = true; }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))  { vel.x =  g->player.speed; g->player.facing = 3; moving = true; }

    if (vel.x != 0 && vel.y != 0) {
        vel.x *= 0.707f;
        vel.y *= 0.707f;
    }

    g->player.moving = moving;

    Vector2 new_pos = g->player.pos;
    new_pos.x += vel.x;
    if (!CheckCollision(g, new_pos)) {
        g->player.pos.x = new_pos.x;
    }
    new_pos = g->player.pos;
    new_pos.y += vel.y;
    if (!CheckCollision(g, new_pos)) {
        g->player.pos.y = new_pos.y;
    }

    if (moving) {
        g->player.anim_timer += g->delta;
        if (g->player.anim_timer > 0.12f) {
            g->player.anim_timer = 0;
            g->player.anim_frame = (g->player.anim_frame + 1) % 4;
        }
    } else {
        g->player.anim_frame = 0;
    }

    float max_x = (MAP_WIDTH  - 1) * TILE_SIZE;
    float max_y = (MAP_HEIGHT - 1) * TILE_SIZE;
    if (g->player.pos.x < 0)     g->player.pos.x = 0;
    if (g->player.pos.y < 0)     g->player.pos.y = 0;
    if (g->player.pos.x > max_x) g->player.pos.x = max_x;
    if (g->player.pos.y > max_y) g->player.pos.y = max_y;

    UpdateExplored(g);
}

static void DrawPlayerPixelArt(int px, int py, int facing, int frame, float t) {
    Color body   = {70, 90, 130, 255};   
    Color skin   = {220, 180, 150, 255}; 
    Color hair   = {60, 40, 30, 255};
    Color boot   = {50, 40, 30, 255};

    int bob = (frame == 1 || frame == 3) ? 1 : 0;
    py += bob;

    DrawEllipse(px + TILE_SIZE/2, py + TILE_SIZE - 4, 8, 3, (Color){0,0,0,80});

    if (facing != 1) { 
        DrawRectangle(px+8,  py+22+bob, 5, 6, boot);
        DrawRectangle(px+15, py+22+bob, 5, 6, boot);
    }

    DrawRectangle(px+6, py+10, 20, 14, body);
    DrawRectangle(px+5, py+12, 22, 10, body); 

    int arm_swing = (frame % 2 == 0) ? 0 : 1;
    DrawRectangle(px+2,  py+11+arm_swing, 5, 10, body);
    DrawRectangle(px+25, py+11-arm_swing, 5, 10, body); 

    float lan_glow = 0.7f + 0.3f * sinf(t * 3.0f);
    DrawRectangle(px+27, py+12, 4, 6, (Color){180,150,60,255});
    DrawCircle(px+29, py+15, (int)(5*lan_glow), (Color){255,200,80,(unsigned char)(120*lan_glow)});

    DrawRectangle(px+9, py+2, 14, 12, skin);
    DrawRectangle(px+9, py+2, 14, 5, hair);
    if (facing == 0 || facing == 3 || facing == 2) { 
        DrawRectangle(px+11, py+9, 2, 2, (Color){30,30,40,255});
        DrawRectangle(px+18, py+9, 2, 2, (Color){30,30,40,255});
    }
    DrawRectangle(px+7, py+10, 18, 4, (Color){120,60,60,255});

    if (false) { 
        DrawCircle(px+TILE_SIZE/2, py+TILE_SIZE/2, 20, (Color){100,160,255,30});
    }
}

void PlayerDraw(GameState* g) {
    int px = (int)g->player.pos.x;
    int py = (int)g->player.pos.y;

    DrawPlayerPixelArt(px, py, g->player.facing, g->player.anim_frame, g->menu_anim);

    NPC* nearby = GetNearbyNPC(g);
    if (nearby && g->current_screen == SCREEN_GAME) {
        float pulse = 0.6f + 0.4f * sinf(g->menu_anim * 4.0f);
        DrawText("[E]", px + TILE_SIZE/2 - 8, py - 18, 14,
            (Color){200,220,255,(unsigned char)(220*pulse)});
    }
}
