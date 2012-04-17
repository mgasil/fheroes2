/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
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

#include <cstdlib>
#include <map>
#include "maps.h"
#include "world.h"
#include "direction.h"
#include "settings.h"
#include "heroes.h"
#include "route.h"

struct cell_t
{
#ifdef WITH_DEBUG
    cell_t() : cost_g(MAXU16), cost_t(MAXU16), cost_d(MAXU16), passbl(0), open(1), parent(-1){};

    u16		cost_g;	// ground
#else
    cell_t() : cost_t(MAXU16), cost_d(MAXU16), passbl(0), open(1), parent(-1){};
#endif
    u16		cost_t; // total
    u16		cost_d; // distance
    u16		passbl;
    u16		open;
    s32		parent;
};


u32 GetCurrentLength(std::map<s32, cell_t> & list, s32 cur)
{
    u32 res = 0;
    const cell_t* cell = &list[cur];
    while(-1 != cell->parent){ cell = &list[cell->parent]; ++res; };
    return res;
}

bool CheckMonsterProtectionAndNotDst(const s32 & to, const s32 & dst)
{
    const MapsIndexes & monsters = Maps::GetTilesUnderProtection(to);
    return monsters.size() && monsters.end() == std::find(monsters.begin(), monsters.end(), dst);
}

bool PassableToTile(const Heroes* hero, const Maps::Tiles & toTile, const Direction::vector_t & direct, const s32 & dst)
{
    // check end point
    if(toTile.GetIndex() == dst)
    {
	// fix toTilePassable with action object
	if(hero && MP2::isPickupObject(toTile.GetObject()))
	    return true;

	// check direct to object
	if(MP2::isActionObject(toTile.GetObject(false), (hero ? hero->isShipMaster() : false)))
	    return Direction::Reflect(direct) & toTile.GetPassable();

	if(MP2::OBJ_HEROES == toTile.GetObject())
	    return toTile.isPassable(NULL, Direction::Reflect(direct), false);
    }

    // check to tile direct
    if(! toTile.isPassable(hero, Direction::Reflect(direct), false))
	return false;

    if(toTile.GetIndex() != dst)
    {
	if(MP2::isPickupObject(toTile.GetObject()) ||
	    MP2::isActionObject(toTile.GetObject(false), (hero ? hero->isShipMaster() : false)))
	    return false;

	// check hero/monster on route
	switch(toTile.GetObject())
	{
	    case MP2::OBJ_HEROES:
	    case MP2::OBJ_MONSTER:
		return false;

	    default: break;
	}

	// check monster protection
	if(CheckMonsterProtectionAndNotDst(toTile.GetIndex(), dst))
	    return false;
    }

    return true;
}

bool PassableFromToTile(const Heroes* hero, const s32 & from, const s32 & to, const Direction::vector_t & direct, const s32 & dst)
{
    const Maps::Tiles & fromTile = world.GetTiles(from);
    const Maps::Tiles & toTile = world.GetTiles(to);

    // check start point
    if(hero && hero->GetIndex() == from)
    {
	if(MP2::isActionObject(fromTile.GetObject(false), hero->isShipMaster()))
	{
	    // check direct from object
	    if(! (direct & fromTile.GetPassable()))
		return false;
	}
	else
	{
	    // check from tile direct
	    if(! fromTile.isPassable(hero, direct, false))
		return false;
	}
    }
    else
    {
	if(MP2::isActionObject(fromTile.GetObject(), (hero ? hero->isShipMaster() : false)))
	{
	    // check direct from object
	    if(! (direct & fromTile.GetPassable()))
		return false;
	}
	else
	{
	    // check from tile direct
	    if(! fromTile.isPassable(hero, direct, false))
		return false;
	}
    }

    if(fromTile.isWater() && !toTile.isWater())
    {
	switch(toTile.GetObject())
	{
	    case MP2::OBJ_BOAT:
            case MP2::OBJ_MONSTER:
            case MP2::OBJ_HEROES:
                return false;

	    case MP2::OBJ_COAST:
		return toTile.GetIndex() == dst;

	    default: break;
	}
    }
    else
    if(!fromTile.isWater() && toTile.isWater())
    {
	switch(toTile.GetObject())
	{
	    case MP2::OBJ_BOAT:
                return true;

            case MP2::OBJ_HEROES:
		return toTile.GetIndex() == dst;

	    default: break;
	}
    }

    return PassableToTile(hero, toTile, direct, dst);
}

u16 GetPenaltyFromTo(const s32 & from, const s32 & to, const Direction::vector_t & direct, const u8 & pathfinding)
{
    const u16 & cost1 = Maps::Ground::GetPenalty(from, direct, pathfinding); // penalty: for [cur] out
    const u16 & cost2 = Maps::Ground::GetPenalty(to, Direction::Reflect(direct), pathfinding); // penalty: for [tmp] in
    return (cost1 + cost2) >> 1;
}

bool Route::PathFind(std::list<Route::Step>* result, const s32 from, const s32 to, const u16 limit, const Heroes* hero)
{
    const u8 pathfinding = (hero ? hero->GetLevelSkill(Skill::Secondary::PATHFINDING) : Skill::Level::NONE);

    s32 cur = from;
    s32 alt = 0;
    s32 tmp = 0;
    std::map<s32, cell_t> list;
    std::map<s32, cell_t>::iterator it1 = list.begin();
    std::map<s32, cell_t>::iterator it2 = list.end();
    Direction::vector_t direct = Direction::CENTER;

#ifdef WITH_DEBUG
    list[cur].cost_g = 0;
#endif
    list[cur].cost_t = 0;
    list[cur].parent = -1;
    list[cur].open   = 0;

    while(cur != to)
    {
	LocalEvent::Get().HandleEvents(false);

	for(direct = Direction::TOP_LEFT; direct != Direction::CENTER; ++direct)
	{
    	    if(Maps::isValidDirection(cur, direct))
	    {
		tmp = Maps::GetDirectionIndex(cur, direct);

		if(list[tmp].open)
		{
		    const u16 costg = GetPenaltyFromTo(cur, tmp, direct, pathfinding);

		    // new
		    if(-1 == list[tmp].parent)
		    {
			if((list[cur].passbl & direct) ||
			   PassableFromToTile(hero, cur, tmp, direct, to))
			{
			    list[cur].passbl |= direct;

#ifdef WITH_DEBUG
	    		    list[tmp].cost_g = costg;
#endif
			    list[tmp].parent = cur;
			    list[tmp].open   = 1;
			    list[tmp].cost_d = 50 * Maps::GetApproximateDistance(tmp, to);
	    		    list[tmp].cost_t = list[cur].cost_t + costg;
			}
		    }
		    // check alt
		    else
		    {
			if(list[tmp].cost_t > list[cur].cost_t + costg &&
			   ((list[cur].passbl & direct) || PassableFromToTile(hero, cur, tmp, direct, to)))
			{
			    list[cur].passbl |= direct;

			    list[tmp].parent = cur;
#ifdef WITH_DEBUG
			    list[tmp].cost_g = costg;
#endif
			    list[tmp].cost_t = list[cur].cost_t + costg;
			}
		    }
    		}
	    }
	}

	list[cur].open = 0;

	it1 = list.begin();
	alt = -1;
	tmp = MAXU16;

	DEBUG(DBG_OTHER, DBG_TRACE, "route, from: " << cur);

	// find minimal cost
	for(; it1 != it2; ++it1) if((*it1).second.open)
	{
	    const cell_t & cell2 = (*it1).second;
#ifdef WITH_DEBUG
	    if(IS_DEBUG(DBG_OTHER, DBG_TRACE) && cell2.cost_g != MAXU16)
	    {
		direct = Direction::Get(cur, (*it1).first);
		if(Direction::UNKNOWN != direct)
		{
		    VERBOSE("\t\tdirect: " << Direction::String(direct) <<
			    ", index: " << (*it1).first <<
			    ", cost g: " << cell2.cost_g <<
			    ", cost t: " << cell2.cost_t <<
			    ", cost d: " << cell2.cost_d);
		}
	    }
#endif

	    if(cell2.cost_t + cell2.cost_d < tmp)
	    {
    		tmp = cell2.cost_t + cell2.cost_d;
    		alt = (*it1).first;
	    }
	}

	// not found, and exception
	if(MAXU16 == tmp || -1 == alt || (limit && GetCurrentLength(list, cur) > limit)) break;
#ifdef WITH_DEBUG
	else
	DEBUG(DBG_OTHER, DBG_TRACE, "select: " << alt);
#endif
	cur = alt;
    }

    // save path
    if(cur == to)
    {
	while(cur != from)
	{
	    if(-1 == list[cur].parent) break;
	    alt = cur;
    	    cur = list[alt].parent;
	    const Direction::vector_t direct = Direction::Get(cur, alt);
	    if(result) result->push_front(Route::Step(cur, direct, GetPenaltyFromTo(cur, alt, direct, pathfinding)));
	}
        return true;
    }

    DEBUG(DBG_OTHER, DBG_TRACE, "not found" << ", from:" << from << ", to: " << to);
    list.clear();

    return false;
}
