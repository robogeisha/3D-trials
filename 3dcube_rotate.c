#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    int a, b, c, d;
} Quad;

Vec3 rotateX(Vec3 v, float angle) {
    return (Vec3){
        .x = v.x,
        .y = v.y * cos(angle) - v.z * sin(angle),
        .z = v.y * sin(angle) + v.z * cos(angle),
    };
}

Vec3 rotateY(Vec3 v, float angle) {
    return (Vec3){
        .x = v.x * cos(angle) + v.z * sin(angle),
        .y = v.y,
        .z = -v.x * sin(angle) + v.z * cos(angle),
    };
}

Vec3 rotateZ(Vec3 v, float angle) {
    return (Vec3){
        .x = v.x * cos(angle) - v.y * sin(angle),
        .y = v.x * sin(angle) + v.y * cos(angle),
        .z = v.z,
    };
}

void render(Vec3 *vertices, Quad *quads, int num_quads, float rot_x, float rot_y, float rot_z) {
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);

    int w = ws.ws_col;
    int h = ws.ws_row;

    char screen[h][w];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            screen[y][x] = ' ';
        }
    }

    for (int i = 0; i < num_quads; i++) {
        Vec3 v[4] = {
            rotateZ(rotateY(rotateX(vertices[quads[i].a], rot_x), rot_y), rot_z),
            rotateZ(rotateY(rotateX(vertices[quads[i].b], rot_x), rot_y), rot_z),
            rotateZ(rotateY(rotateX(vertices[quads[i].c], rot_x), rot_y), rot_z),
            rotateZ(rotateY(rotateX(vertices[quads[i].d], rot_x), rot_y), rot_z),
        };

        int x[4] = {w / 2 + v[0].x * w / 4, w / 2 + v[1].x * w / 4, w / 2 + v[2].x * w / 4, w / 2 + v[3].x * w / 4};
        int y[4] = {h / 2 - v[0].y * h / 4, h / 2 - v[1].y * h / 4, h / 2 - v[2].y * h / 4, h / 2 - v[3].y * h / 4};

        for (int j = 0; j < 4; j++) {
            int x1 = x[j];
            int y1 = y[j];
            int x2 = x[(j + 1) % 4];
            int y2 = y[(j + 1) % 4];

            int dx = abs(x2 - x1);
            int dy = abs(y2 - y1);
                     int steps = dx > dy ? dx : dy;
            float x_inc = (float)(x2 - x1) / steps;
            float y_inc = (float)(y2 - y1) / steps;
            float x_curr = x1;
            float y_curr = y1;

            for (int k = 0; k <= steps; k++) {
                int x_screen = round(x_curr);
                int y_screen = round(y_curr);

                if (x_screen >= 0 && x_screen < w && y_screen >= 0 && y_screen < h) {
                    screen[y_screen][x_screen] = '#';
                }

                x_curr += x_inc;
                y_curr += y_inc;
            }
        }
    }

    printf("\033[1;1H");
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            putchar(screen[y][x]);
        }
    }
}

int main() {
    Vec3 vertices[8] = {
        {.x = -1, .y = -1, .z = -1},
        {.x = 1, .y = -1, .z = -1},
        {.x = 1, .y = 1, .z = -1},
        {.x = -1, .y = 1, .z = -1},
        {.x = -1, .y = -1, .z = 1},
        {.x = 1, .y = -1, .z = 1},
        {.x = 1, .y = 1, .z = 1},
        {.x = -1, .y = 1, .z = 1},
    };

    Quad quads[6] = {
        {.a = 0, .b = 1, .c = 2, .d = 3},
        {.a = 1, .b = 5, .c = 6, .d = 2},
        {.a = 5, .b = 4, .c = 7, .d = 6},
        {.a = 4, .b = 0, .c = 3, .d = 7},
        {.a = 3, .b = 2, .c = 6, .d = 7},
        {.a = 4, .b = 5, .c = 1, .d = 0},
    };

    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);

    float rot_x = 0;
    float rot_y = 0;
    float rot_z = 0;

    while (1) {
        render(vertices, quads, 6, rot_x, rot_y, rot_z);
        rot_x += 0.01;
        rot_y += 0.02;
        rot_z += 0.03;
        usleep(20000);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    return 0;
}
