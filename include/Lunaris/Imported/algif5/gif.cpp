#include "algif.h"

/* Destroy a complete gif, including all frames. */
LUNARIS_DECL void algif_destroy_animation(ALGIF_ANIMATION *gif) {
    for (int i = 0; i < gif->frames_count; i++)
    {
        ALGIF_FRAME *frame = gif->frames + i;

        if (frame->bitmap_8_bit)
            algif_destroy_bitmap (frame->bitmap_8_bit);
        if (frame->rendered)
            al_destroy_bitmap(frame->rendered);
    }
    if (gif->store)
        al_destroy_bitmap(gif->store);
    free (gif->frames);
    free (gif);
}

LUNARIS_DECL void read_palette(ALLEGRO_FILE * file, ALGIF_PALETTE *palette) {
    for (int i = 0; i < palette->colors_count; i++)
    {
        palette->colors[i].r = al_fgetc(file);
        palette->colors[i].g = al_fgetc(file);
        palette->colors[i].b = al_fgetc(file);
    }
}

LUNARIS_DECL void deinterlace(ALGIF_BITMAP *bmp)
{
    ALGIF_BITMAP *n = algif_create_bitmap(bmp->w, bmp->h);
    int i = 0;
    for (int y = 0; y < n->h; y += 8)
    {
        algif_blit(bmp, n, 0, i++, 0, y, n->w, 1);
    }
    for (int y = 4; y < n->h; y += 8)
    {
        algif_blit(bmp, n, 0, i++, 0, y, n->w, 1);
    }
    for (int y = 2; y < n->h; y += 4)
    {
        algif_blit(bmp, n, 0, i++, 0, y, n->w, 1);
    }
    for (int y = 1; y < n->h; y += 2)
    {
        algif_blit(bmp, n, 0, i++, 0, y, n->w, 1);
    }
    algif_blit(n, bmp, 0, 0, 0, 0, n->w, n->h);
    algif_destroy_bitmap(n);
}

LUNARIS_DECL ALGIF_ANIMATION* algif_load_raw(ALLEGRO_FILE* file, const bool del_fp) {
    if (!file)
        return nullptr;

    al_fseek(file, 0, ALLEGRO_SEEK_SET);

    int version;
    ALGIF_BITMAP *bmp = nullptr;
    int i, j;
    ALGIF_ANIMATION *gif = (ALGIF_ANIMATION*)calloc(1, sizeof(ALGIF_ANIMATION));
    ALGIF_FRAME frame;

    const auto freeup = [&, del_fp] {
        if (file && del_fp)
            al_fclose(file);
        if (gif)
            algif_destroy_animation(gif);
        if (bmp)
            algif_destroy_bitmap(bmp);
    };

    gif->frames_count = 0;

    /* is it really a GIF? */
    if (al_fgetc(file) != 'G') { freeup(); return nullptr; }
    if (al_fgetc(file) != 'I') { freeup(); return nullptr; }
    if (al_fgetc(file) != 'F') { freeup(); return nullptr; }
    if (al_fgetc(file) != '8') { freeup(); return nullptr; }

    /* '7' or '9', for 87a or 89a. */
    version = al_fgetc(file);
    if (version != '7' && version != '9') { freeup(); return nullptr; }
    if (al_fgetc(file) != 'a') { freeup(); return nullptr; }

    gif->width = al_fread16le(file);
    gif->height = al_fread16le(file);
    i = al_fgetc(file);
    /* Global color table? */
    if (i & 128)
        gif->palette.colors_count = 1 << ((i & 7) + 1);
    else
        gif->palette.colors_count = 0;
    /* Background color is only valid with a global palette. */
    gif->background_index = al_fgetc(file);

    /* Skip aspect ratio. */
    al_fseek(file, 1, ALLEGRO_SEEK_CUR);

    if (gif->palette.colors_count)
    {
        read_palette(file, &gif->palette);
    }

    memset(&frame, 0, sizeof(frame)); /* For first frame. */
    frame.transparent_index = -1;

    do
    {
        i = al_fgetc(file);

        switch (i)
        {
        case 0x2c:         /* Image Descriptor */
        {
            int w, h;
            int interlaced = 0;

            frame.xoff = al_fread16le(file);
            frame.yoff = al_fread16le(file);
            w = al_fread16le(file);
            h = al_fread16le(file);
            bmp = algif_create_bitmap(w, h);
            if (!bmp) { freeup(); return nullptr; }
            i = al_fgetc(file);

            /* Local palette. */
            if (i & 128)
            {
                frame.palette.colors_count = 1 << ((i & 7) + 1);
                read_palette(file, &frame.palette);
            }
            else
            {
                frame.palette.colors_count = 0;
            }

            if (i & 64)
                interlaced = 1;

            if (LZW_decode(file, bmp)) { freeup(); return nullptr; }

            if (interlaced)
                deinterlace(bmp);

            frame.bitmap_8_bit = bmp;
            bmp = NULL;

            gif->frames_count++;
            gif->frames = (ALGIF_FRAME*)realloc((void*)gif->frames,
                gif->frames_count * sizeof * gif->frames);
            gif->frames[gif->frames_count - 1] = frame;

            memset(&frame, 0, sizeof frame); /* For next frame. */
            frame.transparent_index = -1;

            break;
        }
        case 0x21: /* Extension Introducer. */
            j = al_fgetc(file); /* Extension Type. */
            i = al_fgetc(file); /* Size. */
            if (j == 0xf9) /* Graphic Control Extension. */
            {
                /* size must be 4 */
                if (i != 4) { freeup(); return nullptr; }

                i = al_fgetc(file);
                frame.disposal_method = (i >> 2) & 7;
                frame.duration = al_fread16le(file);
                if (i & 1)  /* Transparency? */
                {
                    frame.transparent_index = al_fgetc(file);
                }
                else
                {
                    al_fseek(file, 1, ALLEGRO_SEEK_CUR);
                    frame.transparent_index = -1;
                }
                i = al_fgetc(file); /* Size. */
            }
            /* Application Extension. */
            else if (j == 0xff)
            {
                if (i == 11)
                {
                    char name[12];
                    al_fread(file, name, 11);
                    i = al_fgetc(file); /* Size. */
                    name[11] = '\0';
                    if (!strcmp(name, "NETSCAPE2.0"))
                    {
                        if (i == 3)
                        {
                            j = al_fgetc(file);
                            gif->loop = al_fread16le(file);
                            if (j != 1)
                                gif->loop = 0;
                            i = al_fgetc(file); /* Size. */
                        }
                    }
                }
            }

            /* Possibly more blocks until terminator block (0). */
            while (i)
            {
                al_fseek(file, i, ALLEGRO_SEEK_CUR);
                i = al_fgetc(file);
            }
            break;
        case 0x3b:
            /* GIF Trailer. */
            if (del_fp) al_fclose(file);
            return gif;
        }
    } while (true);

    freeup();
    return nullptr;
}
