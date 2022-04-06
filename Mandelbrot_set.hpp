#ifndef MANDELBROT_HPP
#define MANDELBROT_HPP

#include <assert.h>
#include <stdio.h>
#include <immintrin.h>
#include <math.h>
#include <time.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

const int FPS_STR_MAX_SIZE      = 16;
const int FPS_STR_FPS_VALUE_POS = 5; 

const int WIDTH                 = 800;
const int HEIGHT                = 600;

const int MAX_ITERATION         = 256;

const int COLOR_INTENSITY       = 120;

const int PIXELS_AT_ONCE        = 8;

const float DX                  = 1 / 800.f;
const float DY                  = 1 / 600.f;

const float SINGLE_ZOOM         = 1.1f;
const float SINGLE_SHIFT        = 0.1f;

const float MAX_R2              = 100.f;

const unsigned BLACK            = 0xFF000000;

const __m256 PIXS_MULT          = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
const __m256 R2_MAX             = _mm256_set1_ps(MAX_R2);
const __m256 MASK_FOR_ITER      = _mm256_set1_ps(1);

struct picture
{
    sf::Texture texture; 
    sf::Sprite  sprite;
    float x_shift = -0.2;
    float y_shift = 0;
    float scale   = 3;
};

struct FPS
{
    sf::Font font;
    sf::Text text;

    clock_t cur_time  = clock();
    clock_t prev_time = 0;

    clock_t FPS_delay = CLOCKS_PER_SEC / 100;

    char fps_str[FPS_STR_MAX_SIZE] = "FPS: 000.0";
};

int  InitParameters(picture *set, FPS *fps);

void WindowKeyPressed(sf::RenderWindow *window, sf::Event *event, picture *set);

void CountMandelbrot(unsigned *pixels, picture *set);

void RenewFPS(FPS *fps_struct);

void SetPixel(unsigned char *pixels, int x_pos, int y_pos, int iter_quantity);

#endif