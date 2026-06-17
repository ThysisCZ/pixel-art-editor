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

void draw_canvas()
{
    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float pos_x = CELL_SIZE + col * CELL_SIZE;
        float pos_y = CELL_SIZE + row * CELL_SIZE;

        DrawRectangle(pos_x, pos_y, CELL_SIZE, CELL_SIZE, WHITE);
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
        int select_size = COLOR_SIZE - CELL_SIZE / 2;

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

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pixel Art Editor");

    while (!WindowShouldClose())
    {
        BeginDrawing();

        draw_canvas();
        draw_palette();

        pick_color();

        EndDrawing();
    }

    return 0;
}