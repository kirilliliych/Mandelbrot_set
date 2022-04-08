#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

//#define PICTURE_SHOWING
#define FPS_CHECKING

const int FPS_STR_MAX_SIZE      = 16;
const int FPS_STR_FPS_VALUE_POS = 5; 
const float SINGLE_ZOOM         = 1.1f;
const float SINGLE_SHIFT        = 0.1f;

struct picture
{
    sf::Texture texture; 
    sf::Sprite  sprite;
    float x_shift = 0;
    float y_shift = 0;
    float scale   = 1;
};

struct FPS
{
    sf::Font font;
    sf::Text text;

    sf::Clock clock;
    sf::Time  cur_time   = clock.getElapsedTime();

    sf::Clock delay_clock;
    sf::Time  delay_time = delay_clock.getElapsedTime();

    float FPS_delay = 0.2f;

    double FPS_sum    = 0;
    int frames_saved  = 0;

    char fps_str[FPS_STR_MAX_SIZE] = "FPS: 0000.00\n";
};


const int WIDTH           = 800;
const int HEIGHT          = 600;

const float DX            = 1 / 800.f;
const float DY            = 1 / 600.f;

const int MAX_ITERATION   = 256;

const float MAX_R2        = 100.f;

const unsigned BLACK      = 0xFF000000;
const unsigned BLUE       = 0xFFFF0000;
const unsigned GREEN      = 0xFF00FF00;
const unsigned RED        = 0xFF0000FF;
const unsigned WHITE      = 0xFFFFFFFF;

void RenewFPS(FPS *fps_struct)
{
    assert(fps_struct != nullptr);
    
    fps_struct->delay_time   = fps_struct->delay_clock.getElapsedTime();
    fps_struct->cur_time     = fps_struct->clock.getElapsedTime();
    fps_struct->FPS_sum     += 1 / fps_struct->cur_time.asSeconds();
    ++fps_struct->frames_saved;

    if (fps_struct->delay_time.asSeconds() > fps_struct->FPS_delay)
    {
        sprintf(fps_struct->fps_str + FPS_STR_FPS_VALUE_POS, "%.2lf\n", 
                fps_struct->FPS_sum / fps_struct->frames_saved);
        
        fps_struct->text.setString(fps_struct->fps_str);
        
        fps_struct->delay_clock.restart();

        fps_struct->FPS_sum = 0;
        fps_struct->frames_saved = 0;

#ifdef FPS_CHECKING
        fprintf(stderr, "%s", fps_struct->fps_str); 
#endif
    }

    fps_struct->clock.restart();
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

            float orig_x_arr[8] = {orig_x, orig_x + DX, orig_x + 2 * DX, orig_x + 3 * DX, orig_x + 4 * DX, orig_x + 5 * DX, orig_x + 6 * DX, orig_x + 7 * DX};
            float orig_y_arr[8] = {orig_y, orig_y,      orig_y,          orig_y,          orig_y,          orig_y,          orig_y,          orig_y         };

            float cur_x_arr[8] = {}; for (int i = 0; i < 8; ++i) cur_x_arr[i] = orig_x_arr[i];
            float cur_y_arr[8] = {}; for (int i = 0; i < 8; ++i) cur_y_arr[i] = orig_y_arr[i];

            int iter_quantity[8] = {0, 0, 0, 0, 0, 0, 0, 0};

            for (int iter = 0; iter < MAX_ITERATION; ++iter)
            {
                float x2[8] = {}; for (int i = 0; i < 8; ++i) x2[i] = cur_x_arr[i] * cur_x_arr[i];
                float y2[8] = {}; for (int i = 0; i < 8; ++i) y2[i] = cur_y_arr[i] * cur_y_arr[i];
                float xy[8] = {}; for (int i = 0; i < 8; ++i) xy[i] = cur_x_arr[i] * cur_y_arr[i];

                float r2[8] = {}; for (int i = 0; i < 8; ++i) r2[i] = x2[i] + y2[i];

                int if_point_in[8] = {};
                for (int i = 0; i < 8; ++i) if (r2[i] < MAX_R2) if_point_in[i] = 1;

                int iter_mask = 0;
                for (int i = 0; i < 8; ++i) iter_mask |= (if_point_in[i] << i);
                if (!iter_mask) break;
                
                for (int i = 0; i < 8; ++i) iter_quantity[i] += if_point_in[i];

                for (int i = 0; i < 8; ++i) cur_x_arr[i] = x2[i] - y2[i] + orig_x_arr[i];
                for (int i = 0; i < 8; ++i) cur_y_arr[i] = xy[i] + xy[i] + orig_y_arr[i];
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

    picture set;
    set.texture.create(WIDTH, HEIGHT);
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

#ifdef FPS_CHECKING
    for (int i = 0; i < 100; ++i)
    {
        CountMandelbrot(pixels, &set);
        RenewFPS(&fps);
    }
#endif

#ifdef PICTURE_SHOWING
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot_set", sf::Style::Default);

    while (window.isOpen())
    {
        sf::Event event;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z))
        {
            set.scale /= SINGLE_ZOOM;
            printf("bigger\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
        {
            set.scale *= SINGLE_ZOOM;
            printf("smaller\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            set.x_shift += SINGLE_SHIFT;
            printf("left\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            set.x_shift -= SINGLE_SHIFT;
            printf("right\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            set.y_shift -= SINGLE_SHIFT;
            printf("down\n");
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            set.y_shift += SINGLE_SHIFT ;
            printf("up\n");
        }

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        CountMandelbrot(pixels, &set);
        set.texture.update((const uint8_t *) pixels);
        
        RenewFPS(&fps);
        
        window.clear();

        window.draw(set.sprite);
        
        window.draw(fps.text);

        window.display();

    }
#endif

    free(pixels);

    return 0;
}