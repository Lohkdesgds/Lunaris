#pragma once

#include <allegro5/allegro5.h>

typedef struct ALGIF_ANIMATION ALGIF_ANIMATION;
typedef struct ALGIF_FRAME ALGIF_FRAME;
typedef struct ALGIF_PALETTE ALGIF_PALETTE;
typedef struct ALGIF_BITMAP ALGIF_BITMAP;
typedef struct ALGIF_RGB ALGIF_RGB;

struct ALGIF_RGB {
    uint8_t r, g, b;
};

struct ALGIF_PALETTE {
    int colors_count;
    ALGIF_RGB colors[256];
};

struct ALGIF_BITMAP {
    int w, h;
    uint8_t *data;
};

struct ALGIF_ANIMATION {
    int width, height;
    int frames_count;
    int background_index;
    int loop; /* -1 = no, 0 = forever, 1..65535 = that many times */
    ALGIF_PALETTE palette;
    ALGIF_FRAME *frames;

    int duration;
    ALLEGRO_BITMAP *store;
};

struct ALGIF_FRAME {
    ALGIF_BITMAP *bitmap_8_bit;
    ALGIF_PALETTE palette;
    int xoff, yoff;
    int duration;               /* in 1/100th seconds */
    int disposal_method;        /* 0 = don't care, 1 = keep, 2 = background, 3 = previous */
    int transparent_index;

    ALLEGRO_BITMAP *rendered;
};

int read_code(ALLEGRO_FILE*, char*, int*, int);
int LZW_decode(ALLEGRO_FILE*, ALGIF_BITMAP*);

inline static void read_palette(ALLEGRO_FILE* file, ALGIF_PALETTE* palette);
inline static void deinterlace(ALGIF_BITMAP* bmp);


ALGIF_ANIMATION* algif_load_raw(ALLEGRO_FILE*);
ALGIF_ANIMATION* algif_load_animation_f(ALLEGRO_FILE*);
ALGIF_ANIMATION* algif_load_animation(char const*);
void algif_render_frame(ALGIF_ANIMATION*, int, int, int);
void algif_destroy_animation(ALGIF_ANIMATION*);

ALGIF_BITMAP* algif_create_bitmap(int, int);
void algif_destroy_bitmap(ALGIF_BITMAP*);
void algif_blit(ALGIF_BITMAP*, ALGIF_BITMAP*, int, int, int, int, int, int);
ALLEGRO_BITMAP* algif_get_bitmap(ALGIF_ANIMATION*, double);
ALLEGRO_BITMAP* algif_get_frame_bitmap(ALGIF_ANIMATION*, int);
double algif_get_frame_duration(ALGIF_ANIMATION*, int);