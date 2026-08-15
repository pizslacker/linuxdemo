#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

/* Internal Render Resolution */
#define FIRE_WIDTH 512
#define FIRE_HEIGHT 300

/* Display Window Resolution */
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define NUM_STARS 500

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

/* Tiny 3x5 font for the scroller (A-Z, Space, ., -, *) */
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

/* 3D Star structure */
struct Star { float x, y, z; } stars[NUM_STARS];

/* Draws a scaled character from our font array */
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

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    SDL_Window* window = SDL_CreateWindow("Linux Demoscene - Parallax & Fire (C/SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, FIRE_WIDTH, FIRE_HEIGHT);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, FIRE_WIDTH, FIRE_HEIGHT);

    // Seed initial stars
    for(int i=0; i<NUM_STARS; i++) {
        stars[i].x = (rand() % 4000) - 1000;
        stars[i].y = (rand() % 4000) - 1000;
        stars[i].z = (rand() % 255) + 1;
    }

    const char* scrollText = "HELLO DEMOSCENE! * # WELCOME TO THE LINUX TERMINAL # * ENJOY THIS PARALLAX STARFIELD WITH SPHERICAL TEXT SPIN AND PIXEL FIRE * GREETINGS FROM NORWAY *       ";
    int time_counter = 0;
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        /* 1. Clear Screen */
        memset(frameBuffer, 0, sizeof(frameBuffer));

        /* 2. Parallax 3D Stars */
        for(int i=0; i<NUM_STARS; i++) {
            stars[i].z -= 2.0f; // Fly forward
            if (stars[i].z <= 1.0f) {
                stars[i].z = 255.0f;
                stars[i].x = (rand() % 2000) - 1000;
                stars[i].y = (rand() % 2000) - 1000;
            }
            // 3D to 2D Projection
            int px = (int)((stars[i].x / stars[i].z) * 120 + (FIRE_WIDTH / 2));
            int py = (int)((stars[i].y / stars[i].z) * 120 + (FIRE_HEIGHT / 2));
            
            if (px >= 0 && px < FIRE_WIDTH && py >= 0 && py < FIRE_HEIGHT) {
                uint8_t intensity = (uint8_t)(255 - stars[i].z); // Dimmer further away
                frameBuffer[py][px] = 0xFF000000 | (intensity<<16) | (intensity<<8) | intensity;
            }
        }

        /* 3. Spherical Text Spin */
        float radius = 110.0f;
        for (int i = 0; scrollText[i] != '\0'; i++) {
            // Map character index and time to an angle on a circle
            float theta = (i * 0.20f) - (time_counter * 0.015f);
            
            float z = sin(theta);
            if (z < 0) continue; // Behind the sphere, cull it
            float x = cos(theta);
            
            // Project mapped 3D coordinates back to 2D screen space
            float scale = 0.5f + (z * 1.5f); 
            float cx = (FIRE_WIDTH / 2.0f) + (x * radius) - (scale * 4.0f); 
            
            // Add a sine wave on the Y axis to give the cylinder a wavy/bouncy deformation
            float cy = (FIRE_HEIGHT / 3.0f) + (sin(time_counter * 0.02f + x * 2.0f) * 40.0f);

            int char_idx = 26; // Default to Space
            char c = scrollText[i];
            if (c >= 'A' && c <= 'Z') char_idx = c - 'A';
            else if (c == '.') char_idx = 27;
            else if (c == '-') char_idx = 28;
            else if (c == '*') char_idx = 29;

            // Give the text an electric cyan glow based on depth
            uint8_t depthColor = (uint8_t)(50 + z * 205);
            uint32_t color = 0xFF000000 | (0<<16) | (depthColor<<8) | 255; 

            drawChar(char_idx, cx, cy, scale, color);
        }

        /* 4. Update and overlay Fire */
        // Keep the bottom row burning
        for (int x = 0; x < FIRE_WIDTH; x++) {
            firePixels[FIRE_HEIGHT - 1][x] = 36;
        }
        updateFire();
        for (int y = 0; y < FIRE_HEIGHT; y++) {
            for (int x = 0; x < FIRE_WIDTH; x++) {
                int heat = firePixels[y][x];
                // Only draw fire if it's hot enough, letting the background peek through the smoke
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

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}