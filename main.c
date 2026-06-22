#include <stdio.h>
#include <raylib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 900
#define CELL_SIZE 25
#define CANVAS_WIDTH 30
#define CANVAS_HEIGHT 20
#define CELL_COUNT CANVAS_WIDTH *CANVAS_HEIGHT
#define COLOR_COUNT 21
#define COLOR_SIZE 75

Color colors[COLOR_COUNT] = {DARKGRAY,
                             MAROON,
                             ORANGE,
                             DARKGREEN,
                             DARKBLUE,
                             DARKPURPLE,
                             DARKBROWN,
                             GRAY,
                             RED,
                             GOLD,
                             LIME,
                             BLUE,
                             VIOLET,
                             BROWN,
                             LIGHTGRAY,
                             PINK,
                             YELLOW,
                             GREEN,
                             SKYBLUE,
                             PURPLE,
                             BEIGE};
Color current_color;
int select_index = -1;

typedef struct
{
    float pos_x;
    float pos_y;
    Color color;
} Pixel;

void draw_canvas(Pixel *pixels)
{
    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float pos_x = CELL_SIZE + col * CELL_SIZE;
        float pos_y = CELL_SIZE + row * CELL_SIZE;

        DrawRectangle(pos_x, pos_y, CELL_SIZE, CELL_SIZE, pixels[i].color);
        DrawRectangleLines(pos_x, pos_y, CELL_SIZE, CELL_SIZE, BLACK);
    }
}

void draw_palette()
{
    for (int i = 0; i < COLOR_COUNT; i++)
    {
        int col = i % (COLOR_COUNT / 3);
        int row = i / (COLOR_COUNT / 3);
        int x_offset = COLOR_SIZE - CELL_SIZE / 2;
        int y_offset = COLOR_SIZE + CANVAS_HEIGHT * CELL_SIZE;
        int select_size = COLOR_SIZE - CELL_SIZE / 2;

        float pos_x = x_offset + col * (COLOR_SIZE + CELL_SIZE);
        float pos_y = y_offset + row * (COLOR_SIZE + CELL_SIZE);

        if (i == select_index)
        {
            DrawRectangle(pos_x, pos_y, COLOR_SIZE, COLOR_SIZE, BLACK);
            DrawRectangle(pos_x + CELL_SIZE / 4, pos_y + CELL_SIZE / 4, select_size, select_size, current_color);
        }
        else
        {
            DrawRectangle(pos_x, pos_y, COLOR_SIZE, COLOR_SIZE, colors[i]);
        }
    }
}

void color_hover()
{
    Vector2 mouse = GetMousePosition();
    float mx = mouse.x;
    float my = mouse.y;

    for (int i = 0; i < COLOR_COUNT; i++)
    {
        int col = i % (COLOR_COUNT / 3);
        int row = i / (COLOR_COUNT / 3);
        int x_offset = COLOR_SIZE - CELL_SIZE / 2;
        int y_offset = COLOR_SIZE + CANVAS_HEIGHT * CELL_SIZE;
        int select_size = COLOR_SIZE - CELL_SIZE / 2;

        float cx = x_offset + col * (COLOR_SIZE + CELL_SIZE);
        float cy = y_offset + row * (COLOR_SIZE + CELL_SIZE);

        if (mx >= cx && mx <= cx + COLOR_SIZE &&
            my >= cy && my <= cy + COLOR_SIZE)
        {
            if (i == select_index)
            {
                DrawRectangleLines(cx + CELL_SIZE / 4, cy + CELL_SIZE / 4, select_size, select_size, WHITE);
            }
            else
            {
                DrawRectangleLines(cx, cy, COLOR_SIZE, COLOR_SIZE, WHITE);
            }
        }
    }
}

void pick_color()
{
    Vector2 mouse = GetMousePosition();
    float mx = mouse.x;
    float my = mouse.y;

    for (int i = 0; i < COLOR_COUNT; i++)
    {
        int col = i % (COLOR_COUNT / 3);
        int row = i / (COLOR_COUNT / 3);
        int x_offset = COLOR_SIZE - CELL_SIZE / 2;
        int y_offset = COLOR_SIZE + CANVAS_HEIGHT * CELL_SIZE;

        float cx = x_offset + col * (COLOR_SIZE + CELL_SIZE);
        float cy = y_offset + row * (COLOR_SIZE + CELL_SIZE);

        if (mx >= cx && mx <= cx + COLOR_SIZE &&
            my >= cy && my <= cy + COLOR_SIZE)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                if (i != select_index)
                {
                    current_color = colors[i];
                    select_index = i;
                }
                else
                {
                    select_index = -1;
                }
            }
        }
    }
}

void cell_hover()
{
    Vector2 mouse = GetMousePosition();
    int mx = mouse.x;
    int my = mouse.y;

    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float cx = CELL_SIZE + col * CELL_SIZE;
        float cy = CELL_SIZE + row * CELL_SIZE;

        if (mx >= cx && mx < cx + CELL_SIZE &&
            my >= cy && my < cy + CELL_SIZE)
        {
            if (select_index != -1)
            {
                DrawRectangle(cx, cy, CELL_SIZE, CELL_SIZE, current_color);
                DrawRectangleLines(cx, cy, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }
    }
}

void draw_pixel(Pixel *pixels)
{
    Vector2 mouse = GetMousePosition();
    int mx = mouse.x;
    int my = mouse.y;

    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float cx = CELL_SIZE + col * CELL_SIZE;
        float cy = CELL_SIZE + row * CELL_SIZE;

        if (mx >= cx && mx < cx + CELL_SIZE &&
            my >= cy && my < cy + CELL_SIZE)
        {
            if (select_index != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                pixels[i].color = current_color;
            }
        }
    }
}

void erase_pixel(Pixel *pixels)
{
    Vector2 mouse = GetMousePosition();
    int mx = mouse.x;
    int my = mouse.y;

    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float cx = CELL_SIZE + col * CELL_SIZE;
        float cy = CELL_SIZE + row * CELL_SIZE;

        if (mx >= cx && mx < cx + CELL_SIZE &&
            my >= cy && my < cy + CELL_SIZE)
        {
            if (select_index != -1 && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
            {
                pixels[i].color = WHITE;
                DrawRectangle(cx, cy, CELL_SIZE, CELL_SIZE, WHITE);
                DrawRectangleLines(cx, cy, CELL_SIZE, CELL_SIZE, WHITE);
            }
        }
    }
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pixel Art Editor");

    Pixel pixels[CELL_COUNT];

    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float pos_x = CELL_SIZE + col * CELL_SIZE;
        float pos_y = CELL_SIZE + row * CELL_SIZE;

        pixels[i].pos_x = pos_x;
        pixels[i].pos_y = pos_y;
        pixels[i].color = WHITE;
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();

        draw_canvas(&pixels[0]);
        draw_palette();

        color_hover();
        pick_color();

        cell_hover();
        draw_pixel(&pixels[0]);
        erase_pixel(&pixels[0]);

        EndDrawing();
    }

    return 0;
}