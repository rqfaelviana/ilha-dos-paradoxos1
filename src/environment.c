#include "game.h"
#include <time.h>

void EnvInit(GameState* g) {
    g->env.time_of_day = 8.0f;  
    g->env.time_speed  = 0.05f; 
    g->env.weather     = WEATHER_FOG;
    g->env.fog_alpha   = 0.4f;
    g->env.is_night    = false;

    for (int i = 0; i < 200; i++) {
        g->env.rain_drops[i].x = (float)GetRandomValue(0, SCREEN_WIDTH);
        g->env.rain_drops[i].y = (float)GetRandomValue(0, SCREEN_HEIGHT);
        g->env.rain_speed[i]   = (float)GetRandomValue(8, 16);
    }

    
    for (int i = 0; i < 20; i++) {
        g->env.fog_patches[i].x = (float)GetRandomValue(0, SCREEN_WIDTH);
        g->env.fog_patches[i].y = (float)GetRandomValue(0, SCREEN_HEIGHT);
        g->env.fog_alpha_individual[i] = (float)GetRandomValue(20, 80) / 255.0f;
    }
}

void EnvUpdate(GameState* g) {
    
    g->env.time_of_day += g->env.time_speed * g->delta;
    if (g->env.time_of_day >= 24.0f) g->env.time_of_day -= 24.0f;

    float tod = g->env.time_of_day;
    g->env.is_night = (tod < 5.0f || tod >= 20.0f);

    
    for (int i = 0; i < 20; i++) {
        g->env.fog_patches[i].x += sinf(g->menu_anim * 0.2f + i) * 0.3f;
        g->env.fog_patches[i].y += cosf(g->menu_anim * 0.15f + i) * 0.2f;
        if (g->env.fog_patches[i].x > SCREEN_WIDTH + 100)
            g->env.fog_patches[i].x = -100;
        if (g->env.fog_patches[i].x < -100)
            g->env.fog_patches[i].x = SCREEN_WIDTH + 100;
        if (g->env.fog_patches[i].y > SCREEN_HEIGHT + 50)
            g->env.fog_patches[i].y = -50;

        g->env.fog_alpha_individual[i] =
            (float)GetRandomValue(15, 60) / 255.0f *
            (0.8f + 0.2f * sinf(g->menu_anim * 0.5f + i));
    }

    
    if (g->env.weather == WEATHER_RAIN || g->env.weather == WEATHER_STORM) {
        for (int i = 0; i < 200; i++) {
            g->env.rain_drops[i].y += g->env.rain_speed[i];
            g->env.rain_drops[i].x += 0.5f;
            if (g->env.rain_drops[i].y > SCREEN_HEIGHT) {
                g->env.rain_drops[i].y = 0;
                g->env.rain_drops[i].x = (float)GetRandomValue(0, SCREEN_WIDTH);
            }
        }
    }

    g->env.rain_timer += g->delta;
    if (g->env.rain_timer > 30.0f) {
        g->env.rain_timer = 0;
        int r = GetRandomValue(0, 2);
        switch (r) {
            case 0: g->env.weather = WEATHER_FOG;   break;
            case 1: g->env.weather = WEATHER_RAIN;  break;
            case 2: g->env.weather = WEATHER_CLEAR;  break;
        }
    }
}

void EnvDraw(GameState* g) {
    float t = g->menu_anim;
    int lantern_positions[][2] = {{10,12},{11,12},{13,12},{15,12},{17,12},{11,6},{11,9}};
    for (int i = 0; i < 7; i++) {
        int lx = lantern_positions[i][0] * TILE_SIZE + TILE_SIZE/2;
        int ly = lantern_positions[i][1] * TILE_SIZE + TILE_SIZE/2;

        
        int tx = lantern_positions[i][0];
        int ty = lantern_positions[i][1];
        if (!g->map.explored[ty][tx]) continue;

        float glow = 0.6f + 0.4f * sinf(t * 2.0f + i * 0.8f);
      
        DrawRectangle(lx-1, ly-12, 2, 12, (Color){70,60,50,255});
        
        DrawCircle(lx, ly-14, (int)(8*glow), (Color){255,200,80,(unsigned char)(60*glow)});
        DrawCircle(lx, ly-14, 4, (Color){255,220,120,(unsigned char)(200*glow)});
    }
}


void EnvDrawOverlay(GameState* g) {
    int W = GetScreenWidth();
    int H = GetScreenHeight();
    float t = g->menu_anim;
    float tod = g->env.time_of_day;

    Color sky_tint = {0,0,0,0};
    if (tod >= 20.0f || tod < 5.0f) {
        
        float night = 1.0f;
        if (tod >= 20.0f) night = (tod - 20.0f) / 2.0f;
        if (tod < 5.0f)   night = (5.0f - tod) / 2.0f;
        if (night > 1.0f) night = 1.0f;
        sky_tint = (Color){5, 8, 30, (unsigned char)(140 * night)};
    } else if (tod < 7.0f) {
        
        float d = (7.0f - tod) / 2.0f;
        sky_tint = (Color){60, 30, 10, (unsigned char)(80 * d)};
    } else if (tod > 17.0f) {
   
        float d = (tod - 17.0f) / 3.0f;
        sky_tint = (Color){80, 30, 10, (unsigned char)(80 * d)};
    }
    if (sky_tint.a > 0) {
        DrawRectangle(0, 0, W, H, sky_tint);
    }

    
    if (g->env.weather == WEATHER_FOG || g->env.weather == WEATHER_STORM) {
     
        for (int i = 0; i < 20; i++) {
            float fx = g->env.fog_patches[i].x;
            float fy = g->env.fog_patches[i].y;
            float fa = g->env.fog_alpha_individual[i];
            float fr = 80.0f + sinf(t * 0.3f + i) * 30.0f;
            DrawCircle((int)fx, (int)fy, (int)fr,
                (Color){30,45,65,(unsigned char)(fa*255)});
        }
       
        for (int y = 0; y < H; y += 80) {
            float band = 0.3f + 0.2f * sinf(t * 0.4f + y * 0.01f);
            DrawRectangle(0, y + (int)(sinf(t*0.3f+y*0.02f)*8),
                W, 30, (Color){25,38,55,(unsigned char)(band*100)});
        }
    }


    if (g->env.weather == WEATHER_RAIN || g->env.weather == WEATHER_STORM) {
        int rain_count = (g->env.weather == WEATHER_STORM) ? 200 : 120;
        for (int i = 0; i < rain_count; i++) {
            int rx = (int)g->env.rain_drops[i].x;
            int ry = (int)g->env.rain_drops[i].y;
            int rlen = (int)(g->env.rain_speed[i] * 0.8f);
            DrawLine(rx, ry, rx+2, ry+rlen, (Color){120,150,200,80});
        }
     
        DrawRectangle(0, 0, W, H, (Color){20,30,50,30});
    }

    if (g->env.is_night) {
   
        srand(42);
        for (int i = 0; i < 60; i++) {
            int sx = rand() % W;
            int sy = rand() % (H/3);
            float br = 0.4f + 0.6f * sinf(t * 1.5f + i * 0.9f);
            DrawRectangle(sx, sy, 1+(i%3==0), 1+(i%3==0),
                (Color){200,210,255,(unsigned char)(br*160)});
        }
        srand((unsigned int)time(NULL));
    }


    if (g->env.is_night) {
        float moon_x = W * 0.8f;
        float moon_y = H * 0.12f;
        DrawCircle((int)moon_x, (int)moon_y, 18, (Color){200,210,240,200});
        DrawCircle((int)(moon_x+5), (int)(moon_y-3), 16, (Color){5,8,20,180});
        
        DrawCircle((int)moon_x, (int)moon_y, 30, (Color){180,190,220,40});
    }


    if (tod > 6.0f && tod < 18.0f) {
        float sun_progress = (tod - 6.0f) / 12.0f; 
        float sun_x = W * sun_progress;
        float sun_y = H * 0.15f - sinf(sun_progress * 3.14159f) * H * 0.1f;
        float sun_glow = 0.3f + 0.1f * sinf(t * 0.5f);

        DrawCircle((int)sun_x, (int)sun_y, 20, (Color){255,220,80,180});
        DrawCircle((int)sun_x, (int)sun_y, 35, (Color){255,200,60,(unsigned char)(80*sun_glow)});
    }


    for (int r = 0; r < 120; r++) {
        float a = (float)r / 120.0f;
        float va = (1.0f - a * a) * 0.5f;
        DrawRectangleLinesEx((Rectangle){(float)r,(float)r,(float)(W-r*2),(float)(H-r*2)},
            1, (Color){0,0,0,(unsigned char)(va*200)});
    }

    if (g->lighthouse_activated) {
        float beam_rot = fmodf(t * 40.0f, 360.0f);
        float beam_len = 400.0f;
        float ang = beam_rot * DEG2RAD;
 
        int lhsx = W/2 + 200;
        int lhsy = H/2 - 100;
        Vector2 tip = {(float)lhsx, (float)lhsy};
        Vector2 b1 = {tip.x + cosf(ang-0.15f)*beam_len, tip.y + sinf(ang-0.15f)*beam_len};
        Vector2 b2 = {tip.x + cosf(ang+0.15f)*beam_len, tip.y + sinf(ang+0.15f)*beam_len};
        DrawTriangle(tip, b1, b2, (Color){255,220,80,40});
    }
}
