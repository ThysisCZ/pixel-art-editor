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
#define MAX_UNDO_STROKES 1000
#define MAX_STROKE_CELLS CELL_COUNT

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

typedef struct
{
    int cell_count;
    int cells[MAX_STROKE_CELLS];
    Color prev_colors[MAX_STROKE_CELLS];
} UndoStroke;

UndoStroke undo_strokes[MAX_UNDO_STROKES];
int undo_stroke_count = 0;

int drawing_stroke = 0;
int current_stroke_cell_count = 0;
int current_stroke_cells[MAX_STROKE_CELLS];
Color current_stroke_prev_colors[MAX_STROKE_CELLS];

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

int get_hovered_cell_index(void)
{
    Vector2 mouse = GetMousePosition();
    int mx = (int)mouse.x;
    int my = (int)mouse.y;

    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float cx = CELL_SIZE + col * CELL_SIZE;
        float cy = CELL_SIZE + row * CELL_SIZE;

        if (mx >= cx && mx < cx + CELL_SIZE &&
            my >= cy && my < cy + CELL_SIZE)
        {
            return i;
        }
    }

    return -1;
}

int already_in_current_stroke(int idx)
{
    for (int i = 0; i < current_stroke_cell_count; i++)
    {
        if (current_stroke_cells[i] == idx)
        {
            return 1;
        }
    }

    return 0;
}

void draw_pixel(Pixel *pixels)
{
    if (select_index == -1)
    {
        return;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        drawing_stroke = 1;
        current_stroke_cell_count = 0;
    }

    if (drawing_stroke && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        int idx = get_hovered_cell_index();

        if (idx != -1 && !already_in_current_stroke(idx))
        {
            current_stroke_cells[current_stroke_cell_count] = idx;
            current_stroke_prev_colors[current_stroke_cell_count] = pixels[idx].color;
            current_stroke_cell_count++;

            pixels[idx].color = current_color;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        if (current_stroke_cell_count > 0)
        {
            if (undo_stroke_count < MAX_UNDO_STROKES)
            {
                UndoStroke *s = &undo_strokes[undo_stroke_count++];
                s->cell_count = current_stroke_cell_count;

                for (int i = 0; i < current_stroke_cell_count; i++)
                {
                    s->cells[i] = current_stroke_cells[i];
                    s->prev_colors[i] = current_stroke_prev_colors[i];
                }
            }
        }

        drawing_stroke = 0;
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

void undo(Pixel *pixels)
{
    if (IsKeyPressed(KEY_Y) && undo_stroke_count > 0)
    {
        UndoStroke *s = &undo_strokes[--undo_stroke_count];

        for (int i = 0; i < s->cell_count; i++)
        {
            pixels[s->cells[i]].color = s->prev_colors[i];
        }
    }
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pixel Art Editor");

    Pixel pixels[CELL_COUNT];
    Pixel pixels_prev[CELL_COUNT];

    for (int i = 0; i < CELL_COUNT; i++)
    {
        int col = i % CANVAS_WIDTH;
        int row = i / CANVAS_WIDTH;

        float pos_x = CELL_SIZE + col * CELL_SIZE;
        float pos_y = CELL_SIZE + row * CELL_SIZE;

        pixels[i].pos_x = pos_x;
        pixels[i].pos_y = pos_y;
        pixels[i].color = WHITE;

        pixels_prev[i].pos_x = pos_x;
        pixels_prev[i].pos_y = pos_y;
        pixels_prev[i].color = WHITE;
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

        undo(&pixels[0]);

        EndDrawing();
    }

    return 0;
}