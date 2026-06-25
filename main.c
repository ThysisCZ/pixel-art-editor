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
#define MAX_STROKES 1000
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
int select_index = -1; // index of current color

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
    Color next_colors[MAX_STROKE_CELLS];
} Stroke;

Stroke strokes[MAX_STROKES];
int stroke_count = 0;
int strokes_total = 0;

bool drawing_stroke = false;
int current_stroke_cell_count = 0;
int current_stroke_cells[MAX_STROKE_CELLS];
Color current_stroke_prev_colors[MAX_STROKE_CELLS];
Color current_stroke_next_colors[MAX_STROKE_CELLS];

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

        // Draw different selection border based on selected color
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
                // Handle color selection and deselection
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
    float mx = mouse.x;
    float my = mouse.y;

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

static int get_current_cell_index()
{
    Vector2 mouse = GetMousePosition();
    float mx = mouse.x;
    float my = mouse.y;

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

static bool already_in_current_stroke(int current_cell_index)
{
    for (int i = 0; i < current_stroke_cell_count; i++)
    {
        if (current_stroke_cells[i] == current_cell_index)
        {
            return true;
        }
    }

    return false;
}

void draw_pixel(Pixel *pixels)
{
    if (select_index != -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        drawing_stroke = true;
        current_stroke_cell_count = 0;
    }

    if (drawing_stroke && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        int current_cell_index = get_current_cell_index();

        if (current_cell_index != -1 && !already_in_current_stroke(current_cell_index))
        {
            // Store stroke cell indices and their colors before drawing
            current_stroke_cells[current_stroke_cell_count] = current_cell_index;
            current_stroke_prev_colors[current_stroke_cell_count] = pixels[current_cell_index].color;
            current_stroke_next_colors[current_stroke_cell_count] = current_color;
            current_stroke_cell_count++;

            pixels[current_cell_index].color = current_color;
        }
    }

    // Save information about the drawn stroke
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        if (current_stroke_cell_count > 0)
        {
            stroke_count++;
            strokes_total++;

            Stroke *undo_stroke = &strokes[--stroke_count];
            undo_stroke->cell_count = current_stroke_cell_count;

            Stroke *redo_stroke = &strokes[stroke_count++];
            redo_stroke->cell_count = current_stroke_cell_count;

            for (int i = 0; i < current_stroke_cell_count; i++)
            {
                undo_stroke->cells[i] = current_stroke_cells[i];
                undo_stroke->prev_colors[i] = current_stroke_prev_colors[i];

                redo_stroke->cells[i] = current_stroke_cells[i];
                redo_stroke->next_colors[i] = current_stroke_next_colors[i];
            }
        }

        drawing_stroke = false;
    }
}

void erase_pixel(Pixel *pixels)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        drawing_stroke = true;
        current_stroke_cell_count = 0;
    }

    if (drawing_stroke && IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        int current_cell_index = get_current_cell_index();

        if (current_cell_index != -1 && !already_in_current_stroke(current_cell_index))
        {
            current_stroke_cells[current_stroke_cell_count] = current_cell_index;
            current_stroke_prev_colors[current_stroke_cell_count] = pixels[current_cell_index].color;
            current_stroke_next_colors[current_stroke_cell_count] = WHITE;
            current_stroke_cell_count++;

            pixels[current_cell_index].color = WHITE;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
    {
        if (current_stroke_cell_count > 0)
        {
            stroke_count++;
            strokes_total++;

            Stroke *undo_stroke = &strokes[--stroke_count];
            undo_stroke->cell_count = current_stroke_cell_count;

            Stroke *redo_stroke = &strokes[stroke_count++];
            redo_stroke->cell_count = current_stroke_cell_count;

            for (int i = 0; i < current_stroke_cell_count; i++)
            {
                undo_stroke->cells[i] = current_stroke_cells[i];
                undo_stroke->prev_colors[i] = current_stroke_prev_colors[i];

                redo_stroke->cells[i] = current_stroke_cells[i];
                redo_stroke->next_colors[i] = current_stroke_next_colors[i];
            }
        }

        drawing_stroke = false;
    }
}

void undo(Pixel *pixels)
{
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Y) && stroke_count > 0)
    {
        Stroke *stroke = &strokes[--stroke_count];

        for (int i = 0; i < stroke->cell_count; i++)
        {
            pixels[stroke->cells[i]].color = stroke->prev_colors[i];
        }
    }
}

void redo(Pixel *pixels)
{
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z) && stroke_count < strokes_total)
    {
        Stroke *stroke = &strokes[stroke_count++];

        for (int i = 0; i < stroke->cell_count; i++)
        {
            pixels[stroke->cells[i]].color = stroke->next_colors[i];
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
        redo(&pixels[0]);

        EndDrawing();
    }

    return 0;
}