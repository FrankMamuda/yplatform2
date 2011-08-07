#ifndef R_FONT_H
#define R_FONT_H

//
// includes
//
#include "r_shared.h"

//
// defines
//
#define FONT_DATA_SIZE 9236
#define FONT_NUM_GLYPHS 256

typedef struct glyphDef_s {
    float width;
    float height;
    float horiAdvance;
    float horiBearingX;
    float horiBearingY;
    float s1;
    float t1;
    float s2;
    float t2;
} glyphDef_t;

// enough data to render font from a module
typedef struct fontInfo_s {
    QList <glyphDef_t*>glyphList;
    float pointSize;
    float fontHeight;
    float ascender;
    float descender;
    imgHandle_t image;
} fontInfo_t;

#endif // R_FONT_H
