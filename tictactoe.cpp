#include <stdio.h>
#include <stdint.h>
#include <raylib.h>
#include <raymath.h>
#include <assert.h>

#define H 3
#define W 3

#define INNER_SIZE 100

enum Placement {
    PLACEMENT_CLEAR,
    PLACEMENT_X,
    PLACEMENT_O,
};

Placement global_current_placement = PLACEMENT_X;
Placement global_winner = PLACEMENT_CLEAR;

struct Grid {
    Placement array[H][W];
    Vector2   pos;
    float     size;
    int       select_x, select_y;

    Placement won;
    Vector2   win_line_start, win_line_end;
};

struct Game {
    Grid array[H][W];
    int current_x, current_y;

    Grid total_grid;
};

void enlarge_rec(Rectangle &r, float amount) {
    r.x      -= amount;
    r.y      -= amount;
    r.width  += amount * 2;
    r.height += amount * 2;
}

Placement check_win(Grid *grid, int *sx, int *sy, int *ex, int *ey) {
    assert(sx && sy && ex && ey);

    // Check rows
    for (int y = 0; y < H; y++) {
        Placement result = grid->array[y][0];

        if (result == PLACEMENT_CLEAR) continue;

        bool found = true;

        for (int x = 1; x < W; x++) {
            if (grid->array[y][x] != result) {
                found = false;
                break;
            }
        }

        if (found) {
            *sx = 0;
            *sy = y;
            *ex = W-1;
            *ey = y;

            return result;
        }
    }

    // Check columns
    for (int x = 0; x < W; x++) {
        Placement result = grid->array[0][x];

        if (result == PLACEMENT_CLEAR) continue;

        bool found = true;

        for (int y = 1; y < H; y++) {
            if (grid->array[y][x] != result) {
                found = false;
                break;
            }
        }

        if (found) {
            *sx = x;
            *sy = 0;
            *ex = x;
            *ey = H-1;

            return result;
        }
    }

    // Check diagonals
    int leading = 1;
    do {
        Placement result;

        if (leading)
            result = grid->array[0][0];
        else
            result = grid->array[0][W-1];

        bool found = true;

        if (result == PLACEMENT_CLEAR) {
            leading--;
            continue;
        }

        for (int i = 0; i < H; i++) {
            int x, y = i;

            if (leading) {
                x = i;
            } else {
                x = W-1-y;
            }

            if (grid->array[y][x] != result) {
                found = false;
                break;
            }
        }

        if (found) {
            if (leading) {
                *sx = 0;
                *sy = 0;
                *ex = W-1;
                *ey = H-1;
            } else {
                *sx = W-1;
                *sy = 0;
                *ex = 0;
                *ey = H-1;
            }

            return result;
        }
        
        leading--;
    } while (leading >= 0);

    return PLACEMENT_CLEAR; // nobody won
}

void win_grid(Grid *grid, Placement winner, int sx, int sy, int ex, int ey) {
    grid->won = winner;
    grid->win_line_start.x = grid->pos.x + (sx + 0.5f) * grid->size / 3.f;
    grid->win_line_start.y = grid->pos.y + (sy + 0.5f) * grid->size / 3.f;
    grid->win_line_end.x   = grid->pos.x + (ex + 0.5f) * grid->size / 3.f;
    grid->win_line_end.y   = grid->pos.y + (ey + 0.5f) * grid->size / 3.f;
}

bool update_grid(Grid *grid) {
    Vector2 mouse = GetMousePosition();

    grid->select_x = -1;
    grid->select_y = -1;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            Rectangle r;

            r.x      = grid->pos.x + x * grid->size / 3.f;
            r.y      = grid->pos.y + y * grid->size / 3.f;
            r.width  = grid->size / 3.f;
            r.height = grid->size / 3.f;

            if (grid->array[y][x] == PLACEMENT_CLEAR && CheckCollisionPointRec(mouse, r)) {
                grid->select_x = x;
                grid->select_y = y;

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    grid->array[y][x] = global_current_placement;

                    if (global_current_placement == PLACEMENT_X)
                        global_current_placement = PLACEMENT_O;
                    else
                        global_current_placement = PLACEMENT_X;

                    int sx, sy, ex, ey;
                    Placement result = check_win(grid, &sx, &sy, &ex, &ey);

                    if (result != PLACEMENT_CLEAR) {
                        win_grid(grid, result, sx, sy, ex, ey);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

void draw_placement(Grid *grid, int x, int y, Placement p, uint8_t alpha) {
    Vector2 pos = grid->pos;

    pos.x += x * grid->size / 3;
    pos.y += y * grid->size / 3;

    pos.x += grid->size / 6;
    pos.y += grid->size / 6;

    float size = grid->size / 8.f;

    if (p == PLACEMENT_X) {
        Color c = PINK;

        c.a = alpha;

        DrawLineEx({pos.x - size, pos.y - size},
                   {pos.x + size, pos.y + size},
                   2,
                   c);
        DrawLineEx({pos.x + size, pos.y - size},
                   {pos.x - size, pos.y + size},
                   2,
                   c);
    } else if (p == PLACEMENT_O) {
        Color c = BLUE;

        c.a = alpha;

        DrawCircleLinesV(pos, size+0, c);
        DrawCircleLinesV(pos, size-1, c);
    }
}

void draw_grid(Grid *grid, Color color, bool is_total_grid) {
    uint8_t alpha = 255;

    if (!is_total_grid && grid->won) {
        alpha = 70;
    }

    color.a = alpha;

    for (int x = 1; x < W; x++) {
        DrawLineV({grid->pos.x + x * grid->size / 3, grid->pos.y},
                  {grid->pos.x + x * grid->size / 3, grid->pos.y + grid->size},
                  color);
    }
    for (int y = 1; y < H; y++) {
        DrawLineV({grid->pos.x, grid->pos.y + y * grid->size / 3 },
                  {grid->pos.x + grid->size, grid->pos.y + y * grid->size / 3},
                  color);
    }

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            Placement p = grid->array[y][x];
            draw_placement(grid, x, y, p, alpha);
        }
    }

    if (grid->won) {
        Color c = PINK;

        if (grid->won == PLACEMENT_O)
            c = BLUE;

        c.a = alpha;

        DrawLineEx(grid->win_line_start, grid->win_line_end, 7, c);
    } else if (grid->select_x != -1 && grid->select_y != -1) {
        int x = grid->select_x;
        int y = grid->select_y;

        draw_placement(grid, x, y, global_current_placement, 200);
    }
}

int main(void) {
    int width = 800;
    int height = 800;

    float padding = 50;

    Game game = {};

    game.current_x = game.current_y = -1;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            Grid *g = &game.array[y][x];
            g->size = width / 3 - padding;
            g->pos  = { padding/2 + x * width / 3.f, padding/2 + y * height / 3.f };
        }
    }

    game.total_grid.size = 1.0f * width;
    game.total_grid.select_x = game.total_grid.select_y = -1;

    SetTraceLogLevel(LOG_ERROR);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    InitWindow(width, height, "Super TicTacToe");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (!global_winner) { // Update
            bool clicked = false;
            Grid *g = 0;
            int g_y = game.current_y;
            int g_x = game.current_x;

            if (game.current_x == -1 && game.current_y == -1) {
                for (int y = 0; y < H; y++) {
                    for (int x = 0; x < W; x++) {
                        g = &game.array[y][x];

                        if (g->won) continue;

                        clicked = update_grid(g);
                        if (clicked) {
                            g_x = x;
                            g_y = y;
                            goto end;
                        }
                    }
                }
            } else {
                g = &game.array[game.current_y][game.current_x];
                clicked = update_grid(g);
            }

end:

            if (clicked) {
                if (g->won) {
                    assert(g_y != -1);
                    assert(g_x != -1);

                    game.total_grid.array[g_y][g_x] = g->won;
                    int sx, sy, ex, ey;
                    Placement result = check_win(&game.total_grid,
                                                 &sx, &sy, &ex, &ey);

                    if (result != PLACEMENT_CLEAR) {
                        // Someone won!
                        win_grid(&game.total_grid, result, sx, sy, ex, ey);
                        global_winner = result;

                        for (int i = 0; i < 3; i++)
                            printf("%s WON!\n", global_winner == PLACEMENT_X ? "X" : "O");
                    }
                }

                Grid *to_grid = &game.array[g->select_y][g->select_x];

                if (to_grid->won) {
                    game.current_x = -1;
                    game.current_y = -1;
                } else {
                    game.current_x = g->select_x;
                    game.current_y = g->select_y;
                }

                g->select_x = -1;
                g->select_y = -1;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                Grid *g = &game.array[y][x];
                draw_grid(g, WHITE, false);
            }
        }

        // Draw green outline
        Rectangle r;

        r.x = game.current_x * width / 3.f;
        r.y = game.current_y * height / 3.f;
        r.width = width / 3.f;
        r.height = height / 3.f;

        enlarge_rec(r, -5);

        DrawRectangleRoundedLinesEx(r, 0.125f, 9, 2, GREEN);

        draw_grid(&game.total_grid, GRAY, true);

        EndDrawing();
    }

    CloseWindow();
}
