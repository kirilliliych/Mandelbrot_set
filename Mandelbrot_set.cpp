#include <assert.h>
#include <stdio.h>
#include <immintrin.h>
#include <math.h>
#include <time.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

const int FPS_STR_MAX_SIZE      = 16;
const int FPS_STR_FPS_VALUE_POS = 5; 

struct picture
{
    sf::Image image;
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


const int WIDTH           = 800;
const int HEIGHT          = 600;

const float DX = 1 / 800.f;
const float DY = 1 / 600.f;

const int MAX_ITERATION   = 256;

const float MAX_R2        = 100.f;

const unsigned BLACK  = 0xFF000000;
const unsigned BLUE   = 0xFFFF0000;
const unsigned GREEN  = 0xFF00FF00;
const unsigned RED    = 0xFF0000FF;
const unsigned WHITE  = 0xFFFFFFFF;

const __m256 PIXS_MULT     = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
const __m256 R2_MAX        = _mm256_set1_ps(MAX_R2);
const __m256 MASK_FOR_ITER = _mm256_set1_ps(1);

void RenewFPS(FPS *fps_struct)
{
    assert(fps_struct != nullptr);

    fps_struct->cur_time = clock();
    if (fps_struct->cur_time - fps_struct->prev_time > fps_struct->FPS_delay)
    {
        sprintf(fps_struct->fps_str + FPS_STR_FPS_VALUE_POS, "%.1lf\n", 
                ((float) CLOCKS_PER_SEC / (fps_struct->cur_time - fps_struct->prev_time)));
        
        fps_struct->text.setString(fps_struct->fps_str);
    }
    fps_struct->prev_time = fps_struct->cur_time;
}

void SetPixel(unsigned *pixels, int x_pos, int y_pos, int iter_quantity)
{
    assert(pixels != nullptr);

    if (iter_quantity == MAX_ITERATION)
    {
        *(pixels + (y_pos * WIDTH + x_pos)) = BLACK;
    }
    else
    {
        *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 0) = 100 * (1 + sin(iter_quantity + 0));
        *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 1) = 100 * (1 + sin(iter_quantity + 2));
        *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 2) = 100 * (1 + sin(iter_quantity + 4));
        *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 3) = 0xFF;
    }
}

void CountMandelbrot(unsigned *pixels, picture *set)
{
    assert(pixels != nullptr);
    assert(set    != nullptr);

    for (int y_pos = 0; y_pos < HEIGHT; ++y_pos)
    {
        for (int x_pos = 0; x_pos < WIDTH; x_pos += 8 / set->scale)
        {
            float orig_x = (((float) x_pos - WIDTH  / 2) * DX + set->x_shift) * set->scale;
            float orig_y = (((float) y_pos - HEIGHT / 2) * DY + set->y_shift) * set->scale;

            __m256 orig_x_arr = _mm256_add_ps(_mm256_set1_ps(orig_x), _mm256_mul_ps(PIXS_MULT, _mm256_set1_ps(DX)));
            __m256 orig_y_arr = _mm256_set1_ps(orig_y);
            //float orig_x_arr[8] = {orig_x, orig_x + DX, orig_x + 2 * DX, orig_x + 3 * DX, orig_x + 4 * DX, orig_x + 5 * DX, orig_x + 6 * DX, orig_x + 7 * DX};
            //float orig_y_arr[8] = {orig_y, orig_y,      orig_y,          orig_y,          orig_y,          orig_y,          orig_y,          orig_y         };

            __m256 cur_x_arr  = orig_x_arr;
            __m256 cur_y_arr  = orig_y_arr;
            //float cur_x_arr[8] = {}; for (int i = 0; i < 8; ++i) cur_x_arr[i] = orig_x_arr[i];
            //float cur_y_arr[8] = {}; for (int i = 0; i < 8; ++i) cur_y_arr[i] = orig_y_arr[i];

            union
            {
                __m256i iter_nums = _mm256_setzero_si256();
                int iter_quantity[8];
            };
            
            //int iter_quantity[8] = {0, 0, 0, 0, 0, 0, 0, 0};

            for (int iter = 0; iter < MAX_ITERATION; ++iter)
            {   
                __m256 x2 = _mm256_mul_ps(cur_x_arr, cur_x_arr);
                __m256 y2 = _mm256_mul_ps(cur_y_arr, cur_y_arr);
                __m256 xy = _mm256_mul_ps(cur_x_arr, cur_y_arr);
                //float x2[8] = {}; for (int i = 0; i < 8; ++i) x2[i] = cur_x_arr[i] * cur_x_arr[i];
                //float y2[8] = {}; for (int i = 0; i < 8; ++i) y2[i] = cur_y_arr[i] * cur_y_arr[i];
                //float xy[8] = {}; for (int i = 0; i < 8; ++i) xy[i] = cur_x_arr[i] * cur_y_arr[i];

                __m256 r2 = _mm256_add_ps(x2, y2);
                //float r2[8] = {}; for (int i = 0; i < 8; ++i) r2[i] = x2[i] + y2[i];

                __m256 if_point_in = _mm256_cmp_ps(r2, R2_MAX, _CMP_LT_OS);
                //int if_point_in[8] = {};
                //for (int i = 0; i < 8; ++i) if (r2[i] < MAX_R2) if_point_in[i] = 1;

                if (!_mm256_movemask_ps(if_point_in))
                {
                    break;
                }
                //int iter_mask = 0;
                //for (int i = 0; i < 8; ++i) iter_mask |= (if_point_in[i] << i);
                //if (!iter_mask) break;
                
                iter_nums = _mm256_add_epi32(_mm256_cvtps_epi32(_mm256_and_ps(if_point_in, MASK_FOR_ITER)), iter_nums);
                //for (int i = 0; i < 8; ++i) iter_quantity[i] += if_point_in[i];

                cur_x_arr = _mm256_add_ps(_mm256_sub_ps(x2, y2), orig_x_arr);
                cur_y_arr = _mm256_add_ps(_mm256_add_ps(xy, xy), orig_y_arr);
                //for (int i = 0; i < 8; ++i) cur_x_arr[i] = x2[i] - y2[i] + orig_x_arr[i];
                //for (int i = 0; i < 8; ++i) cur_y_arr[i] = xy[i] + xy[i] + orig_y_arr[i];
            }

            for (int i = 0; i < 8; ++i)
            {
                SetPixel(pixels, x_pos + i, y_pos, iter_quantity[i]);
            }
        }
    }
}


int main()
{
    unsigned *pixels = (unsigned *) calloc(WIDTH * HEIGHT, sizeof(unsigned));

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot_set", sf::Style::Default);     
    
    picture set;
    set.image.create(WIDTH, HEIGHT, (const uint8_t *) pixels);
    if (!set.texture.loadFromImage(set.image))
    {
        printf("Error: can't load texture\n");
    }
    set.sprite.setTexture(set.texture);

    FPS fps;
    if (!fps.font.loadFromFile("CamingoMono-Regular.ttf"))
    {
        printf("Error: can't load font\n");
        
        return 1;
    }
    fps.text.setFont(fps.font);
    fps.text.setCharacterSize(30);
    fps.text.setFillColor(sf::Color::Green);

    while (window.isOpen())
    {
        sf::Event event;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        {
            set.scale -= 0.3;
            printf("bigger\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
        {
            set.scale += 0.3;
            printf("smaller\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            set.x_shift -= 0.1;
            printf("left\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            set.x_shift += 0.1;
            printf("right\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            set.y_shift -= 0.1;
            printf("down\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            set.y_shift += 0.1;
            printf("up\n");
        }

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        CountMandelbrot(pixels, &set);
        set.image.create(WIDTH, HEIGHT, (const uint8_t *) pixels);
        if (!set.texture.loadFromImage(set.image))
        {
            printf("Error: can't load texture");
        }
        
        RenewFPS(&fps);
        
        window.clear();

        window.draw(set.sprite);
        
        window.draw(fps.text);

        window.display();
    }

    free(pixels);

    return 0;
}