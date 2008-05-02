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

#include <list>
#include <iostream>
#include <algorithm>
#include "agg.h"
#include "icn.h"
#include "tools.h"
#include "world.h"
#include "race.h"
#include "config.h"
#include "error.h"
#include "heroes.h"
#include "sprite.h"
#include "maps.h"
#include "gamearea.h"
#include "game_focus.h"
#include "display.h"
#include "maps_tiles.h"

Maps::TilesAddon::TilesAddon(u8 lv, u32 gid, u8 obj, u8 ii) : level(lv), uniq(gid), object(obj), index(ii)
{
}

Maps::TilesAddon & Maps::TilesAddon::operator= (const Maps::TilesAddon & ta)
{
    level = ta.level;

    object = ta.object;

    index = ta.index;

    uniq = ta.uniq;
    
    return *this;
}

bool Maps::TilesAddon::PredicateSortRules1(const Maps::TilesAddon & ta1, const Maps::TilesAddon & ta2)
{
    return ((ta1.level % 4) > (ta2.level % 4));
}

bool Maps::TilesAddon::PredicateSortRules2(const Maps::TilesAddon & ta1, const Maps::TilesAddon & ta2)
{
    return ((ta1.level % 4) < (ta2.level % 4));
}

u16 Maps::TilesAddon::isRoad(const TilesAddon & ta, u8 direct)
{
    switch(ta.object)
    {
	// castle and tower (gate)
    	case 0x8C:
	case 0x8D:
	case 0x8E:
	case 0x8F:
	    return (13 == ta.index ||
		    29 == ta.index ||
	    	    45 == ta.index ||
	    	    61 == ta.index ||
	    	    77 == ta.index ||
	    	    93 == ta.index ||
	    	    109 == ta.index ||
	    	    125 == ta.index ||
	    	    141 == ta.index ||
	    	    157 == ta.index ||
	    	    173 == ta.index ||
	    	    189 == ta.index ? Direction::TOP | Direction::BOTTOM : 0);

	// castle lands (gate)
        case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	    return ( 7 == ta.index ||
		    17 == ta.index ||
		    27 == ta.index ||
		    37 == ta.index ||
		    47 == ta.index ||
		    57 == ta.index ||
		    67 == ta.index ||
		    77 == ta.index ? Direction::TOP | Direction::BOTTOM : 0);

	// from sprite road
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	    if(0  == ta.index ||
	       4  == ta.index ||
	       5  == ta.index ||
	       7  == ta.index ||
	       9  == ta.index ||
	       12 == ta.index ||
	       13 == ta.index ||
	       16 == ta.index ||
	       19 == ta.index ||
	       20 == ta.index ||
	       26 == ta.index)	return Direction::TOP | Direction::BOTTOM;
	    else
	    if(2  == ta.index ||
	       21 == ta.index ||
	       28 == ta.index)	return Direction::RIGHT | Direction::LEFT;
	    else
	    if(3  == ta.index ||
	       6  == ta.index ||
	       14 == ta.index)	return Direction::TOP | Direction::BOTTOM | Direction::RIGHT | Direction::LEFT;
	    else
	    if(17 == ta.index ||
	       29 == ta.index)	return Direction::TOP_LEFT | Direction::BOTTOM_RIGHT;
	    else
	    if(18 == ta.index ||
	       30 == ta.index)	return Direction::TOP_RIGHT | Direction::BOTTOM_LEFT;
	
	default:
	    break;
    }

    return 0;
}

Maps::Tiles::Tiles(u16 mi, const MP2::mp2tile_t & mp2tile) : maps_index(mi), tile_sprite(TILEWIDTH, TILEWIDTH, 8, SDL_SWSURFACE), tile_index(mp2tile.tileIndex),
    shape(mp2tile.shape), general(mp2tile.generalObject), quantity1(mp2tile.quantity1), quantity2(mp2tile.quantity2), path_sprite(NULL),
    animation(false), fogs(Color::BLUE | Color::GREEN | Color::RED | Color::YELLOW | Color::ORANGE | Color::PURPLE)
{
    tile_sprite.LoadPalette(AGG::Cache::Get().GetPAL());

    SetTile(mp2tile.tileIndex, mp2tile.shape);

    AddonsPushLevel1(mp2tile);
    AddonsPushLevel2(mp2tile);
}

void Maps::Tiles::SetTile(const u16 index, const u8 shape)
{
    AGG::GetTIL(TIL::GROUND32, index, shape, tile_sprite);
}

void Maps::Tiles::AddonsPushLevel1(const MP2::mp2tile_t & mt)
{
    if(mt.objectName1 && mt.indexName1 < 0xFF)
	addons_level1.push_back(TilesAddon(0, mt.uniqNumber1, mt.objectName1, mt.indexName1));
}

void Maps::Tiles::AddonsPushLevel1(const MP2::mp2addon_t & ma)
{
    if(ma.objectNameN1 && ma.indexNameN1 < 0xFF)
	addons_level1.push_back(TilesAddon(ma.quantityN, ma.uniqNumberN1, ma.objectNameN1, ma.indexNameN1));
}

void Maps::Tiles::AddonsPushLevel2(const MP2::mp2tile_t & mt)
{
    if(mt.objectName2 && mt.indexName2 < 0xFF)
	addons_level2.push_back(TilesAddon(0, mt.uniqNumber2, mt.objectName2, mt.indexName2));
}

void Maps::Tiles::AddonsPushLevel2(const MP2::mp2addon_t & ma)
{
    if(ma.objectNameN2 && ma.indexNameN2 < 0xFF)
	addons_level2.push_back(TilesAddon(ma.quantityN, ma.uniqNumberN2, ma.objectNameN2, ma.indexNameN2));
}

void Maps::Tiles::AddonsSort(void)
{
    if(addons_level1.size()) addons_level1.sort(Maps::TilesAddon::PredicateSortRules1);

    if(addons_level2.size()) addons_level2.sort(Maps::TilesAddon::PredicateSortRules2);
}

Maps::Ground::ground_t Maps::Tiles::GetGround(void) const
{
    // list grounds from GROUND32.TIL
    if(30 > tile_index)
        return Maps::Ground::WATER;
        
    else if(92 > tile_index)
        return Maps::Ground::GRASS;

    else if(146 > tile_index)
        return Maps::Ground::SNOW;

    else if(208 > tile_index)
        return Maps::Ground::SWAMP;

    else if(262 > tile_index)
        return Maps::Ground::LAVA;

    else if(321 > tile_index)
        return Maps::Ground::DESERT;

    else if(361 > tile_index)
        return Maps::Ground::DIRT;

    else if(415 > tile_index)
        return Maps::Ground::WASTELAND;

    //else if(432 > tile_index)

	return Maps::Ground::BEACH;
}

void Maps::Tiles::Remove(u32 uniq)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator       it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	while(it1 != it2)
	    if(uniq == (*it1).uniq)
		it1 = addons_level1.erase(it1);
	    else
		++it1;
    }

    if(addons_level2.size())
    {
	std::list<TilesAddon>::iterator       it1 = addons_level2.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level2.end();

	while(it1 != it2)
	    if(uniq == (*it1).uniq)
		it1 = addons_level2.erase(it1);
	    else
		++it1;
    }
}

void Maps::Tiles::RedrawAll(const u32 anime_frame) const
{
    RedrawTile();
    RedrawBottom(anime_frame);

    switch(general)
    {
	// boat
	case MP2::OBJ_BOAT:
	    RedrawBoat();
	    break;

	// heroes
	case MP2::OBJ_HEROES:
	    RedrawHeroes();
	    break;

	// monster
	case MP2::OBJ_MONSTER:
	    RedrawMonster(anime_frame);
	    break;
	
	default:
	    break;
    }

    RedrawTop(anime_frame);
    RedrawRoute();
    RedrawGrid();
}

void Maps::Tiles::RedrawTile(void) const
{
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(area & mp)
    {
	const s16 dstx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
	const s16 dsty = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

	Display::Get().Blit(tile_sprite, dstx, dsty);
    }
}

void Maps::Tiles::RedrawBottom(const u32 anime_frame) const
{
    Display & display = Display::Get();
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(area & mp)
    {
	const s16 dstx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
	const s16 dsty = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

	if(addons_level1.size())
	{
	    std::list<TilesAddon>::const_iterator it1 = addons_level1.begin();
	    std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	    for(; it1 != it2; ++it1)
	    {
		const u8 & object = (*it1).object;
	        const u8 & index  = (*it1).index;
		const ICN::icn_t icn = MP2::GetICNObject(object);

		if(ICN::UNKNOWN != icn && ICN::MINIHERO != icn && ICN::MONS32 != icn)
		{
		    const Sprite & sprite = AGG::GetICN(icn, index);
		    display.Blit(sprite, dstx + sprite.x(), dsty + sprite.y());

		    // possible anime
		    if(const u16 anime_index = ICN::AnimationFrame(icn, index, anime_frame))
		    {
			const Sprite & anime_sprite = AGG::GetICN(icn, anime_index);
			display.Blit(anime_sprite, dstx + anime_sprite.x(), dsty + anime_sprite.y());
		    }
		}
	    }
	}
    }
}

void Maps::Tiles::RedrawTop(const u32 anime_frame) const
{
    Display & display = Display::Get();
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(area & mp)
    {
	const s16 dstx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
	const s16 dsty = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

	if(addons_level2.size())
	{
	    std::list<TilesAddon>::const_iterator it1 = addons_level2.begin();
	    std::list<TilesAddon>::const_iterator it2 = addons_level2.end();

	    for(; it1 != it2; ++it1)
	    {
		const u8 & object = (*it1).object;
		const u8 & index  = (*it1).index;
	        const ICN::icn_t icn = MP2::GetICNObject(object);

		if(ICN::UNKNOWN != icn && ICN::MINIHERO != icn && ICN::MONS32 != icn)
		{
		    const Sprite & sprite = AGG::GetICN(icn, index);
		    display.Blit(sprite, dstx + sprite.x(), dsty + sprite.y());

		    // possible anime
		    if(const u16 anime_index = ICN::AnimationFrame(icn, index, anime_frame))
		    {
			const Sprite & anime_sprite = AGG::GetICN(icn, anime_index);
			display.Blit(anime_sprite, dstx + anime_sprite.x(), dsty + anime_sprite.y());
		    }
		}
	    }
	}
    }
}

void Maps::Tiles::RedrawRoute(void) const
{
    if(!path_sprite) return;

    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(area & mp)
    {
	const s16 dstx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
	const s16 dsty = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

	Display::Get().Blit(*path_sprite, dstx + 16 - path_sprite->w() / 2, dsty + path_sprite->y() - 3);
    }
}

void Maps::Tiles::RedrawGrid(void) const
{
    if(! H2Config::Debug()) return;

    Display & display = Display::Get();
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(area & mp)
    {
	const s16 dstx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
	const s16 dsty = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

	display.Lock();
	display.SetPixel(dstx, dsty, AGG::GetColor(0x40));
	display.Unlock();
    }
}

bool Maps::Tiles::isAnimation(void) const
{
    // check object
    switch(general)
    {
	case MP2::OBJ_HEROES:
	case MP2::OBJ_MONSTER:	return true;

	default: break;
    }

    return animation;
}

void Maps::Tiles::FixAnimation(void)
{
    // level 1
    if(!animation && addons_level1.size())
    {
	std::list<TilesAddon>::const_iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    const u8 & object = (*it1).object;
	    const u8 & index  = (*it1).index;
	    const ICN::icn_t icn = MP2::GetICNObject(object);

	    if(ICN::UNKNOWN != icn && ICN::AnimationFrame(icn, index))
	    {
		animation = true;
		
		break;
	    }
	}
    }

    // level 2
    if(!animation && addons_level2.size())
    {
	std::list<TilesAddon>::const_iterator it1 = addons_level2.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level2.end();

	for(; it1 != it2; ++it1)
	{
	    const u8 & object = (*it1).object;
	    const u8 & index  = (*it1).index;
	    const ICN::icn_t icn = MP2::GetICNObject(object);

	    if(ICN::UNKNOWN != icn && ICN::AnimationFrame(icn, index))
	    {
		animation = true;

		break;
	    }
	}
    }
}

Maps::TilesAddon * Maps::Tiles::FindAddonLevel1(u32 uniq1)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    if(uniq1 == addon.uniq) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindAddonLevel2(u32 uniq2)
{
    if(addons_level2.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level2.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level2.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    if(uniq2 == addon.uniq) return &addon;
	}
    }

    return NULL;
}

void Maps::Tiles::DebugInfo(u16 index) const
{
    std::list<TilesAddon>::const_iterator it1;
    std::list<TilesAddon>::const_iterator it2;

    std::cout << std::endl << "----------------:--------" << std::endl;

    std::string value;

    if(0xFFFF > index)
    {
	String::AddInt(value, index);

	std::cout << "index           : " << value << std::endl;
    
	value.clear();
    }
    
    String::AddInt(value, tile_index);

    std::cout << "tile            : " << value << std::endl;
    
    value.clear();

    value = Ground::String(GetGround());
    if(isRoad()) value += ", (road)";
    std::cout << "ground          : " << value << std::endl;

    value.clear();

    value = isPassable() ? "true" : "false";
    std::cout << "passable        : " << value << std::endl;

    value.clear();
    
    String::AddInt(value, general);
    value += ", (" + std::string(MP2::StringObject(general)) + ")";
    std::cout << "general object  : " << value << std::endl;

    value.clear();
    
    String::AddInt(value, quantity1);
    
    std::cout << "quantity 1      : " << value << std::endl;

    value.clear();
    
    String::AddInt(value, quantity2);
    
    std::cout << "quantity 2      : " << value << std::endl;

    if(addons_level1.size())
    {
	it1 = addons_level1.begin();
	it2 = addons_level1.end();
	
	for(; it1 != it2; ++it1)
	{
	    const TilesAddon & addon = *it1;

	    std::cout << "----------------1--------" << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.object);
    
	    std::cout << "object          : " << value << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.index);
    
	    std::cout << "index           : " << value << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.uniq);
    
	    std::cout << "uniq            : " << value << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.level);
    
	    std::cout << "level           : " << value << std::endl;
	}
    }

    if(addons_level2.size())
    {
	it1 = addons_level2.begin();
	it2 = addons_level2.end();
	
	for(; it1 != it2; ++it1)
	{
	    const TilesAddon & addon = *it1;

	    std::cout << "----------------2--------" << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.object);
    
	    std::cout << "object          : " << value << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.index);
    
	    std::cout << "index           : " << value << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.uniq);
    
	    std::cout << "uniq            : " << value << std::endl;

	    value.clear();
    
	    String::AddInt(value, addon.level);
    
	    std::cout << "level           : " << value << std::endl;
	}
    }

    std::cout << "----------------:--------" << std::endl << std::endl;
}

void Maps::Tiles::RedrawMonster(const u32 anime_sprite) const
{
    Display & display = Display::Get();
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(!(area & mp)) return;
    const s16 dx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
    const s16 dy = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

    Monster::monster_t monster = Monster::Monster(*this);

    if(Monster::UNKNOWN <= monster)
    {
	Error::Warning("Maps::Tiles::RedrawMonster: unknown monster, ", monster);

	return;
    }
	
    u8 align_x = 0;
	
    switch(monster)
    {
	    case Monster::CAVALRY:
	    case Monster::CHAMPION:
	    case Monster::UNICORN:
	    case Monster::PHOENIX:
	    case Monster::CENTAUR:
	    case Monster::HYDRA:
	    case Monster::GREEN_DRAGON:
	    case Monster::RED_DRAGON:
	    case Monster::BLACK_DRAGON:
	    case Monster::BOAR:
	    case Monster::ROC:
	    case Monster::BONE_DRAGON:
	    case Monster::NOMAD:
		align_x = 22;
		break;

	    case Monster::WOLF:
	    case Monster::GRIFFIN:
		align_x = 18;
		break;

	    default:
		align_x = 16;
		break;
    }

    // redraw top tiles
    if(BORDERWIDTH < dy) world.GetTiles(maps_index - world.w()).RedrawAll(anime_sprite);

    // redraw left tiles
    if(BORDERWIDTH < dx) world.GetTiles(maps_index - 1).RedrawAll(anime_sprite);

    // draw first sprite
    const Sprite & sprite_first = AGG::GetICN(ICN::MINIMON, monster * 9);

    Point dst_pt(dx + TILEWIDTH - std::abs(sprite_first.x()) - align_x,
                 dy + TILEWIDTH - std::abs(sprite_first.y()) - 4);
    Rect src_rt;

    GameArea::SrcRectFixed(src_rt, dst_pt, sprite_first.w(), sprite_first.h());

    display.Blit(sprite_first, src_rt, dst_pt);

    // draw second sprite
    const Sprite & sprite_next = AGG::GetICN(ICN::MINIMON, monster * 9 + 1 + (anime_sprite % 6));

    dst_pt.x = dx + TILEWIDTH - std::abs(sprite_next.x()) - align_x;
    dst_pt.y = dy + TILEWIDTH - std::abs(sprite_next.y()) - 4;

    GameArea::SrcRectFixed(src_rt, dst_pt, sprite_next.w(), sprite_next.h());

    display.Blit(sprite_next, src_rt, dst_pt);
}

void Maps::Tiles::RedrawBoat(void) const
{
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(area & mp)
    {
	const s16 dstx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
	const s16 dsty = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

	Display::Get().Blit(AGG::GetICN(ICN::OBJNWAT2, 23), dstx, dsty);
    }
}

void Maps::Tiles::RedrawHeroes(void) const
{
    Display & display = Display::Get();
    const Rect & area = GameArea::GetRect();
    const Point mp(maps_index % world.w(), maps_index / world.w());

    if(!(area & mp)) return;
    const s16 dx = BORDERWIDTH + TILEWIDTH * (mp.x - area.x);
    const s16 dy = BORDERWIDTH + TILEWIDTH * (mp.y - area.y);

    if(const Heroes *heroes = world.GetHeroes(maps_index))
    {
	bool reflect = heroes->ReflectSprite();

	const Sprite & sprite1 = heroes->SpriteHero(0);
	const Sprite & sprite2 = heroes->SpriteFlag(0);

	Point dst_pt1(reflect ? dx + TILEWIDTH - sprite1.x() - sprite1.w() : dx + sprite1.x(), dy + sprite1.y() + TILEWIDTH);
	Point dst_pt2(reflect ? dx + TILEWIDTH - sprite2.x() - sprite2.w() : dx + sprite2.x(), dy + sprite2.y() + TILEWIDTH);

	dst_pt1.y -= 5;
	dst_pt2.y -= 5;

	Rect src_rt1;
	Rect src_rt2;

	GameArea::SrcRectFixed(src_rt1, dst_pt1, sprite1.w(), sprite1.h());
	GameArea::SrcRectFixed(src_rt2, dst_pt2, sprite2.w(), sprite2.h());

	display.Blit(sprite1, src_rt1, dst_pt1);
	display.Blit(sprite2, src_rt2, dst_pt2);
    }
    else
	Error::Warning("Maps::Tiles::RedrawHeroes: unknown hero, maps index: ", maps_index);
}

MP2::object_t Maps::Tiles::GetObject(void) const
{
    switch(general)
    {
	case MP2::OBJ_ZERO:		return MP2::OBJ_ZERO;
	case MP2::OBJN_ALCHEMYTOWER:	return MP2::OBJN_ALCHEMYTOWER;
	case MP2::OBJN_DAEMONCAVE:	return MP2::OBJN_DAEMONCAVE;
	case MP2::OBJN_FAERIERING:	return MP2::OBJN_FAERIERING;
	case MP2::OBJN_GAZEBO:		return MP2::OBJN_GAZEBO;
	case MP2::OBJN_GRAVEYARD:	return MP2::OBJN_GRAVEYARD;
        case MP2::OBJN_ARCHERHOUSE:	return MP2::OBJN_ARCHERHOUSE;
        case MP2::OBJN_DWARFCOTT:	return MP2::OBJN_DWARFCOTT;

        case MP2::OBJN_PEASANTHUNT:	return MP2::OBJN_PEASANTHUNT;
	case MP2::OBJN_DRAGONCITY:	return MP2::OBJN_DRAGONCITY;
	case MP2::OBJN_LIGHTHOUSE:	return MP2::OBJN_LIGHTHOUSE;
	case MP2::OBJN_WATERMILL:	return MP2::OBJN_WATERMILL;
	case MP2::OBJN_MINES:		return MP2::OBJN_MINES;
	case MP2::OBJN_OBELISK:		return MP2::OBJN_OBELISK;
	case MP2::OBJN_OASIS:		return MP2::OBJN_OASIS;
	case MP2::OBJ_COAST:		return MP2::OBJ_COAST;
	case MP2::OBJN_SAWMILL:		return MP2::OBJN_SAWMILL;
	case MP2::OBJN_ORACLE:		return MP2::OBJN_ORACLE;

	case MP2::OBJN_DERELICTSHIP:	return MP2::OBJN_DERELICTSHIP;
	case MP2::OBJN_DESERTTENT:	return MP2::OBJN_DESERTTENT;
	case MP2::OBJN_CASTLE:		return MP2::OBJN_CASTLE;
        case MP2::OBJN_STONELIGHTS:	return MP2::OBJN_STONELIGHTS;
	case MP2::OBJN_WAGONCAMP:	return MP2::OBJN_WAGONCAMP;
	case MP2::OBJN_WINDMILL:	return MP2::OBJN_WINDMILL;

	case MP2::OBJN_RNDTOWN:		return MP2::OBJN_RNDTOWN;
	case MP2::OBJN_RNDCASTLE:	return MP2::OBJN_RNDCASTLE;
	case MP2::OBJ_SHRUB2:		return MP2::OBJ_SHRUB2;
	case MP2::OBJ_NOTHINGSPECIAL:	return MP2::OBJ_NOTHINGSPECIAL;
	case MP2::OBJN_WATCHTOWER:	return MP2::OBJN_WATCHTOWER;
	case MP2::OBJN_TREEHOUSE:	return MP2::OBJN_TREEHOUSE;
	case MP2::OBJN_TREECITY:	return MP2::OBJN_TREECITY;
	case MP2::OBJN_RUINS:		return MP2::OBJN_RUINS;
	case MP2::OBJN_FORT:		return MP2::OBJN_FORT;
	case MP2::OBJN_TRADINGPOST:	return MP2::OBJN_TRADINGPOST;

	case MP2::OBJN_ABANDONEDMINE:	return MP2::OBJN_ABANDONEDMINE;
	case MP2::OBJN_TREEKNOWLEDGE:	return MP2::OBJN_TREEKNOWLEDGE;
	case MP2::OBJN_DOCTORHUT:	return MP2::OBJN_DOCTORHUT;
	case MP2::OBJN_TEMPLE:		return MP2::OBJN_TEMPLE;
	case MP2::OBJN_HILLFORT:	return MP2::OBJN_HILLFORT;
	case MP2::OBJN_HALFLINGHOLE:	return MP2::OBJN_HALFLINGHOLE;
	case MP2::OBJN_MERCENARYCAMP:	return MP2::OBJN_MERCENARYCAMP;
	case MP2::OBJN_PIRAMID:		return MP2::OBJN_PIRAMID;
	case MP2::OBJN_CITYDEAD:	return MP2::OBJN_CITYDEAD;
	case MP2::OBJN_EXCAVATION:	return MP2::OBJN_EXCAVATION;
	case MP2::OBJN_SPHINX:		return MP2::OBJN_SPHINX;

	case MP2::OBJ_OILLAKE:		return MP2::OBJ_OILLAKE;
	case MP2::OBJN_TROLLBRIDGE:	return MP2::OBJN_TROLLBRIDGE;
	case MP2::OBJN_WITCHSHUT:	return MP2::OBJN_WITCHSHUT;
	case MP2::OBJN_XANADU:		return MP2::OBJN_XANADU;
	case MP2::OBJN_CAVE:		return MP2::OBJN_CAVE;
	case MP2::OBJN_MAGELLANMAPS:	return MP2::OBJN_MAGELLANMAPS;
	case MP2::OBJN_SHIPWRECK:	return MP2::OBJN_SHIPWRECK;
	case MP2::OBJN_MAGICWELL:	return MP2::OBJN_MAGICWELL;

	case MP2::OBJN_OBSERVATIONTOWER:return MP2::OBJN_OBSERVATIONTOWER;
	case MP2::OBJN_FREEMANFOUNDRY:	return MP2::OBJN_FREEMANFOUNDRY;
	case MP2::OBJ_TREES:		return MP2::OBJ_TREES;
	case MP2::OBJ_MOUNTS:		return MP2::OBJ_MOUNTS;
	case MP2::OBJ_VOLCANO:		return MP2::OBJ_VOLCANO;
	case MP2::OBJ_STONES:		return MP2::OBJ_STONES;
	case MP2::OBJ_FLOWERS:		return MP2::OBJ_FLOWERS;
	case MP2::OBJ_WATERLAKE:	return MP2::OBJ_WATERLAKE;
	case MP2::OBJ_VEGETATION2:	return MP2::OBJ_VEGETATION2;
	case MP2::OBJ_DEADTREE:		return MP2::OBJ_DEADTREE;
	case MP2::OBJ_STUMP:		return MP2::OBJ_STUMP;
	case MP2::OBJ_CRATER:		return MP2::OBJ_CRATER;
	case MP2::OBJ_CACTUS:		return MP2::OBJ_CACTUS;
	case MP2::OBJ_MOUND:		return MP2::OBJ_MOUND;
	case MP2::OBJ_DUNE:		return MP2::OBJ_DUNE;

	case MP2::OBJ_LAVAPOOL:		return MP2::OBJ_LAVAPOOL;
	case MP2::OBJ_SHRUB:		return MP2::OBJ_SHRUB;

	case MP2::OBJ_ALCHEMYTOWER:	return MP2::OBJ_ALCHEMYTOWER;
	case MP2::OBJ_SIGN:		return MP2::OBJ_SIGN;
	case MP2::OBJ_BUOY:		return MP2::OBJ_BUOY;
	case MP2::OBJ_SKELETON:		return MP2::OBJ_SKELETON;
	case MP2::OBJ_DAEMONCAVE:	return MP2::OBJ_DAEMONCAVE;
	case MP2::OBJ_TREASURECHEST:	return MP2::OBJ_TREASURECHEST;
	case MP2::OBJ_FAERIERING:	return MP2::OBJ_FAERIERING;
	case MP2::OBJ_CAMPFIRE:		return MP2::OBJ_CAMPFIRE;
	case MP2::OBJ_FOUNTAIN:		return MP2::OBJ_FOUNTAIN;
	case MP2::OBJ_GAZEBO:		return MP2::OBJ_GAZEBO;
	case MP2::OBJ_ANCIENTLAMP:	return MP2::OBJ_ANCIENTLAMP;
	case MP2::OBJ_GRAVEYARD:	return MP2::OBJ_GRAVEYARD;
	case MP2::OBJ_ARCHERHOUSE:	return MP2::OBJ_ARCHERHOUSE;
	case MP2::OBJ_GOBLINHUNT:	return MP2::OBJ_GOBLINHUNT;
	case MP2::OBJ_DWARFCOTT:	return MP2::OBJ_DWARFCOTT;

	case MP2::OBJ_PEASANTHUNT:	return MP2::OBJ_PEASANTHUNT;
	case MP2::OBJ_EVENT:		return MP2::OBJ_EVENT;
	case MP2::OBJ_DRAGONCITY:	return MP2::OBJ_DRAGONCITY;
	case MP2::OBJ_LIGHTHOUSE:	return MP2::OBJ_LIGHTHOUSE;
	case MP2::OBJ_WATERMILL:	return MP2::OBJ_WATERMILL;
	case MP2::OBJ_MINES:		return MP2::OBJ_MINES;
	case MP2::OBJ_MONSTER:		return MP2::OBJ_MONSTER;
	case MP2::OBJ_OBELISK:		return MP2::OBJ_OBELISK;
	case MP2::OBJ_OASIS:		return MP2::OBJ_OASIS;
	case MP2::OBJ_RESOURCE:		return MP2::OBJ_RESOURCE;
	case MP2::OBJ_SAWMILL:		return MP2::OBJ_SAWMILL;
	case MP2::OBJ_ORACLE:		return MP2::OBJ_ORACLE;
	case MP2::OBJ_SHRINE1:		return MP2::OBJ_SHRINE1;

	case MP2::OBJ_DERELICTSHIP:	return MP2::OBJ_DERELICTSHIP;
	case MP2::OBJ_DESERTTENT:	return MP2::OBJ_DESERTTENT;
	case MP2::OBJ_CASTLE:		return MP2::OBJ_CASTLE;
	case MP2::OBJ_STONELIGHTS:	return MP2::OBJ_STONELIGHTS;
	case MP2::OBJ_WAGONCAMP:	return MP2::OBJ_WAGONCAMP;
	case MP2::OBJ_WHIRLPOOL:	return MP2::OBJ_WHIRLPOOL;
	case MP2::OBJ_WINDMILL:		return MP2::OBJ_WINDMILL;
	case MP2::OBJ_ARTIFACT:		return MP2::OBJ_ARTIFACT;
	case MP2::OBJ_BOAT:		return MP2::OBJ_BOAT;
	case MP2::OBJ_RNDULTIMATEARTIFACT:return MP2::OBJ_RNDULTIMATEARTIFACT;
	case MP2::OBJ_RNDARTIFACT:	return MP2::OBJ_RNDARTIFACT;
	case MP2::OBJ_RNDRESOURCE:	return MP2::OBJ_RNDRESOURCE;
	case MP2::OBJ_RNDMONSTER:	return MP2::OBJ_RNDMONSTER;

	case MP2::OBJ_RNDTOWN:		return MP2::OBJ_RNDTOWN;
	case MP2::OBJ_RNDCASTLE:	return MP2::OBJ_RNDCASTLE;
	case MP2::OBJ_RNDMONSTER1:	return MP2::OBJ_RNDMONSTER1;
	case MP2::OBJ_RNDMONSTER2:	return MP2::OBJ_RNDMONSTER2;
	case MP2::OBJ_RNDMONSTER3:	return MP2::OBJ_RNDMONSTER3;
	case MP2::OBJ_RNDMONSTER4:	return MP2::OBJ_RNDMONSTER4;
	case MP2::OBJ_HEROES:		return MP2::OBJ_HEROES;
	case MP2::OBJ_WATCHTOWER:	return MP2::OBJ_WATCHTOWER;
	case MP2::OBJ_TREEHOUSE:	return MP2::OBJ_TREEHOUSE;
	case MP2::OBJ_TREECITY:		return MP2::OBJ_TREECITY;
	case MP2::OBJ_RUINS:		return MP2::OBJ_RUINS;
	case MP2::OBJ_FORT:		return MP2::OBJ_FORT;
	case MP2::OBJ_TRADINGPOST:	return MP2::OBJ_TRADINGPOST;

	case MP2::OBJ_ABANDONEDMINE:	return MP2::OBJ_ABANDONEDMINE;
	case MP2::OBJ_PEASANTHUNT2:	return MP2::OBJ_PEASANTHUNT2;
	case MP2::OBJ_STANDINGSTONES:	return MP2::OBJ_STANDINGSTONES;
	case MP2::OBJ_IDOL:		return MP2::OBJ_IDOL;
	case MP2::OBJ_TREEKNOWLEDGE:	return MP2::OBJ_TREEKNOWLEDGE;
	case MP2::OBJ_DOCTORHUT:	return MP2::OBJ_DOCTORHUT;
	case MP2::OBJ_TEMPLE:		return MP2::OBJ_TEMPLE;
	case MP2::OBJ_HILLFORT:		return MP2::OBJ_HILLFORT;
	case MP2::OBJ_HALFLINGHOLE:	return MP2::OBJ_HALFLINGHOLE;
	case MP2::OBJ_MERCENARYCAMP:	return MP2::OBJ_MERCENARYCAMP;
	case MP2::OBJ_SHRINE2:		return MP2::OBJ_SHRINE2;
	case MP2::OBJ_SHRINE3:		return MP2::OBJ_SHRINE3;
	case MP2::OBJ_PIRAMID:		return MP2::OBJ_PIRAMID;
	case MP2::OBJ_CITYDEAD:		return MP2::OBJ_CITYDEAD;
	case MP2::OBJ_EXCAVATION:	return MP2::OBJ_EXCAVATION;
	case MP2::OBJ_SPHINX:		return MP2::OBJ_SPHINX;

	case MP2::OBJ_WAGON:		return MP2::OBJ_WAGON;
	case MP2::OBJN_ARTESIANSPRING:	return MP2::OBJN_ARTESIANSPRING;
	case MP2::OBJ_ARTESIANSPRING:	return MP2::OBJ_ARTESIANSPRING;
	case MP2::OBJ_TROLLBRIDGE:	return MP2::OBJ_TROLLBRIDGE;
	case MP2::OBJN_CRAKEDLAKE:	return MP2::OBJN_CRAKEDLAKE;
	case MP2::OBJ_CRAKEDLAKE:	return MP2::OBJ_CRAKEDLAKE;
	case MP2::OBJ_WITCHSHUT:	return MP2::OBJ_WITCHSHUT;
	case MP2::OBJ_XANADU:		return MP2::OBJ_XANADU;
	case MP2::OBJ_CAVE:		return MP2::OBJ_CAVE;
	case MP2::OBJ_LEANTO:		return MP2::OBJ_LEANTO;
	case MP2::OBJ_MAGELLANMAPS:	return MP2::OBJ_MAGELLANMAPS;
	case MP2::OBJ_FLOTSAM:		return MP2::OBJ_FLOTSAM;
	case MP2::OBJ_SHIPWRECK:	return MP2::OBJ_SHIPWRECK;
	case MP2::OBJ_SHIPWRECKSURVIROR:return MP2::OBJ_SHIPWRECKSURVIROR;
	case MP2::OBJ_BOTTLE:		return MP2::OBJ_BOTTLE;
	case MP2::OBJ_MAGICWELL:	return MP2::OBJ_MAGICWELL;
	case MP2::OBJ_MAGICGARDEN:	return MP2::OBJ_MAGICGARDEN;

	case MP2::OBJ_OBSERVATIONTOWER:	return MP2::OBJ_OBSERVATIONTOWER;
	case MP2::OBJ_FREEMANFOUNDRY:	return MP2::OBJ_FREEMANFOUNDRY;

	case MP2::OBJ_RNDARTIFACT1:	return MP2::OBJ_RNDARTIFACT1;
	case MP2::OBJ_RNDARTIFACT2:	return MP2::OBJ_RNDARTIFACT2;
	case MP2::OBJ_RNDARTIFACT3:	return MP2::OBJ_RNDARTIFACT3;


        // loyality version
        case MP2::OBJN_JAIL:		return MP2::OBJN_JAIL;
        case MP2::OBJ_JAIL:		return MP2::OBJ_JAIL;
        case MP2::OBJN_TRAVELLERTENT:	return MP2::OBJN_TRAVELLERTENT;
        case MP2::OBJ_TRAVELLERTENT:	return MP2::OBJ_TRAVELLERTENT;
        case MP2::OBJ_BARRIER:		return MP2::OBJ_BARRIER;
    



	case MP2::OBJ_UNKNW_02:		return MP2::OBJ_UNKNW_02;
	case MP2::OBJ_UNKNW_03:		return MP2::OBJ_UNKNW_03;
	case MP2::OBJ_UNKNW_04:		return MP2::OBJ_UNKNW_04;
	case MP2::OBJ_UNKNW_06:		return MP2::OBJ_UNKNW_06;
	case MP2::OBJ_UNKNW_08:		return MP2::OBJ_UNKNW_08;
	case MP2::OBJ_UNKNW_09:		return MP2::OBJ_UNKNW_09;
	case MP2::OBJ_UNKNW_0B:		return MP2::OBJ_UNKNW_0B;
	case MP2::OBJ_UNKNW_0E:		return MP2::OBJ_UNKNW_0E;
	case MP2::OBJ_UNKNW_11:		return MP2::OBJ_UNKNW_11;
	case MP2::OBJ_UNKNW_12:		return MP2::OBJ_UNKNW_12;
	case MP2::OBJ_UNKNW_13:		return MP2::OBJ_UNKNW_13;
	case MP2::OBJ_UNKNW_18:		return MP2::OBJ_UNKNW_18;
	case MP2::OBJ_UNKNW_1B:		return MP2::OBJ_UNKNW_1B;
	case MP2::OBJ_UNKNW_1F:		return MP2::OBJ_UNKNW_1F;
	case MP2::OBJ_UNKNW_21:		return MP2::OBJ_UNKNW_21;
	case MP2::OBJ_UNKNW_26:		return MP2::OBJ_UNKNW_26;
	case MP2::OBJ_UNKNW_27:		return MP2::OBJ_UNKNW_27;
	case MP2::OBJ_UNKNW_29:		return MP2::OBJ_UNKNW_29;
	case MP2::OBJ_UNKNW_2A:		return MP2::OBJ_UNKNW_2A;
	case MP2::OBJ_UNKNW_2B:		return MP2::OBJ_UNKNW_2B;
	case MP2::OBJ_UNKNW_2C:		return MP2::OBJ_UNKNW_2C;
	case MP2::OBJ_UNKNW_2D:		return MP2::OBJ_UNKNW_2D;
	case MP2::OBJ_UNKNW_2E:		return MP2::OBJ_UNKNW_2E;
	case MP2::OBJ_UNKNW_2F:		return MP2::OBJ_UNKNW_2F;
	case MP2::OBJ_UNKNW_32:		return MP2::OBJ_UNKNW_32;
	case MP2::OBJ_UNKNW_33:		return MP2::OBJ_UNKNW_33;
	case MP2::OBJ_UNKNW_34:		return MP2::OBJ_UNKNW_34;
	case MP2::OBJ_UNKNW_35:		return MP2::OBJ_UNKNW_35;
	case MP2::OBJ_UNKNW_36:		return MP2::OBJ_UNKNW_36;
	case MP2::OBJ_UNKNW_37:		return MP2::OBJ_UNKNW_37;
	case MP2::OBJ_UNKNW_41:		return MP2::OBJ_UNKNW_41;
	case MP2::OBJ_UNKNW_42:		return MP2::OBJ_UNKNW_42;
	case MP2::OBJ_UNKNW_43:		return MP2::OBJ_UNKNW_43;
	case MP2::OBJ_UNKNW_4A:		return MP2::OBJ_UNKNW_4A;
	case MP2::OBJ_UNKNW_4B:		return MP2::OBJ_UNKNW_4B;
	case MP2::OBJ_UNKNW_50:		return MP2::OBJ_UNKNW_50;
	case MP2::OBJ_UNKNW_58:		return MP2::OBJ_UNKNW_58;
	case MP2::OBJ_UNKNW_5A:		return MP2::OBJ_UNKNW_5A;
	case MP2::OBJ_UNKNW_5C:		return MP2::OBJ_UNKNW_5C;
	case MP2::OBJ_UNKNW_5D:		return MP2::OBJ_UNKNW_5D;
	case MP2::OBJ_UNKNW_5F:		return MP2::OBJ_UNKNW_5F;
	case MP2::OBJ_UNKNW_62:		return MP2::OBJ_UNKNW_62;
	case MP2::OBJ_UNKNW_72:		return MP2::OBJ_UNKNW_72;
	case MP2::OBJ_UNKNW_73:		return MP2::OBJ_UNKNW_73;
	case MP2::OBJ_UNKNW_74:		return MP2::OBJ_UNKNW_74;
	case MP2::OBJ_UNKNW_75:		return MP2::OBJ_UNKNW_75;
	case MP2::OBJ_UNKNW_76:		return MP2::OBJ_UNKNW_76;
	case MP2::OBJ_UNKNW_77:		return MP2::OBJ_UNKNW_77;
	case MP2::OBJ_UNKNW_79:		return MP2::OBJ_UNKNW_79;
	case MP2::OBJ_UNKNW_7A:		return MP2::OBJ_UNKNW_7A;
	case MP2::OBJ_UNKNW_7C:		return MP2::OBJ_UNKNW_7C;
	case MP2::OBJ_UNKNW_7D:		return MP2::OBJ_UNKNW_7D;
	case MP2::OBJ_UNKNW_7E:		return MP2::OBJ_UNKNW_7E;
	case MP2::OBJ_UNKNW_7F:		return MP2::OBJ_UNKNW_7F;
	case MP2::OBJ_UNKNW_80:		return MP2::OBJ_UNKNW_80;
	case MP2::OBJ_UNKNW_91:		return MP2::OBJ_UNKNW_91;
	case MP2::OBJ_UNKNW_92:		return MP2::OBJ_UNKNW_92;
	case MP2::OBJ_UNKNW_A1:		return MP2::OBJ_UNKNW_A1;
	case MP2::OBJ_UNKNW_A6:		return MP2::OBJ_UNKNW_A6;
	case MP2::OBJ_UNKNW_AA:		return MP2::OBJ_UNKNW_AA;
	case MP2::OBJ_UNKNW_B2:		return MP2::OBJ_UNKNW_B2;
	case MP2::OBJ_UNKNW_B8:		return MP2::OBJ_UNKNW_B8;
	case MP2::OBJ_UNKNW_B9:		return MP2::OBJ_UNKNW_B9;
	case MP2::OBJ_UNKNW_D1:		return MP2::OBJ_UNKNW_D1;
	case MP2::OBJ_UNKNW_E2:		return MP2::OBJ_UNKNW_E2;
	case MP2::OBJ_UNKNW_E3:		return MP2::OBJ_UNKNW_E3;
	case MP2::OBJ_UNKNW_E4:		return MP2::OBJ_UNKNW_E4;
	case MP2::OBJ_UNKNW_E5:		return MP2::OBJ_UNKNW_E5;
	case MP2::OBJ_UNKNW_E6:		return MP2::OBJ_UNKNW_E6;
	case MP2::OBJ_UNKNW_E7:		return MP2::OBJ_UNKNW_E7;
	case MP2::OBJ_UNKNW_E8:		return MP2::OBJ_UNKNW_E8;
	case MP2::OBJ_UNKNW_E9:		return MP2::OBJ_UNKNW_E9;
	case MP2::OBJ_UNKNW_EA:		return MP2::OBJ_UNKNW_EA;
	case MP2::OBJ_UNKNW_EB:		return MP2::OBJ_UNKNW_EB;
	case MP2::OBJ_UNKNW_EC:		return MP2::OBJ_UNKNW_EC;
	case MP2::OBJ_UNKNW_ED:		return MP2::OBJ_UNKNW_ED;
	case MP2::OBJ_UNKNW_EE:		return MP2::OBJ_UNKNW_EE;
	case MP2::OBJ_UNKNW_EF:		return MP2::OBJ_UNKNW_EF;
	case MP2::OBJ_UNKNW_F0:		return MP2::OBJ_UNKNW_F0;
	case MP2::OBJ_UNKNW_F1:		return MP2::OBJ_UNKNW_F1;
	case MP2::OBJ_UNKNW_F2:		return MP2::OBJ_UNKNW_F2;
	case MP2::OBJ_UNKNW_F3:		return MP2::OBJ_UNKNW_F3;
	case MP2::OBJ_UNKNW_F9:		return MP2::OBJ_UNKNW_F9;
	case MP2::OBJ_UNKNW_FA:		return MP2::OBJ_UNKNW_FA;
	case MP2::OBJ_UNKNW_FC:		return MP2::OBJ_UNKNW_FC;
	case MP2::OBJ_UNKNW_FD:		return MP2::OBJ_UNKNW_FD;
	case MP2::OBJ_UNKNW_FE:		return MP2::OBJ_UNKNW_FE;
	case MP2::OBJ_UNKNW_FF:		return MP2::OBJ_UNKNW_FF;


	default: Error::Warning("Maps::Tiles::GetObject: unknown object: ", general); Error::Warning("maps index: ", maps_index); break;
    }
    
    return MP2::OBJ_ZERO;
}

/* accept move */
bool Maps::Tiles::isPassable(void) const
{
    if(Game::Focus::Get().Type() != Game::Focus::HEROES) return false;

    std::list<TilesAddon>::const_iterator it1 = addons_level1.begin();
    std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

    for(; it1 != it2; ++it1)
    	if((*it1).level == 0 && MP2::OBJ_ZERO != general && MP2::OBJ_COAST != general) return false;

    if(Game::Focus::Get().GetHeroes().isShipMaster())
    {
    	if(Ground::WATER != Maps::Tiles::GetGround()) return false;

        switch(general)
	{
	    case MP2::OBJ_BOAT:
	    case MP2::OBJ_STONES:
	    case MP2::OBJN_MAGELLANMAPS:
	    case MP2::OBJ_DERELICTSHIP:
	    case MP2::OBJN_DERELICTSHIP:
	    case MP2::OBJN_SHIPWRECK:

            case MP2::OBJ_WHIRLPOOL:
            case MP2::OBJ_BUOY:
            case MP2::OBJ_BOTTLE:
    	    case MP2::OBJ_SHIPWRECKSURVIROR:
            case MP2::OBJ_FLOTSAM:
    		return false;
	    
	    default: break;
	}
    }
    else
    {
	if(Ground::WATER == Maps::Tiles::GetGround()) return false;
/*
        switch(general)
	{
	    case MP2::OBJ_COAST:
	    case MP2::OBJ_DUNE:
	    case MP2::OBJ_FLOWERS:		
	    case MP2::OBJ_SHRUB:
	    case MP2::OBJ_SHRUB2:
	    case MP2::OBJ_STUMP:
	    case MP2::OBJ_ZERO:		
		return true;

	    default: return false;
	}
*/
    }

    return true;
}

/* check road */
bool Maps::Tiles::isRoad(const Direction::vector_t & direct) const
{
    switch(direct)
    {
	case Direction::UNKNOWN:
	case Direction::CENTER:	return addons_level1.end() != find_if(addons_level1.begin(), addons_level1.end(), TilesAddon::isRoad);

	default: break;
    }

    if(addons_level1.size())
    {
	std::list<TilesAddon>::const_iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1) if(TilesAddon::isRoad(*it1, direct)) return true;
    }

    return false;
}

Maps::TilesAddon * Maps::Tiles::FindResource(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNRSRC
	    if(0xB7 < addon.object && 0xBC > addon.object && (addon.index % 2)) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindRNDResource(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNRSRC
	    if(0xB7 < addon.object && 0xBC > addon.object && 17 == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindArtifact(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNARTI
	    if(0x2B < addon.object && 0x30 > addon.object && (addon.index % 2)) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindRNDArtifact(const u8 level)
{
    u8 index = 0xA3;

    switch(level)
    {
	case MP2::OBJ_RNDARTIFACT1: index = 0xA7; break;
	case MP2::OBJ_RNDARTIFACT2: index = 0xA9; break;
	case MP2::OBJ_RNDARTIFACT3: index = 0xAB; break;
	default: break;
    }

    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNARTI
	    if(0x2B < addon.object && 0x30 > addon.object && index == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindUltimateArtifact(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNARTI
	    if(0x2B < addon.object && 0x30 > addon.object && 0xA4 == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindMiniHero(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // MINIHERO
	    if(0x53 < addon.object && 0x58 > addon.object) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindEvent(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNMUL2
            if(0xA3 < addon.object && 0xA8 > addon.object && 0xA3 == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindBoat(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNWAT2
            if(0x9F < addon.object && 0xA4 > addon.object && 0x17 == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindCastle(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNTOWN
            if(0x8B < addon.object && 0x90 > addon.object) return &addon;
	}
    }

    if(addons_level2.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level2.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level2.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNTOWN
            if(0x8B < addon.object && 0x90 > addon.object) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindRNDCastle(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNTWRD
            if(0x97 < addon.object && 0x9C > addon.object && 32 > addon.index) return &addon;
	}
    }

    if(addons_level2.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level2.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level2.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNTWRD
            if(0x97 < addon.object && 0x9C > addon.object && 32 > addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindFlags(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // FLAG32
            if(0x37 < addon.object && 0x3C > addon.object) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindRNDMonster(const u8 level)
{
    u8 index = 0x42;

    switch(level)
    {
	case MP2::OBJ_RNDMONSTER1: index = 0x43; break;
	case MP2::OBJ_RNDMONSTER2: index = 0x44; break;
	case MP2::OBJ_RNDMONSTER3: index = 0x45; break;
	case MP2::OBJ_RNDMONSTER4: index = 0x46; break;
	default: break;
    }

    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // MONS32
	    if(0x2F < addon.object && 0x34 > addon.object && index == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindMonster(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // MONS32
	    if(0x2F < addon.object && 0x34 > addon.object) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindBottle(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNWAT
	    if(0xC7 < addon.object && 0xCC > addon.object && 0 == addon.index) return &addon;
	}
    }

    return NULL;
}

Maps::TilesAddon * Maps::Tiles::FindCampFire(void)
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    TilesAddon & addon = *it1;

	    // OBJNDSRT
            if(0x63 < addon.object && 0x68 > addon.object && 61 == addon.index) return &addon;
	    else
	    // OBJNMULT
            if(0xEB < addon.object && 0xF0 > addon.object && 131 == addon.index) return &addon;
	    else
	    // OBJNSNOW
            if(0x57 < addon.object && 0x5C > addon.object && 4 == addon.index) return &addon;
	}
    }

    return NULL;
}

const Maps::TilesAddon * Maps::Tiles::FindMines(void) const
{
    if(addons_level1.size())
    {
	std::list<TilesAddon>::const_iterator it1 = addons_level1.begin();
	std::list<TilesAddon>::const_iterator it2 = addons_level1.end();

	for(; it1 != it2; ++it1)
	{
	    const TilesAddon & addon = *it1;

	    // EXTRAOVR
	    if(0x74 == addon.object) return &addon;
	}
    }

    return NULL;
}

/* ICN::FLAGS32 version */
void Maps::Tiles::CaptureFlags32(const MP2::object_t obj, const Color::color_t col)
{
    u8 index = 0;

    switch(col)
    {
	case Color::BLUE:	index = 0; break;
	case Color::GREEN:	index = 1; break;
	case Color::RED:	index = 2; break;
	case Color::YELLOW:	index = 3; break;
	case Color::ORANGE:	index = 4; break;
	case Color::PURPLE:	index = 5; break;
	case Color::GRAY:	index = 6; break;

	default: break;
    }

    switch(obj)
    {
	case MP2::OBJ_MINES:		index += 14; CorrectFlags32(index); RedrawAll(); break;
	//case MP2::OBJ_DRAGONCITY:	index += 35; CorrectFlags32(index); Redraw(); break; unused
        case MP2::OBJ_LIGHTHOUSE:	index += 42; CorrectFlags32(index); RedrawAll(); break;

	case MP2::OBJ_ALCHEMYTOWER:
	{
	    index += 21;
	    if(Maps::isValidDirection(maps_index, Direction::TOP))
	    {
		Maps::Tiles & tile = world.GetTiles(Maps::GetDirectionIndex(maps_index, Direction::TOP));
		tile.CorrectFlags32(index);
		tile.RedrawAll();
	    }
	}
	break;

	case MP2::OBJ_SAWMILL:
	{
	    index += 28;
	    if(Maps::isValidDirection(maps_index, Direction::TOP_RIGHT))
    	    {
    		Maps::Tiles & tile = world.GetTiles(Maps::GetDirectionIndex(maps_index, Direction::TOP_RIGHT));
    		tile.CorrectFlags32(index);
		tile.RedrawAll();
	    }
	}
	break;

	case MP2::OBJ_CASTLE:
	{
	    index *= 2;

	    if(Maps::isValidDirection(maps_index, Direction::LEFT))
	    {
		Maps::Tiles & tile = world.GetTiles(Maps::GetDirectionIndex(maps_index, Direction::LEFT));
    		tile.CorrectFlags32(index);
		tile.RedrawAll();
	    }

	    if(Maps::isValidDirection(maps_index, Direction::RIGHT))
	    {
		Maps::Tiles & tile = world.GetTiles(Maps::GetDirectionIndex(maps_index, Direction::RIGHT));
    		tile.CorrectFlags32(index);
		tile.RedrawAll();
	    }
	}
	break;

	default: return;
    }
}

/* correct flags, ICN::FLAGS32 vesion */
void Maps::Tiles::CorrectFlags32(const u8 index)
{
    TilesAddon * taddon = FindFlags();

    // replace flag
    if(NULL != taddon)
	taddon->index = index;
    else
    // or new flag
	addons_level2.push_back(TilesAddon(TilesAddon::UPPER, world.GetUniq(), 0x38, index));
}
