#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

/* Internal Render Resolution */
#define FIRE_WIDTH 552
#define FIRE_HEIGHT 300
#define NUM_STARS 1000

/* Classic 37-color fire palette */
static const uint32_t firePalette[37] = {
    0xFF070707, 0xFF1f0707, 0xFF2f0f07, 0xFF470f07, 0xFF571707, 0xFF671f07,
    0xFF771f07, 0xFF8f2707, 0xFF9f2f07, 0xFFaf3f07, 0xFFbf4707, 0xFFc74707,
    0xFFDF4F07, 0xFFdf5707, 0xFFdf5707, 0xFFd75f07, 0xFFd7670f, 0xFFcf6f0f,
    0xFFcf770f, 0xFFcf7f0f, 0xFFCF8717, 0xFFc78717, 0xFFc78f17, 0xFFc7971f,
    0xFFbf9f1f, 0xFFbf9f1f, 0xFFbfa727, 0xFFbfaf27, 0xFFBfb727, 0xFFbfbf2f,
    0xFFcfc72f, 0xFFcfcf37, 0xFFcfdf3f, 0xFFdfdf47, 0xFFefef4f, 0xFFffff5b,
    0xFFffffff 
};

/* Expanded 3x5 font (Added m, a, d, e for the intro screen) */
const char* font3x5[][5] = {
    {"###", "# #", "###", "# #", "# #"}, // 0: A
    {"## ", "# #", "## ", "# #", "## "}, // 1: B
    {" ##", "#  ", "#  ", "#  ", " ##"}, // 2: C
    {"## ", "# #", "# #", "# #", "## "}, // 3: D
    {"###", "#  ", "## ", "#  ", "###"}, // 4: E
    {"###", "#  ", "## ", "#  ", "#  "}, // 5: F
    {" ##", "#  ", "# #", "# #", " ##"}, // 6: G
    {"# #", "# #", "###", "# #", "# #"}, // 7: H
    {"###", " # ", " # ", " # ", "###"}, // 8: I
    {"  #", "  #", "  #", "# #", " # "}, // 9: J
    {"# #", "# #", "## ", "# #", "# #"}, // 10: K
    {"#  ", "#  ", "#  ", "#  ", "###"}, // 11: L
    {"# #", "###", "###", "# #", "# #"}, // 12: M 
    {"###", "# #", "# #", "# #", "# #"}, // 13: N 
    {"###", "# #", "# #", "# #", "###"}, // 14: O
    {"###", "# #", "###", "#  ", "#  "}, // 15: P
    {"###", "# #", "# #", "###", "  #"}, // 16: Q
    {"###", "# #", "## ", "# #", "# #"}, // 17: R
    {" ##", "#  ", " # ", "  #", "## "}, // 18: S
    {"###", " # ", " # ", " # ", " # "}, // 19: T
    {"# #", "# #", "# #", "# #", "###"}, // 20: U
    {"# #", "# #", "# #", "# #", " # "}, // 21: V
    {"# #", "# #", "###", "###", "# #"}, // 22: W 
    {"# #", "# #", " # ", "# #", "# #"}, // 23: X
    {"# #", "# #", " # ", " # ", " # "}, // 24: Y
    {"###", "  #", " # ", "#  ", "###"}, // 25: Z
    {"   ", "   ", "   ", "   ", "   "}, // 26: Space
    {"   ", "   ", "   ", "   ", " # "}, // 27: .
    {"   ", "   ", "###", "   ", "   "}, // 28: -
    {" # ", "###", "# #", "###", " # "}, // 29: *
    {" # ", " # ", " # ", "   ", " # "}, // 30: ! 
    {"#  ", "# #", "## ", "# #", "# #"}, // 31: k 
    {"#  ", "#  ", "## ", "# #", "## "}, // 32: b 
    {"# #", "# #", " ##", "  #", "## "}, // 33: y 
    {"   ", "# #", "###", "# #", "# #"}, // 34: m (lowered)
    {"   ", " ##", "  #", " ##", "###"}, // 35: a 
    {"  #", "  #", " ##", "# #", " ##"}, // 36: d 
    {"   ", " ##", "###", "#  ", " ##"}  // 37: e 
};

// FIXED: Explicit (int) cast prevents -Wsign-compare compiler warnings
#define FONT_MAX_INDEX ((int)(sizeof(font3x5) / sizeof(font3x5[0])) - 1)

uint8_t firePixels[FIRE_HEIGHT][FIRE_WIDTH];
uint32_t frameBuffer[FIRE_HEIGHT][FIRE_WIDTH];
struct Star { float x, y, z; } stars[NUM_STARS];

/* Bresenham's Line Algorithm */
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        if (x0 >= 0 && x0 < FIRE_WIDTH && y0 >= 0 && y0 < FIRE_HEIGHT) {
            frameBuffer[y0][x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void drawChar(int ch_idx, float cx, float cy, float scale, uint32_t color) {
    if (ch_idx < 0 || ch_idx > FONT_MAX_INDEX) return; 
    int size = (int)(2.0f * scale); 
    if (size < 1) size = 1;

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 3; x++) {
            if (font3x5[ch_idx][y][x] != ' ') {
                int px = (int)(cx + x * size);
                int py = (int)(cy + y * size);
                for(int dy = 0; dy < size; dy++) {
                    for(int dx = 0; dx < size; dx++) {
                        if(px+dx >= 0 && px+dx < FIRE_WIDTH && py+dy >= 0 && py+dy < FIRE_HEIGHT) {
                            frameBuffer[py+dy][px+dx] = color;
                        }
                    }
                }
            }
        }
    }
}

/* Additive Blending Character renderer */
void drawGlowChar(int ch_idx, int cx, int cy, uint32_t color) {
    if (ch_idx < 0 || ch_idx > FONT_MAX_INDEX) return; 
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 3; x++) {
            if (font3x5[ch_idx][y][x] != ' ') {
                int px = cx + x;
                int py = cy + y;
                if(px >= 0 && px < FIRE_WIDTH && py >= 0 && py < FIRE_HEIGHT) {
                    uint32_t bg = frameBuffer[py][px];
                    int r = ((bg >> 16) & 0xFF) + ((color >> 16) & 0xFF);
                    int g = ((bg >> 8) & 0xFF) + ((color >> 8) & 0xFF);
                    int b = (bg & 0xFF) + (color & 0xFF);
                    frameBuffer[py][px] = 0xFF000000 | ((r>255?255:r)<<16) | ((g>255?255:g)<<8) | (b>255?255:b);
                }
            }
        }
    }
}

void updateFire() {
    for (int x = 0; x < FIRE_WIDTH; x++) {
        for (int y = 1; y < FIRE_HEIGHT; y++) {
            int srcHeat = firePixels[y][x];
            if (srcHeat == 0) {
                firePixels[y - 1][x] = 0;
                continue;
            }
            int randIdx = rand() % 3; 
            int dstX = x - randIdx + 1;
            int dstY = y - 1;
            if (dstX >= 0 && dstX < FIRE_WIDTH && dstY >= 0) {
                int newHeat = srcHeat - (randIdx & 1);
                firePixels[dstY][dstX] = newHeat > 0 ? newHeat : 0;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    int window_width = 1280;
    int window_height = 720;
    bool isFullscreen = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0) {
            isFullscreen = true;
        } else if ((strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) && i + 1 < argc) {
            window_width = atoi(argv[++i]); 
        } else if ((strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--height") == 0) && i + 1 < argc) {
            window_height = atoi(argv[++i]); 
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("Fatal: Failed to init SDL: %s", SDL_GetError());
        return 1;
    }

    Mix_Music *bgm = NULL;
    if (Mix_OpenAudio(22500, MIX_DEFAULT_FORMAT, 2, 2048) == 0) {
        bgm = Mix_LoadMUS("snd1.mp3");
        if (bgm) {
            // FIXED: Fades the music in over 4000 milliseconds (4 seconds)
            Mix_FadeInMusic(bgm, -1, 4000); 
        } else {
        SDL_Log("Warning: Audio failed to initialize, continuing without sound. %s", Mix_GetError());
    }

    Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
    if (isFullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_ShowCursor(SDL_DISABLE);
    }

    SDL_Window* window = SDL_CreateWindow("Linux Demoscene by k!M (C/SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, windowFlags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FIRE_WIDTH, FIRE_HEIGHT);
    SDL_RenderSetLogicalSize(renderer, FIRE_WIDTH, FIRE_HEIGHT);

    for(int i=0; i<NUM_STARS; i++) {
        stars[i].x = (rand() % 2000) - 1000;
        stars[i].y = (rand() % 2000) - 1000;
        stars[i].z = (rand() % 255) + 1;
    }

    const char* scrollText = "*** HELLO  DEMOSCENE!  ***  WELCOME  TO  THE  LINUX  TERMINAL  ***  ENJOY  THIS  PARALLAX  STARFIELD  WITH  SPHERICAL  TEXT  SPIN  AND  PIXEL  FIRE  ***  GREETINGS  FROM  NORWAY!   ***";
    
    // State machine variables
    int demo_state = 0; // 0 = Intro, 1 = Main Demo
    int intro_frames = 0;
    int time_counter = 0;
    
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        memset(frameBuffer, 0, sizeof(frameBuffer));

        /* --- GLOBAL BACKGROUND UPDATES --- */
        // We update the fire invisibly during the intro so it's fully blazing when the scene cuts
        for (int x = 0; x < FIRE_WIDTH; x++) firePixels[FIRE_HEIGHT - 1][x] = 36;
        updateFire();
        
        float starSpeed = 3.0f;

        /* --- STATE 0: INTRO FADE-IN SCREEN --- */
        if (demo_state == 0) {
            intro_frames++;
            
            // Silently push the stars forward in the background
            for(int i=0; i<NUM_STARS; i++) {
                stars[i].z -= starSpeed;
                if (stars[i].z <= 1.0f) {
                    stars[i].z = 255.0f;
                    stars[i].x = (rand() % 2000) - 1000;
                    stars[i].y = (rand() % 2000) - 1000;
                }
            }

            // Calculate Fade Alpha (60 FPS base)
            int alpha = 0;
            if (intro_frames < 60) alpha = (intro_frames * 255) / 60;               // Fade In
            else if (intro_frames < 150) alpha = 255;                               // Hold
            else if (intro_frames < 210) alpha = 255 - ((intro_frames - 150) * 255) / 60; // Fade Out
            else alpha = 0;                                                         // Black Screen Hold

            // Switch to Main Demo at 4 seconds (240 frames)
            if (intro_frames >= 240) {
                demo_state = 1;
            } else if (alpha > 0) {
                // "made by k!M" -> indices: m, a, d, e, space, b, y, space, k, !, M
                int intro_indices[] = {34, 35, 36, 37, 26, 32, 33, 26, 31, 30, 12};
                
                float scale = 1.0f;
                int size = 2; // Pixel block size for scale 1.0
                
                // Math to perfectly center the text string on screen
                int char_width = 3 * size;
                int spacing = 1 * size;
                int total_width = 11 * char_width + 10 * spacing;
                int start_x = (FIRE_WIDTH - total_width) / 2;
                int start_y = (FIRE_HEIGHT - (5 * size)) / 2;

                for (int i = 0; i < 11; i++) {
                    drawChar(intro_indices[i], start_x + i * (char_width + spacing), start_y, scale, 0xFF000000 | (alpha<<16) | (alpha<<8) | alpha);
                }
            }
        } 
        
        /* --- STATE 1: MAIN DEMO --- */
        else {
            /* Warp-Speed Star Trails */
            for(int i=0; i<NUM_STARS; i++) {
                float old_z = stars[i].z;
                stars[i].z -= starSpeed;
                
                if (stars[i].z <= 1.0f) {
                    stars[i].z = 255.0f;
                    stars[i].x = (rand() % 2000) - 1000;
                    stars[i].y = (rand() % 2000) - 1000;
                    old_z = stars[i].z; 
                }
                
                int px = (int)((stars[i].x / stars[i].z) * 120 + (FIRE_WIDTH / 2));
                int py = (int)((stars[i].y / stars[i].z) * 120 + (FIRE_HEIGHT / 2));
                
                int prev_px = (int)((stars[i].x / old_z) * 120 + (FIRE_WIDTH / 2));
                int prev_py = (int)((stars[i].y / old_z) * 120 + (FIRE_HEIGHT / 2));
                
                int twinkle = rand() % 20;
                int intensity = (int)(255 - stars[i].z) - twinkle;
                if (intensity < 0) intensity = 0;
                if (intensity > 255) intensity = 255;
                
                uint8_t r = intensity, g = intensity, b = intensity;
                if (i % 3 == 0) b = (intensity + 50 > 255) ? 255 : intensity + 50;
                else if (i % 4 == 0) { 
                    r = (intensity + 50 > 255) ? 255 : intensity + 50; 
                    g = (intensity + 20 > 255) ? 255 : intensity + 20;
                }
                
                drawLine(prev_px, prev_py, px, py, 0xFF000000 | (r<<16) | (g<<8) | b);
            }

            /* Spherical Text Spin */
            float radius = 110.0f;
            for (int i = 0; scrollText[i] != '\0'; i++) {
                float theta = (time_counter * 0.02f) - (i * 0.35f); 
                
                if (theta < 0.0f || theta > 3.14159265f) continue; 
                
                float z = sin(theta);
                float x = cos(theta);
                
                float scale = 0.5f + (z * 1.5f); 
                float cx = (FIRE_WIDTH / 2.0f) + (x * radius) - (scale * 4.0f); 
                float cy = (FIRE_HEIGHT / 3.0f) + (sin(time_counter * 0.02f + x * 2.0f) * 40.0f); 

                int char_idx = 26;
                char c = scrollText[i];
                if (c >= 'A' && c <= 'Z') char_idx = c - 'A';
                else if (c == '.') char_idx = 27;
                else if (c == '-') char_idx = 28;
                else if (c == '*') char_idx = 29;

                uint8_t depthColor = (uint8_t)(50 + z * 205);
                drawChar(char_idx, cx, cy, scale, 0xFF000000 | (0<<16) | (depthColor<<8) | 255);
            }

            /* Overlay Fire */
            for (int y = 0; y < FIRE_HEIGHT; y++) {
                for (int x = 0; x < FIRE_WIDTH; x++) {
                    int heat = firePixels[y][x];
                    if (heat > 3) frameBuffer[y][x] = firePalette[heat];
                }
            }


        /* --- POST-PROCESSING: CRT SCANLINES --- 
           (Applied to both Intro screen and Main demo for visual consistency) */
        for (int y = 0; y < FIRE_HEIGHT; y += 2) {
            for (int x = 0; x < FIRE_WIDTH; x++) {
                uint32_t c = frameBuffer[y][x];
                uint8_t r = ((c >> 16) & 0xFF) / 2;
                uint8_t g = ((c >> 8) & 0xFF) / 2;
                uint8_t b = (c & 0xFF) / 2;
                frameBuffer[y][x] = 0xFF000000 | (r<<16) | (g<<8) | b;
            }
        }

        /* Glowing Watermark (Bottom Right) */
            int wm_indices[] = {32, 33, 26, 31, 30, 12}; // by k!M
            int wm_x = FIRE_WIDTH - 28;      
            int wm_y = FIRE_HEIGHT - 8;      
            
            for(int i=0; i<6; i++) { 
                int cx = wm_x + (i * 4); 
                for(int oy=-1; oy<=1; oy++) {
                    for(int ox=-1; ox<=1; ox++) {
                        if(ox != 0 || oy != 0) {
                            drawGlowChar(wm_indices[i], cx + ox, wm_y + oy, 0xFF002244); 
                        }
                    }
                }
                drawGlowChar(wm_indices[i], cx, wm_y, 0xFF88FFFF);
            }

            time_counter++;
        }

        /* --- PUSH TO GPU --- */
        SDL_UpdateTexture(texture, NULL, frameBuffer, FIRE_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL); 
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    if (isFullscreen) SDL_ShowCursor(SDL_ENABLE); 

    if (bgm) Mix_FreeMusic(bgm);
    Mix_CloseAudio();
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}