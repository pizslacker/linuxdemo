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
    0xFFffffff // Hottest
};

/* Expanded 3x5 font for the scroller & watermark */
const char* font3x5[32][5] = {
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
    {"#  ", "# #", "## ", "# #", "# #"}  // 31: k 
};

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
    if (ch_idx < 0 || ch_idx > 31) return;
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
    if (ch_idx < 0 || ch_idx > 31) return;
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

    int window_width = 2560;
    int window_height = 1440;
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return 1;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize: %s", Mix_GetError());
    }

    Mix_Music *bgm = Mix_LoadMUS("snd1.mp3");
    if (bgm) Mix_PlayMusic(bgm, -1);

    Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
    
    if (isFullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_ShowCursor(SDL_DISABLE);
    }

    SDL_Window* window = SDL_CreateWindow("Linux Demoscene - Warp & Fire (C/SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, windowFlags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_RenderSetLogicalSize(renderer, FIRE_WIDTH, FIRE_HEIGHT);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FIRE_WIDTH, FIRE_HEIGHT);

    for(int i=0; i<NUM_STARS; i++) {
        stars[i].x = (rand() % 2000) - 1000;
        stars[i].y = (rand() % 2000) - 1000;
        stars[i].z = (rand() % 255) + 1;
    }

    const char* scrollText = "//^-_ HELLO  DEMOSCENE  ***  WELCOME  TO  THE  LINUX  TERMINAL  ***  ENJOY  THIS  PARALLAX  STARFIELD  WITH  SPHERICAL  TEXT  SPIN  AND  PIXEL  FIRE  ***  GREETINGS  FROM  NORWAY  _-^";
    int time_counter = 0;
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        memset(frameBuffer, 0, sizeof(frameBuffer));

        /* 2. Warp-Speed Star Trails */
        float starSpeed = 3.0f;
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

        /* 3. Spherical Text Spin (FIXED JUMBLING) */
        float radius = 110.0f;
        for (int i = 0; scrollText[i] != '\0'; i++) {
            
            // New math: Time pushes characters forward. Index delays when they enter.
            float theta = (time_counter * 0.02f) - (i * 0.35f); 
            
            // CRITICAL FIX: Only draw characters if they are strictly between 0 and PI.
            // This prevents the string from wrapping around and overlapping itself on the screen.
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

        /* 4. Update and overlay Fire */
        for (int x = 0; x < FIRE_WIDTH; x++) firePixels[FIRE_HEIGHT - 1][x] = 36;
        updateFire();
        for (int y = 0; y < FIRE_HEIGHT; y++) {
            for (int x = 0; x < FIRE_WIDTH; x++) {
                int heat = firePixels[y][x];
                if (heat > 3) frameBuffer[y][x] = firePalette[heat];
            }
        }

        /* 4.5 Glowing Watermark (Bottom Right) */
        int wm_indices[] = {31, 30, 12};
        int wm_x = FIRE_WIDTH - 18;      
        int wm_y = FIRE_HEIGHT - 8;      
        
        for(int i=0; i<3; i++) {
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

        /* 5. Post-Processing: CRT Scanlines */
        for (int y = 0; y < FIRE_HEIGHT; y += 2) {
            for (int x = 0; x < FIRE_WIDTH; x++) {
                uint32_t c = frameBuffer[y][x];
                uint8_t r = ((c >> 16) & 0xFF) / 2;
                uint8_t g = ((c >> 8) & 0xFF) / 2;
                uint8_t b = (c & 0xFF) / 2;
                frameBuffer[y][x] = 0xFF000000 | (r<<16) | (g<<8) | b;
            }
        }

        /* 6. Push to GPU */
        SDL_UpdateTexture(texture, NULL, frameBuffer, FIRE_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL); 
        SDL_RenderPresent(renderer);

        time_counter++;
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