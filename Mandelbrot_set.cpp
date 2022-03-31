#include <assert.h>
#include <stdio.h>
#include <math.h>
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
    float scale = 3;
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

struct complex_number
{
    float real_part = 0;
    float imag_part = 0;
};

const int HEIGHT          = 600;
const int WIDTH           = 800;

const int MAX_ITERATION   = 256;

const double MAX_R2       = 10;

const unsigned int BLACK  = 0xFF000000;
const unsigned int BLUE   = 0xFFFF0000;
const unsigned int GREEN  = 0xFF00FF00;
const unsigned int RED    = 0xFF0000FF;
const unsigned int WHITE  = 0xFFFFFFFF;

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

void CountMandelbrot(unsigned int *pixels, picture *set)
{
    for (int y_pos = 0; y_pos < HEIGHT; ++y_pos)
    {
        for (int x_pos = 0; x_pos < WIDTH; ++x_pos)
        {
            complex_number original_number = {(((float) x_pos - WIDTH / 2) / WIDTH + set->x_shift) * set->scale, (((float) y_pos - HEIGHT / 2) / HEIGHT + set->y_shift) * set->scale};
            complex_number moving_number = original_number;

            size_t iteration = 0;
            float r2 = 0;

            for (iteration; (iteration < MAX_ITERATION) && (r2 < MAX_R2); ++iteration)
            {
                r2 = moving_number.real_part * moving_number.real_part + moving_number.imag_part * moving_number.imag_part;
            
                float new_real_part = moving_number.real_part * moving_number.real_part  - 
                                      moving_number.imag_part * moving_number.imag_part  + original_number.real_part;

                float new_img_part  = 2 * moving_number.real_part * moving_number.imag_part  + original_number.imag_part;

                moving_number.real_part = new_real_part;
                moving_number.imag_part = new_img_part;
            }
            if (iteration == MAX_ITERATION)
            {
                *(pixels + (y_pos * WIDTH + x_pos)) = BLACK;
            }
            else
            {
                /**(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int))     = 7 * iteration;
                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int) + 1) = 128 + 15 * iteration;
                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int) + 2) = 255 - 3 * iteration;
                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int) + 3) = 0xFF;*/

                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int))     = 100 * (1 + sin(iteration + 0));
                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int) + 1) = 100 * (1 + sin(iteration + 2));
                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int) + 2) = 100 * (1 + sin(iteration + 4));
                *(((unsigned char *) pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned int) + 3) = 0xFF;
            }
        }
    }
}


int main()
{
    unsigned int *pixels = (unsigned int *) calloc(WIDTH * HEIGHT, sizeof(unsigned int));

    picture set;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot_set", sf::Style::Default);     
    
    set.image.create(WIDTH, HEIGHT, (const uint8_t *) pixels);
    if (!set.texture.loadFromImage(set.image))
    {
        printf("Error: can't load texture");
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
