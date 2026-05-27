#include "game.h"

static int base_map[MAP_HEIGHT][MAP_WIDTH] = {
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,3,3,0,0,0,0,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,3,0,0,4,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,3,0,0,0,0,0,6,6,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,3,0,0,0,4,0,0,6,6,0,0,4,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,3,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,3,0,0,0,0,0,6,6,0,0,6,6,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,3,0,0,0,0,0,0,6,6,0,0,6,6,0,0,0,5,5,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,3,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,5,5,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,6,6,0,0,0,6,0,0,6,0,0,0,0,0,4,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,6,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,4,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,4,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,6,6,0,0,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,0,0,0,0,0,5,5,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,0,0,0,0,0,5,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,4,0,0,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,6,0,0,0,0,0,0,0,0,0,6,6,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
};

void MapInit(GameState* g) {
    g->map.width  = MAP_WIDTH;
    g->map.height = MAP_HEIGHT;

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            g->map.tiles[y][x] = base_map[y][x];
            g->map.explored[y][x] = false;
            g->map.door_state[y][x] = false;
            g->map.door_needs_logic[y][x] = false;
        }
    }

    for (int x = 3; x < 25; x++) {
        if (g->map.tiles[12][x] == TILE_GRASS)
            g->map.tiles[12][x] = TILE_PATH;
        if (g->map.tiles[13][x] == TILE_GRASS)
            g->map.tiles[13][x] = TILE_PATH;
    }
    for (int y = 6; y < 22; y++) {
        if (g->map.tiles[y][10] == TILE_GRASS)
            g->map.tiles[y][10] = TILE_PATH;
        if (g->map.tiles[y][11] == TILE_GRASS)
            g->map.tiles[y][11] = TILE_PATH;
    }

    g->map.tiles[5][5]  = TILE_BUILDING;
    g->map.tiles[5][6]  = TILE_BUILDING;
    g->map.tiles[6][5]  = TILE_BUILDING;
    g->map.tiles[6][6]  = TILE_BUILDING;
    g->map.tiles[7][5]  = TILE_DOOR_CLOSED;

    g->map.tiles[5][15] = TILE_BUILDING;
    g->map.tiles[5][16] = TILE_BUILDING;
    g->map.tiles[6][15] = TILE_BUILDING;
    g->map.tiles[6][16] = TILE_BUILDING;
    g->map.tiles[7][15] = TILE_DOOR_CLOSED;

    g->map.tiles[10][18] = TILE_BUILDING;
    g->map.tiles[10][19] = TILE_BUILDING;
    g->map.tiles[11][18] = TILE_BUILDING;
    g->map.tiles[11][19] = TILE_BUILDING;
    g->map.tiles[12][18] = TILE_DOOR_CLOSED;

    for (int y = 18; y < 22; y++) {
        for (int x = 18; x < 23; x++) {
            if (g->map.tiles[y][x] == TILE_GRASS)
                g->map.tiles[y][x] = TILE_RUINS;
        }
    }

    g->map.tiles[19][20] = TILE_DOOR_CLOSED;
    g->map.door_needs_logic[19][20] = true;
    strcpy(g->map.door_condition[19][20], "Q");

    g->map.tiles[8][14] = TILE_DOOR_CLOSED;
    g->map.door_needs_logic[8][14] = true;
    strcpy(g->map.door_condition[8][14], "nP_OR_R");

    g->map.tiles[11][20] = TILE_DOOR_CLOSED;
    g->map.door_needs_logic[11][20] = true;
    strcpy(g->map.door_condition[11][20], "Q_IMPL_R");

    g->map.tiles[4][22] = TILE_BUILDING;
    g->map.tiles[4][23] = TILE_BUILDING;
    g->map.tiles[5][22] = TILE_BUILDING;
    g->map.tiles[5][23] = TILE_BUILDING;
    g->map.tiles[3][22] = TILE_BUILDING;
    g->map.tiles[3][23] = TILE_BUILDING;
    g->map.tiles[6][22] = TILE_DOOR_CLOSED;

    for (int x = 0; x < MAP_WIDTH; x++) {
        if (g->map.tiles[2][x] == TILE_GRASS) g->map.tiles[2][x] = TILE_SAND;
        if (g->map.tiles[3][x] == TILE_WATER) g->map.tiles[3][x] = TILE_SAND;
    }
}

bool TileIsWalkable(int tile) {
    switch (tile) {
        case TILE_GRASS:       return true;
        case TILE_SAND:        return true;
        case TILE_PATH:        return true;
        case TILE_RUINS:       return true;
        case TILE_DOOR_OPEN:   return true;
        default:               return false;
    }
}

bool CheckCollision(GameState* g, Vector2 pos) {
    float pw = TILE_SIZE - 6;
    float ph = TILE_SIZE - 4;
    Vector2 corners[4] = {
        {pos.x + 3,      pos.y + 4     },
        {pos.x + pw,     pos.y + 4     },
        {pos.x + 3,      pos.y + ph    },
        {pos.x + pw,     pos.y + ph    },
    };

    for (int i = 0; i < 4; i++) {
        int tx = (int)(corners[i].x / TILE_SIZE);
        int ty = (int)(corners[i].y / TILE_SIZE);
        if (tx < 0 || ty < 0 || tx >= MAP_WIDTH || ty >= MAP_HEIGHT) return true;

        int tile = g->map.tiles[ty][tx];

        if (tile == TILE_DOOR_CLOSED && g->map.door_needs_logic[ty][tx]) {
            const char* cond = g->map.door_condition[ty][tx];
            bool has_condition = false;

            if (strcmp(cond, "nP_OR_R") == 0) {
                bool not_P = g->clues[3].discovered; 
                bool has_R = g->clues[8].discovered && g->clues[8].discovered;
                has_condition = not_P || has_R;
            } else if (strcmp(cond, "Q_IMPL_R") == 0) {
                bool has_Q = g->clues[7].confirmed;
                bool has_R = g->clues[8].discovered;
                has_condition = has_Q && has_R;
            } else {
                for (int c = 0; c < g->clue_count; c++) {
                    if (strcmp(g->clues[c].tag, cond) == 0 && g->clues[c].confirmed) {
                        has_condition = true;
                        break;
                    }
                }
            }

            if (has_condition) {
                g->map.tiles[ty][tx] = TILE_DOOR_OPEN;
                if (strcmp(cond, "nP_OR_R") == 0) {
                    ShowNotification(g, "TEMPLO DESBLOQUEADO",
                        "¬P ∨ R confirmado. As anotacoes de Martim estao la dentro.", COL_UI_GOLD);
                } else if (strcmp(cond, "Q_IMPL_R") == 0) {
                    ShowNotification(g, "FORJA DESBLOQUEADA",
                        "Q → R confirmado. O diario do Ferreiro pode revelar mais.", COL_PARADOXAL);
                } else {
                    ShowNotification(g, "PORTA ABERTA",
                        "A proposicao logica ativou o mecanismo antigo.", COL_UI_GOLD);
                }
                return false;
            }
        }

        if (!TileIsWalkable(tile)) return true;
    }
    return false;
}

void UpdateExplored(GameState* g) {
    int px = (int)(g->player.pos.x / TILE_SIZE);
    int py = (int)(g->player.pos.y / TILE_SIZE);
    int radius = 5;
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                int tx = px + dx, ty = py + dy;
                if (tx >= 0 && ty >= 0 && tx < MAP_WIDTH && ty < MAP_HEIGHT)
                    g->map.explored[ty][tx] = true;
            }
        }
    }
}

static Color GetTileColor(int tile, int x, int y, float t) {
    bool checker = (x + y) % 2 == 0;
    switch (tile) {
        case TILE_GRASS:
            return checker ? COL_GRASS : COL_GRASS2;
        case TILE_WATER: {
            float wave = sinf(x * 0.3f + t) * 0.5f + 0.5f;
            return ColorLerp(COL_WATER, COL_WATER2, wave);
        }
        case TILE_SAND:
            return checker ? COL_SAND : (Color){110,92,64,255};
        case TILE_STONE:
            return checker ? COL_STONE : COL_STONE2;
        case TILE_TREE:
            return checker ? COL_TREE : COL_TREE2;
        case TILE_BUILDING:
            return checker ? COL_BUILDING : COL_BUILDING2;
        case TILE_PATH:
            return checker ? COL_PATH : (Color){80,72,58,255};
        case TILE_RUINS:
            return checker ? COL_RUINS : (Color){55,50,46,255};
        case TILE_DOOR_CLOSED:
            return COL_DOOR_C;
        case TILE_DOOR_OPEN:
            return COL_DOOR_O;
        default:
            return DARKGRAY;
    }
}

static void DrawTileDecoration(int tile, int px, int py, int x, int y, float t) {
    switch (tile) {
        case TILE_TREE: {
            int cx = px + TILE_SIZE/2;
            int cy = py + TILE_SIZE/2;
            DrawEllipse(cx+2, cy+10, 10, 5, (Color){0,0,0,60});
            DrawRectangle(cx-2, cy, 4, 12, (Color){80,50,30,255});
            DrawCircle(cx, cy-4, 12, COL_TREE);
            DrawCircle(cx-4, cy-8, 8, COL_TREE2);
            DrawCircle(cx+4, cy-7, 7, (Color){22,58,26,255});
            break;
        }
        case TILE_BUILDING: {
            DrawRectangle(px+2, py+2, TILE_SIZE-4, 4, (Color){100,80,60,255});
            if ((x+y)%3 == 0)
                DrawRectangle(px+8, py+8, 10, 8, (Color){60,80,120,180});
            break;
        }
        case TILE_DOOR_CLOSED: {
            DrawRectangle(px+6, py+4, TILE_SIZE-12, TILE_SIZE-8, (Color){90,60,35,255});
            DrawCircle(px+TILE_SIZE-10, py+TILE_SIZE/2, 2, (Color){180,150,80,255});
            DrawRectangleLinesEx((Rectangle){(float)(px+5),(float)(py+3),(float)(TILE_SIZE-10),(float)(TILE_SIZE-6)},
                1, (Color){60,40,20,200});
            break;
        }
        case TILE_DOOR_OPEN: {
            DrawRectangle(px+2, py+4, 4, TILE_SIZE-6, (Color){60,40,20,200});
            break;
        }
        case TILE_RUINS: {
            if ((x*3+y*7) % 5 == 0) {
                DrawRectangle(px+10, py+4, 6, TILE_SIZE-8, (Color){75,70,65,255});
                DrawRectangle(px+8, py+2, 10, 4, (Color){80,75,70,255});
            }
            if ((x+y*2) % 7 == 0) {
                DrawText("∧", px+4, py+4, 10, (Color){120,110,100,100});
            }
            break;
        }
        case TILE_WATER: {
            float rip = sinf(t * 2.0f + x * 0.5f + y * 0.3f);
            if (rip > 0.8f) {
                DrawRectangle(px+4, py+TILE_SIZE/2, TILE_SIZE-8, 1, (Color){80,120,180,100});
            }
            break;
        }
        case TILE_SAND: {
            if ((x*5+y*3)%4 == 0)
                DrawRectangle(px+GetRandomValue(0,TILE_SIZE-2), py+GetRandomValue(0,TILE_SIZE-2),
                    1, 1, (Color){140,120,85,100});
            break;
        }
        default: break;
    }
}

void MapDraw(GameState* g) {
    float t = g->menu_anim;
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    int cam_tx = (int)((g->camera.target.x - sw/g->camera.zoom/2) / TILE_SIZE) - 1;
    int cam_ty = (int)((g->camera.target.y - sh/g->camera.zoom/2) / TILE_SIZE) - 1;
    int cam_bx = cam_tx + (int)(sw/g->camera.zoom/TILE_SIZE) + 3;
    int cam_by = cam_ty + (int)(sh/g->camera.zoom/TILE_SIZE) + 3;

    if (cam_tx < 0) cam_tx = 0;
    if (cam_ty < 0) cam_ty = 0;
    if (cam_bx > MAP_WIDTH)  cam_bx = MAP_WIDTH;
    if (cam_by > MAP_HEIGHT) cam_by = MAP_HEIGHT;

    for (int y = cam_ty; y < cam_by; y++) {
        for (int x = cam_tx; x < cam_bx; x++) {
            int tile = g->map.tiles[y][x];
            int px = x * TILE_SIZE;
            int py = y * TILE_SIZE;

            Color col = GetTileColor(tile, x, y, t);

            if (!g->map.explored[y][x]) {
                col.r = col.r / 4;
                col.g = col.g / 4;
                col.b = col.b / 4;
            }

            DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, col);
        }
    }

    for (int y = cam_ty; y < cam_by; y++) {
        for (int x = cam_tx; x < cam_bx; x++) {
            if (!g->map.explored[y][x]) continue;
            int tile = g->map.tiles[y][x];
            int px = x * TILE_SIZE;
            int py = y * TILE_SIZE;
            DrawTileDecoration(tile, px, py, x, y, t);
        }
    }

    float lh_pulse = 0.5f + 0.5f * sinf(t * 1.5f);
    int lhx = 23 * TILE_SIZE + TILE_SIZE/2;
    int lhy = 4 * TILE_SIZE;
    DrawCircle(lhx, lhy, 30 + (int)(10*lh_pulse), (Color){255,220,100,(unsigned char)(30*lh_pulse)});
    DrawCircle(lhx, lhy, 12, (Color){255,220,100,(unsigned char)(200*lh_pulse)});

    if (g->map.explored[19][20]) {
        DrawText("¬P → Q", 20*TILE_SIZE + 2, 18*TILE_SIZE + 4, 8, (Color){150,130,100,180});
        DrawText("∴ ¬P", 20*TILE_SIZE + 2, 19*TILE_SIZE + 4, 8, (Color){150,130,100,180});
    }

    for (int y = cam_ty; y < cam_by; y++) {
        for (int x = cam_tx; x < cam_bx; x++) {
            if (!g->map.explored[y][x]) continue;
            DrawRectangleLinesEx(
                (Rectangle){(float)(x*TILE_SIZE),(float)(y*TILE_SIZE),(float)TILE_SIZE,(float)TILE_SIZE},
                0.3f, (Color){0,0,0,30});
        }
    }
}
