#ifndef H2PAL_H
#define H2PAL_H

#include "types.h"
#include "surface.h"
#include <vector>

#define PALETTE_SIZE 256

namespace PAL
{
    enum
    {
        STANDARD,        //default
        YELLOW_TEXT,
        WHITE_TEXT,
        GRAY_TEXT,
        RED,             //blood lust, ...
        GRAY,            //petrify, ...
        BROWN,
        TAN,             //puzzle
        NO_CYCLE,
        MIRROR_IMAGE
    };

    void CreatePalette( std::vector<SDL_Color> & palette, const u8 table[PALETTE_SIZE] );
    void CreateStandardPalette();
    void InitAllPalettes();
    void Clear();
    int CurrentPalette();
    void SwapPalette( int type );
    RGBA GetPaletteColor( u8 index );
}

#endif
