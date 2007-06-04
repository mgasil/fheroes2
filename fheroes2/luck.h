/***************************************************************************
 *   Copyright (C) 2006 by Andrey Afletdinov                               *
 *   afletdinov@mail.dc.baikal.ru                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef H2LUCK_H
#define H2LUCK_H

#include <string>

namespace Luck
{
    typedef enum {
        CURSED     = -3,
        AWFUL      = -2,
        BAD        = -1,
        NORMAL     = 0,
        GOOD       = 1,
        GREAT      = 2,
        IRISH      = 3
    } luck_t;

    const std::string & String(luck_t luck);
    const std::string & Description(luck_t luck);

    inline luck_t & operator++ (luck_t & luck){ return luck = IRISH == luck ? IRISH : luck_t(luck + 1); };
    inline luck_t & operator-- (luck_t & luck){ return luck = CURSED == luck ? CURSED : luck_t(luck - 1); };
};

#endif
