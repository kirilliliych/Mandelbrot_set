#include "Mandelbrot_set.hpp"

//#define PICTURE_SHOWING
#define FPS_CHECKING

int main()
{
    unsigned *pixels = (unsigned *) calloc(WIDTH * HEIGHT, sizeof(unsigned));
    picture set;
    FPS fps;
    InitParameters(&set, &fps);

#ifdef FPS_CHECKING
    for (int i = 0; i < 1000; ++i)
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

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                WindowKeyPressed(&window, &event, &set);
            }
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

int InitParameters(picture *set, FPS *fps)
{
    assert(set != nullptr);
    assert(fps != nullptr);

    set->texture.create(WIDTH, HEIGHT);
    set->sprite.setTexture(set->texture);
    
    if (!fps->font.loadFromFile("CamingoMono-Regular.ttf"))
    {
        printf("Error: can't load %s\n", "CamingoMono-Regular.ttf");
        
        return 1;
    }
    fps->text.setFont(fps->font);
    fps->text.setCharacterSize(30);
    fps->text.setFillColor(sf::Color::Green);

    return 0;
}

void WindowKeyPressed(sf::RenderWindow *window, sf::Event *event, picture *set)
{
    assert(window != nullptr);
    assert(event  != nullptr);
    assert(set    != nullptr);

    switch (event->key.code)
    {
        case sf::Keyboard::Escape:
            window->close();
            break;

        case sf::Keyboard::Z:
            set->scale /= SINGLE_ZOOM;
            break;

        case sf::Keyboard::X:
            set->scale *= SINGLE_ZOOM;
            break;

        case sf::Keyboard::Left:
        case sf::Keyboard::A:
            set->x_shift += SINGLE_SHIFT;
            break;

        case sf::Keyboard::Right:
        case sf::Keyboard::D:
            set->x_shift -= SINGLE_SHIFT;
            break;
        
        case sf::Keyboard::Down:
        case sf::Keyboard::S:
            set->y_shift -= SINGLE_SHIFT;
            break;
        
        case sf::Keyboard::Up:
        case sf::Keyboard::W:
            set->y_shift += SINGLE_SHIFT;
            break;

        default:
            break;
    }
}

void CountMandelbrot(unsigned *pixels, picture *set)
{
    assert(pixels != nullptr);
    assert(set    != nullptr);

    for (int y_pos = 0; y_pos < HEIGHT; ++y_pos)
    {
        for (int x_pos = 0; x_pos < WIDTH; x_pos += PIXELS_AT_ONCE / set->scale)
        {
            float orig_x = (((float) x_pos - WIDTH  / 2) * DX + set->x_shift) * set->scale;
            float orig_y = (((float) y_pos - HEIGHT / 2) * DY + set->y_shift) * set->scale;

            __m256 orig_x_arr = _mm256_add_ps(_mm256_mul_ps(PIXS_MULT, _mm256_set1_ps(DX)),
                                              _mm256_set1_ps(orig_x));
            __m256 orig_y_arr = _mm256_set1_ps(orig_y);

            __m256 cur_x_arr  = orig_x_arr;
            __m256 cur_y_arr  = orig_y_arr;

            union
            {
                __m256i iter_nums = _mm256_setzero_si256();
                int iter_quantity[PIXELS_AT_ONCE];
            };

            for (int iter = 0; iter < MAX_ITERATION; ++iter)
            {   
                __m256 x2 = _mm256_mul_ps(cur_x_arr, cur_x_arr);
                __m256 y2 = _mm256_mul_ps(cur_y_arr, cur_y_arr);
                __m256 xy = _mm256_mul_ps(cur_x_arr, cur_y_arr);

                __m256 r2 = _mm256_add_ps(x2, y2);

                __m256 if_point_in = _mm256_cmp_ps(r2, R2_MAX, _CMP_LT_OS);

                if (!_mm256_movemask_ps(if_point_in))
                {
                    break;
                }
                
                iter_nums = _mm256_add_epi32(_mm256_cvtps_epi32(_mm256_and_ps(if_point_in, MASK_FOR_ITER)), iter_nums);

                cur_x_arr = _mm256_add_ps(_mm256_sub_ps(x2, y2), orig_x_arr);
                cur_y_arr = _mm256_add_ps(_mm256_add_ps(xy, xy), orig_y_arr);
            }

            for (int i = 0; i < PIXELS_AT_ONCE; ++i)
            {
                SetPixel((unsigned char *) pixels, x_pos + i, y_pos, iter_quantity[i]);
            }
        }
    }
}

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

void SetPixel(unsigned char *pixels, int x_pos, int y_pos, int iter_quantity)
{
    assert(pixels != nullptr);

    if (iter_quantity == MAX_ITERATION)
    {
        *((unsigned int *) pixels + (y_pos * WIDTH + x_pos)) = BLACK;
    }
    else
    {
        *((pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 0) = COLOR_INTENSITY * (1 + sin(iter_quantity + 1));      // red
        *((pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 1) = COLOR_INTENSITY * (1 + sin(iter_quantity + 3));      // green
        *((pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 2) = COLOR_INTENSITY * (1 + sin(iter_quantity + 5));      // blue
        *((pixels) + (y_pos * WIDTH + x_pos) * sizeof(unsigned) + 3) = 0xFF;                                                // alpha
    }
}