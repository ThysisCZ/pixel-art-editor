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

    for (int i = 0; i < COLOR_COUNT; i++)
    {
        int col = i % (COLOR_COUNT / 3);
        int row = i / (COLOR_COUNT / 3);
        int x_offset = COLOR_SIZE - CELL_SIZE / 2;
        int y_offset = COLOR_SIZE + CANVAS_HEIGHT * CELL_SIZE;

        float pos_x = x_offset + col * (COLOR_SIZE + CELL_SIZE);
        float pos_y = y_offset + row * (COLOR_SIZE + CELL_SIZE);

        DrawRectangle(pos_x, pos_y, COLOR_SIZE, COLOR_SIZE, colors[i]);
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

        EndDrawing();
    }

    return 0;
}