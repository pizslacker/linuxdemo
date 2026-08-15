#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h> // NEW: SDL_mixer header
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

/* Internal Render Resolution */
#define FIRE_WIDTH 512
#define FIRE_HEIGHT 300

/* Display Window Resolution */
#define WINDOW_WIDTH 2560
#define WINDOW_HEIGHT 1440

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

/* Tiny 3x5 font for the scroller */
const char* font3x5[30][5] = {
    {"###", "# #", "###", "# #", "# #"}, // A
    {"## ", "# #", "## ", "# #", "## "}, // B
    {" ##", "#  ", "#  ", "#  ", " ##"}, // C
    {"## ", "# #", "# #", "# #", "## "}, // D
    {"###", "#  ", "## ", "#  ", "###"}, // E
    {"###", "#  ", "## ", "#  ", "#  "}, // F
    {" ##", "#  ", "# #", "# #", " ##"}, // G
    {"# #", "# #", "###", "# #", "# #"}, // H
    {"###", " # ", " # ", " # ", "###"}, // I
    {"  #", "  #", "  #", "# #", " # "}, // J
    {"# #", "# #", "## ", "# #", "# #"}, // K
    {"#  ", "#  ", "#  ", "#  ", "###"}, // L
    {"# #", "###", "###", "# #", "# #"}, // M 
    {"###", "# #", "# #", "# #", "# #"}, // N 
    {"###", "# #", "# #", "# #", "###"}, // O
    {"###", "# #", "###", "#  ", "#  "}, // P
    {"###", "# #", "# #", "###", "  #"}, // Q
    {"###", "# #", "## ", "# #", "# #"}, // R
    {" ##", "#  ", " # ", "  #", "## "}, // S
    {"###", " # ", " # ", " # ", " # "}, // T
    {"# #", "# #", "# #", "# #", "###"}, // U
    {"# #", "# #", "# #", "# #", " # "}, // V
    {"# #", "# #", "###", "###", "# #"}, // W 
    {"# #", "# #", " # ", "# #", "# #"}, // X
    {"# #", "# #", " # ", " # ", " # "}, // Y
    {"###", "  #", " # ", "#  ", "###"}, // Z
    {"   ", "   ", "   ", "   ", "   "}, // 26: Space
    {"   ", "   ", "   ", "   ", " # "}, // 27: .
    {"   ", "   ", "###", "   ", "   "}, // 28: -
    {" # ", "###", "# #", "###", " # "}, // 29: *
};

uint8_t firePixels[FIRE_HEIGHT][FIRE_WIDTH];
uint32_t frameBuffer[FIRE_HEIGHT][FIRE_WIDTH];
struct Star { float x, y, z; } stars[NUM_STARS];

/* Bresenham's Line Algorithm for Star Trails */
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
    if (ch_idx < 0 || ch_idx > 29) return;
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

    // Parse command line arguments
    bool isFullscreen = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0) {
            isFullscreen = true;
        }
    }

    // NEW: Initialize both Video and Audio subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("Could not initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // NEW: Open audio device (44.1kHz, standard format, stereo, 2048 byte chunk size)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize: %s", Mix_GetError());
        // Continuing anyway so the graphics still run without audio
    }

    // NEW: Load the WAV file as background music
    Mix_Music *bgm = Mix_LoadMUS("snd1.mp3");
    if (bgm) {
        Mix_PlayMusic(bgm, -1); // -1 means loop infinitely
    } else {
        SDL_Log("Could not load sdn1.mp3: %s", Mix_GetError());
    }

    Uint32 windowFlags = 0;
    if (isFullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_ShowCursor(SDL_DISABLE);
    }

    SDL_Window* window = SDL_CreateWindow("Linux Demoscene - Warp & Fire", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, windowFlags);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    SDL_RenderSetLogicalSize(renderer, FIRE_WIDTH, FIRE_HEIGHT);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FIRE_WIDTH, FIRE_HEIGHT);

    for(int i=0; i<NUM_STARS; i++) {
        stars[i].x = (rand() % 2000) - 1000;
        stars[i].y = (rand() % 2000) - 1000;
        stars[i].z = (rand() % 255) + 1;
    }

    const char* scrollText = "HELLO DEMOSCENE * WELCOME TO THE LINUX TERMINAL * ENJOY THIS PARALLAX STARFIELD WITH SPHERICAL TEXT SPIN AND PIXEL FIRE * GREETINGS FROM NORWAY *       ";
    int time_counter = 0;
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
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
            if (i % 3 == 0) { 
                b = (intensity + 50 > 255) ? 255 : intensity + 50;
            } else if (i % 4 == 0) { 
                r = (intensity + 50 > 255) ? 255 : intensity + 50; 
                g = (intensity + 20 > 255) ? 255 : intensity + 20;
            }
            
            uint32_t color = 0xFF000000 | (r<<16) | (g<<8) | b;
            drawLine(prev_px, prev_py, px, py, color);
        }

        /* 3. Spherical Text Spin */
        float radius = 110.0f;
        for (int i = 0; scrollText[i] != '\0'; i++) {
            // Updated character spacing here
            float theta = (i * 0.20f) - (time_counter * 0.015f); 
            
            float z = sin(theta);
            if (z < 0) continue; 
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
            uint32_t color = 0xFF000000 | (0<<16) | (depthColor<<8) | 255; 

            drawChar(char_idx, cx, cy, scale, color);
        }

        /* 4. Update and overlay Fire */
        for (int x = 0; x < FIRE_WIDTH; x++) {
            firePixels[FIRE_HEIGHT - 1][x] = 36;
        }
        updateFire();
        for (int y = 0; y < FIRE_HEIGHT; y++) {
            for (int x = 0; x < FIRE_WIDTH; x++) {
                int heat = firePixels[y][x];
                if (heat > 3) {
                    frameBuffer[y][x] = firePalette[heat];
                }
            }
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

    if (isFullscreen) {
        SDL_ShowCursor(SDL_ENABLE); 
    }

    // NEW: Cleanup audio resources
    if (bgm) {
        Mix_FreeMusic(bgm);
    }
    Mix_CloseAudio();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}