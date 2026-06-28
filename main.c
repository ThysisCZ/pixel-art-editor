#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 900
#define CELL_SIZE 25
#define CANVAS_WIDTH 30
#define CANVAS_HEIGHT 20
#define CELL_COUNT (CANVAS_WIDTH * CANVAS_HEIGHT)
#define COLOR_COUNT 21
#define COLOR_SIZE 75
#define MAX_STROKES 1000
#define MAX_DEAD_ENDS (12 * CELL_COUNT)

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
Color selected_color;
int select_index = -1; // index of selected color

typedef struct
{
    float pos_x;
    float pos_y;
    Color color;
} Pixel;

typedef struct
{
    int cell_count;
    int cells[CELL_COUNT];
    Color prev_colors[CELL_COUNT];
    Color next_colors[CELL_COUNT];
} Stroke;

Stroke strokes[MAX_STROKES];
int stroke_count = 0;
int strokes_total = 0;

bool drawing_stroke = false;
int current_stroke_cell_count = 0;
int current_stroke_cells[CELL_COUNT];
Color current_stroke_prev_colors[CELL_COUNT];
Color current_stroke_next_colors[CELL_COUNT];

bool fill_mode = false;
bool fill_active = false;
Color fill_source_color = WHITE;
int fill_current_index = -1;
int dead_end_count = 0;
int current_fill_cell_count = 0;
int current_fill_cells[CELL_COUNT];

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
            DrawRectangle(pos_x + CELL_SIZE / 4, pos_y + CELL_SIZE / 4, select_size, select_size, selected_color);
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
                    selected_color = colors[i];
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
            if (!fill_mode)
            {
                if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
                {
                    DrawRectangle(cx, cy, CELL_SIZE, CELL_SIZE, WHITE);
                    DrawRectangleLines(cx, cy, CELL_SIZE, CELL_SIZE, WHITE);
                }
                else
                {
                    if (select_index != -1)
                    {
                        DrawRectangle(cx, cy, CELL_SIZE, CELL_SIZE, selected_color);
                        DrawRectangleLines(cx, cy, CELL_SIZE, CELL_SIZE, WHITE);
                    }
                }
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
            current_stroke_next_colors[current_stroke_cell_count] = selected_color;
            current_stroke_cell_count++;

            pixels[current_cell_index].color = selected_color;
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

static bool colors_equal(Color c1, Color c2)
{
    return (c1.r == c2.r &&
            c1.g == c2.g &&
            c1.b == c2.b);
}

static int generate_next_index(int current_index)
{
    int right_index = current_index + 1;
    int bottom_index = current_index + CANVAS_WIDTH;
    int left_index = current_index - 1;
    int top_index = current_index - CANVAS_WIDTH;

    int random_index = GetRandomValue(1, 4);
    int next_index = current_index;

    switch (random_index)
    {
    case 1:
        if (right_index < CELL_COUNT)
        {
            next_index = right_index;
            break;
        }
    case 2:
        if (bottom_index < CELL_COUNT)
        {
            next_index = bottom_index;
            break;
        }
    case 3:
        if (left_index >= 0)
        {
            next_index = left_index;
            break;
        }
    case 4:
        if (top_index >= 0)
        {
            next_index = top_index;
            break;
        }
    }

    return next_index;
}

static bool already_in_current_fill(int current_cell_index)
{
    for (int i = 0; i < CELL_COUNT; i++)
    {
        if (current_fill_cells[i] == current_cell_index)
        {
            return true;
        }
    }

    return false;
}

void fill(Pixel *pixels)
{
    int start_index = get_current_cell_index();

    float pos_x = pixels[start_index].pos_x;
    float pos_y = pixels[start_index].pos_y;

    if (IsKeyPressed(KEY_B))
    {
        if (!fill_mode)
        {
            fill_mode = true;
        }
        else
        {
            fill_mode = false;
        }
    }

    if (fill_mode && start_index != -1)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            fill_source_color = pixels[start_index].color;
            fill_active = true;

            fill_current_index = start_index;
            dead_end_count = 0;
            current_fill_cell_count = 0;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            fill_source_color = pixels[start_index].color;
            fill_active = true;

            fill_current_index = start_index;
            dead_end_count = 0;
            current_fill_cell_count = 0;
        }

        if ((IsMouseButtonReleased(MOUSE_BUTTON_LEFT) ||
             (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))) &&
            fill_active)
        {
            fill_source_color = WHITE;
            fill_active = false;

            fill_current_index = -1;
            dead_end_count = 0;
        }

        if (select_index != -1)
        {
            // Fill with palette colors
            if (fill_active && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !colors_equal(fill_source_color, selected_color))
            {
                while (dead_end_count < MAX_DEAD_ENDS)
                {
                    int current_index = fill_current_index;
                    int next_index = generate_next_index(current_index);

                    int right_index = current_index + 1;
                    int bottom_index = current_index + CANVAS_WIDTH;
                    int left_index = current_index - 1;
                    int top_index = current_index - CANVAS_WIDTH;

                    if (colors_equal(pixels[current_index].color, fill_source_color))
                    {
                        pixels[current_index].color = selected_color;
                        current_fill_cells[current_fill_cell_count] = current_index;
                        current_fill_cell_count++;
                    }

                    if (colors_equal(pixels[next_index].color, fill_source_color))
                    {
                        current_index = next_index;
                        dead_end_count = 0;
                    }

                    // Handle dead end
                    if (!colors_equal(pixels[right_index].color, fill_source_color) &&
                        !colors_equal(pixels[bottom_index].color, fill_source_color) &&
                        !colors_equal(pixels[left_index].color, fill_source_color) &&
                        !colors_equal(pixels[top_index].color, fill_source_color))
                    {
                        next_index = generate_next_index(current_index);

                        if (colors_equal(pixels[next_index].color, selected_color))
                        {
                            // Stop on outer area with selected color
                            if (!already_in_current_fill(next_index))
                            {
                                pixels[next_index].color = selected_color;
                                dead_end_count = MAX_DEAD_ENDS;
                            }
                            else
                            {
                                current_index = next_index;
                            }
                        }

                        dead_end_count++;
                    }

                    fill_current_index = current_index;
                }
            }
        }

        // Erase multiple cells at once
        if (fill_active && IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !colors_equal(fill_source_color, WHITE))
        {
            while (dead_end_count < MAX_DEAD_ENDS)
            {
                int current_index = fill_current_index;
                int next_index = generate_next_index(current_index);

                int right_index = current_index + 1;
                int bottom_index = current_index + CANVAS_WIDTH;
                int left_index = current_index - 1;
                int top_index = current_index - CANVAS_WIDTH;

                if (colors_equal(pixels[current_index].color, fill_source_color))
                {
                    pixels[current_index].color = WHITE;
                    current_fill_cells[current_fill_cell_count] = current_index;
                    current_fill_cell_count++;
                }

                if (colors_equal(pixels[next_index].color, fill_source_color))
                {
                    current_index = next_index;
                    dead_end_count = 0;
                }

                // Handle dead end
                if (!colors_equal(pixels[right_index].color, fill_source_color) &&
                    !colors_equal(pixels[bottom_index].color, fill_source_color) &&
                    !colors_equal(pixels[left_index].color, fill_source_color) &&
                    !colors_equal(pixels[top_index].color, fill_source_color))
                {
                    next_index = generate_next_index(current_index);

                    if (colors_equal(pixels[next_index].color, WHITE))
                    {
                        if (!already_in_current_fill(next_index))
                        {
                            pixels[next_index].color = WHITE;
                            dead_end_count = MAX_DEAD_ENDS;
                        }
                        else
                        {
                            current_index = next_index;
                        }
                    }

                    dead_end_count++;
                }

                fill_current_index = current_index;
            }
        }

        Color fill_cursor_color = selected_color;

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            fill_cursor_color = WHITE;
        }

        if (select_index != -1 || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            DrawRectangle(pos_x, pos_y, CELL_SIZE, CELL_SIZE, BLACK);
            DrawRectangle(pos_x + CELL_SIZE / 4, pos_y + CELL_SIZE / 4, CELL_SIZE / 2, CELL_SIZE / 2, fill_cursor_color);
            DrawRectangleLines(pos_x, pos_y, CELL_SIZE, CELL_SIZE, WHITE);
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

        if (!fill_mode)
        {
            draw_pixel(&pixels[0]);
            erase_pixel(&pixels[0]);
        }

        undo(&pixels[0]);
        redo(&pixels[0]);

        fill(&pixels[0]);

        EndDrawing();
    }

    return 0;
}