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

#include <algorithm>
#include <functional>
#include "agg.h"
#include "settings.h"
#include "payment.h"
#include "world.h"
#include "kingdom.h"
#include "maps_tiles.h"
#include "text.h"
#include "luck.h"
#include "morale.h"
#include "speed.h"
#include "castle.h"
#include "heroes.h"
#include "race.h"
#include "battle_stats.h"
#include "tools.h"
#include "army.h"

const char* Army::String(u32 size)
{
    const char* str_size[] = { _("army|Few"), _("army|Several"), _("army|Pack"), _("army|Lots"), _("army|Horde"), _("army|Throng"), _("army|Swarm"), _("army|Zounds"), _("army|Legion") };

    switch(GetSize(size))
    {
	case FEW:	return str_size[0];
        case SEVERAL:	return str_size[1];
        case PACK:	return str_size[2];
        case LOTS:	return str_size[3];
        case HORDE:	return str_size[4];
        case THRONG:	return str_size[5];
        case SWARM:	return str_size[6];
        case ZOUNDS:	return str_size[7];
        case LEGION:	return str_size[8];
    }

    return str_size[0];
}

Army::armysize_t Army::GetSize(u32 count)
{
    if(LEGION <= count)		return LEGION;
    else
    if(ZOUNDS <= count)		return ZOUNDS;
    else
    if(SWARM <= count)		return SWARM;
    else
    if(THRONG <= count)		return THRONG;
    else
    if(HORDE <= count)		return HORDE;
    else
    if(LOTS <= count)		return LOTS;
    else
    if(PACK <= count)		return PACK;
    else
    if(SEVERAL <= count)	return SEVERAL;

    return FEW;
}

Army::army_t::army_t(HeroBase* s) : army(ARMYMAXTROOPS), commander(s), combat_format(FORMAT_SPREAD), color(Color::GRAY)
{
    for(std::vector<Troop>::iterator
	it = army.begin(); it != army.end(); ++it)
	(*it).army = this;
}

Army::army_t::army_t(const army_t & a) : army(ARMYMAXTROOPS), commander(NULL), combat_format(FORMAT_SPREAD), color(Color::GRAY)
{
    for(std::vector<Troop>::iterator
	it = army.begin(); it != army.end(); ++it)
	(*it).army = this;

    Import(a.army);
}

void Army::army_t::FromGuardian(const Maps::Tiles & t)
{
    Reset();

    if(MP2::isCaptureObject(t.GetObject()))
	color = world.ColorCapturedObject(t.GetIndex());

    u8 obj = t.GetObject();

    if(MP2::OBJ_HEROES == obj)
    {
	const Heroes* hero = world.GetHeroes(t.GetIndex());
	if(hero) obj = hero->GetUnderObject();
    }

    switch(obj)
    {
	case MP2::OBJ_PYRAMID:
            army[0].Set(Monster::ROYAL_MUMMY, 10);
            army[1].Set(Monster::VAMPIRE_LORD, 10);
            army[2].Set(Monster::ROYAL_MUMMY, 10);
            army[3].Set(Monster::VAMPIRE_LORD, 10);
            army[4].Set(Monster::ROYAL_MUMMY, 10);
	    break;

	case MP2::OBJ_GRAVEYARD:
	    army[0].Set(Monster::MUTANT_ZOMBIE, 100);
	    ArrangeForBattle();
	    break;

	case MP2::OBJ_SHIPWRECK:
	    army[0].Set(Monster::GHOST, t.GetQuantity2());
	    ArrangeForBattle();
	    break;

	case MP2::OBJ_DERELICTSHIP:
	    army[0].Set(Monster::SKELETON, 200);
	    ArrangeForBattle();
	    break;

	case MP2::OBJ_ARTIFACT:
	    switch(t.GetQuantity2())
	    {
		case 6:	army[0].Set(Monster::ROGUE, 50); break;	
		case 7:	army[0].Set(Monster::GENIE, 1); break;	
		case 8:	army[0].Set(Monster::PALADIN, 1); break;	
		case 9:	army[0].Set(Monster::CYCLOPS, 1); break;	
		case 10:army[0].Set(Monster::PHOENIX, 1); break;	
		case 11:army[0].Set(Monster::GREEN_DRAGON, 1); break;	
		case 12:army[0].Set(Monster::TITAN, 1); break;	
		case 13:army[0].Set(Monster::BONE_DRAGON, 1); break;
		default: break;	
	    }
	    ArrangeForBattle();
	    break;

	//case MP2::OBJ_ABANDONEDMINE:
	//    army[0] = Troop(t);
	//    ArrangeForBattle();
	//    break;

	case MP2::OBJ_CITYDEAD:
            army[0].Set(Monster::ZOMBIE, 20);
            army[1].Set(Monster::VAMPIRE_LORD, 5);
            army[2].Set(Monster::POWER_LICH, 5);
            army[3].Set(Monster::VAMPIRE_LORD, 5);
            army[4].Set(Monster::ZOMBIE, 20);
	    break;

	case MP2::OBJ_TROLLBRIDGE:
            army[0].Set(Monster::TROLL, 4);
            army[1].Set(Monster::WAR_TROLL, 4);
            army[2].Set(Monster::TROLL, 4);
            army[3].Set(Monster::WAR_TROLL, 4);
            army[4].Set(Monster::TROLL, 4);
	    break;

	case MP2::OBJ_DRAGONCITY:
            army[0].Set(Monster::GREEN_DRAGON, 3);
            army[1].Set(Monster::RED_DRAGON, 2);
            army[2].Set(Monster::BLACK_DRAGON, 1);
	    break;

	case MP2::OBJ_DAEMONCAVE:
            army[0].Set(Monster::EARTH_ELEMENT, 2);
            army[1].Set(Monster::EARTH_ELEMENT, 2);
            army[2].Set(Monster::EARTH_ELEMENT, 2);
            army[3].Set(Monster::EARTH_ELEMENT, 2);
	    break;

	default:
	    army[0] = Troop(t);
	    ArrangeForBattle();
	    break;
    }
}

Army::army_t & Army::army_t::operator= (const army_t & a)
{
    commander = NULL;

    Import(a.army);
    combat_format = a.combat_format;
    color = a.color;

    return *this;
}

void Army::army_t::SetCombatFormat(format_t f)
{
    combat_format = f;
}

u8 Army::army_t::GetCombatFormat(void) const
{
    return combat_format;
}

void Army::army_t::Import(const std::vector<Troop> & v)
{
    for(u8 ii = 0; ii < Size(); ++ii)
    {
	if(ii < v.size())
	    army[ii] = v[ii];
	else
	    army[ii].Reset();
    }
}

void Army::army_t::UpgradeMonsters(const Monster & m)
{
    for(u8 ii = 0; ii < Size(); ++ii) if(army[ii].isValid() && army[ii] == m) army[ii].Upgrade();
}

u8 Army::army_t::Size(void) const
{
    return army.size() > ARMYMAXTROOPS ? army.size() : ARMYMAXTROOPS;
}

Army::Troop & Army::army_t::FirstValid(void)
{
    std::vector<Troop>::iterator it = std::find_if(army.begin(), army.end(), Army::isValidTroop);

    if(it == army.end())
    {
	DEBUG(DBG_GAME, DBG_WARN, "not found" << ", return first..");
	it = army.begin();
    }

    return *it;
}

s8 Army::army_t::GetTroopIndex(const Troop & t) const
{
    for(u8 ii = 0; ii < Size(); ++ii) if(&army[ii] == &t) return ii;
    return -1;
}

Army::Troop & Army::army_t::At(u8 index)
{
    return index < Size() ? army[index] : army[Size() - 1];
}

const Army::Troop & Army::army_t::At(u8 index) const
{
    return index < Size() ? army[index] : army[Size() - 1];
}

u8 Army::army_t::GetCount(void) const
{
    return std::count_if(army.begin(), army.end(), Army::isValidTroop);
}

u8 Army::army_t::GetUniqCount(void) const
{
    std::vector<Monster> troops;
    troops.reserve(ARMYMAXTROOPS);

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()) troops.push_back(*it);
    troops.resize(std::unique(troops.begin(), troops.end()) - troops.begin());

    return troops.size();
}

u32 Army::army_t::GetCountMonsters(const Monster & m) const
{
    u32 c = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid() && (*it) == m) c += (*it).GetCount();

    return c;
}

bool Army::army_t::JoinTroop(const Monster & mon, const u32 count)
{
    return JoinTroop(Troop(mon, count));
}

bool Army::army_t::JoinTroop(const Troop & troop)
{
    if(!troop.isValid()) return false;

    std::vector<Troop>::iterator it = std::find(army.begin(), army.end(), static_cast<Monster>(troop));
    if(it == army.end()) it = std::find_if(army.begin(), army.end(), std::not1(std::mem_fun_ref(&Troop::isValid)));

    if(it != army.end())
    {
	if((*it).isValid())
	    (*it).SetCount((*it).GetCount() + troop.GetCount());
	else
	    (*it).Set(troop, troop.GetCount());
	DEBUG(DBG_GAME, DBG_INFO, "monster: " << troop.GetName() << ", count: " << std::dec << troop.GetCount() << ", commander: " << (commander ? commander->GetName() : "unknown"));
	return true;
    }

    return false;
}

bool Army::army_t::CanJoinTroop(const Monster & mon) const
{
    std::vector<Troop>::const_iterator it = std::find(army.begin(), army.end(), mon);
    if(it == army.end()) it = std::find_if(army.begin(), army.end(), std::not1(std::mem_fun_ref(&Troop::isValid)));

    return it != army.end();
}

bool Army::army_t::CanJoinArmy(const Army::army_t & army2) const
{
    Army::army_t army1(*this);

    for(std::vector<Troop>::const_iterator
	it = army2.army.begin(); it != army2.army.end(); ++it)
	    if((*it).isValid() && ! army1.JoinTroop(*it)) return false;

    return true;
}

bool Army::army_t::JoinArmy(Army::army_t & army2)
{
    for(std::vector<Troop>::iterator
	it = army2.army.begin(); it != army2.army.end(); ++it) if((*it).isValid())
    {
	if(JoinTroop(*it))
	    (*it).Reset();
	else
	    return false;
    }

    return true;
}

bool Army::army_t::isValid(void) const
{
    return army.end() != std::find_if(army.begin(),army.end(), Army::isValidTroop);
}

bool Army::army_t::HasMonster(const Monster & mons) const
{
    return army.end() != std::find(army.begin(), army.end(), mons);
}

Color::color_t Army::army_t::GetColor(void) const
{
    return commander ? commander->GetColor() : color;
}

void Army::army_t::SetColor(Color::color_t cl)
{
    color = cl;
}

u8 Army::army_t::GetRace(void) const
{
    std::vector<u8> races;
    races.reserve(ARMYMAXTROOPS);

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()) races.push_back((*it).GetRace());

    races.resize(std::unique(races.begin(), races.end()) - races.begin());

    if(races.empty())
    {
        DEBUG(DBG_GAME, DBG_WARN, "empty");
        return Race::MULT;
    }

    return 1 < races.size() ? Race::MULT : races.at(0);
}

s8 Army::army_t::GetLuck(void) const
{
    return commander ? commander->GetLuck() : GetLuckModificator(NULL);
}

s8 Army::army_t::GetLuckModificator(std::string *strs) const
{
    return Luck::NORMAL;
}

bool Army::army_t::AllTroopsIsRace(u8 race) const
{
    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid() && (*it).GetRace() != race) return false;

    return true;
}

s8 Army::army_t::GetMorale(void) const
{
    return commander ? commander->GetMorale() : GetMoraleModificator(NULL);
}

// TODO:: need optimize
s8 Army::army_t::GetMoraleModificator(std::string *strs) const
{
    s8 result(Morale::NORMAL);

    // different race penalty
    u8 count = 0;
    u8 count_kngt = 0;
    u8 count_barb = 0;
    u8 count_sorc = 0;
    u8 count_wrlk = 0;
    u8 count_wzrd = 0;
    u8 count_necr = 0;
    u8 count_bomg = 0;
    bool ghost_present = false;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it) if((*it).isValid())
    {
        switch((*it).GetRace())
	{
            case Race::KNGT: ++count_kngt; break;
            case Race::BARB: ++count_barb; break;
            case Race::SORC: ++count_sorc; break;
            case Race::WRLK: ++count_wrlk; break;
            case Race::WZRD: ++count_wzrd; break;
            case Race::NECR: ++count_necr; break;
            case Race::BOMG: ++count_bomg; break;
            default: break;
	}
        if(*it == Monster::GHOST) ghost_present = true;
    }

    u8 r = Race::MULT;
    if(count_kngt){ ++count; r = Race::KNGT; }
    if(count_barb){ ++count; r = Race::BARB; }
    if(count_sorc){ ++count; r = Race::SORC; }
    if(count_wrlk){ ++count; r = Race::WRLK; }
    if(count_wzrd){ ++count; r = Race::WZRD; }
    if(count_necr){ ++count; r = Race::NECR; }
    if(count_bomg){ ++count; r = Race::BOMG; }
    const u8 uniq_count = GetUniqCount();

    switch(count)
    {
        case 2:
        case 0: break;
        case 1:
    	    if(0 == count_necr && !ghost_present)
            {
		if(1 < uniq_count)
                {
		    ++result;
            	    if(strs)
            	    {
            		std::string str = _("All %{race} troops +1");
            		String::Replace(str, "%{race}", Race::String(r));
            		strs->append(str);
            		strs->append("\n");
            	    }
		}
            }
	    else
            {
	        if(strs)
                {
            	    strs->append(_("Entire unit is undead, so morale does not apply."));
            	    strs->append("\n");
            	}
		return 0;
	    }
            break;
        case 3:
            result -= 1;
            if(strs)
            {
        	strs->append(_("Troops of 3 alignments -1"));
        	strs->append("\n");
    	    }
            break;
        case 4:
    	    result -= 2;
            if(strs)
            {
        	strs->append(_("Troops of 4 alignments -2"));
        	strs->append("\n");
    	    }
            break;
        default:
            result -= 3;
            if(strs)
            {
        	strs->append(_("Troops of 5 alignments -3"));
        	strs->append("\n");
    	    }
            break;
    }

    // undead in life group
    if((1 < uniq_count && (count_necr || ghost_present) && (count_kngt || count_barb || count_sorc || count_wrlk || count_wzrd || count_bomg)) ||
    // or artifact Arm Martyr
	(commander && commander->HasArtifact(Artifact::ARM_MARTYR)))
    {
        result -= 1;
        if(strs)
        {
    	    strs->append(_("Some undead in groups -1"));
    	    strs->append("\n");
    	}
    }

    return result;
}

Army::Troop & Army::army_t::GetWeakestTroop(void)
{
    std::vector<Troop>::iterator first, last, lowest;

    first = army.begin();
    last  = army.end();

    while(first != last) if(isValidTroop(*first)) break; else ++first;

    lowest = first;

    if(first != last)
    while(++first != last) if(isValidTroop(*first) && WeakestTroop(*first, *lowest)) lowest = first;

    return *lowest;
}

const Army::Troop & Army::army_t::GetSlowestTroop(void) const
{
    std::vector<Troop>::const_iterator first, last, lowest;

    first = army.begin();
    last  = army.end();

    while(first != last) if(isValidTroop(*first)) break; else ++first;

    lowest = first;

    if(first != last)
    while(++first != last) if(isValidTroop(*first) && SlowestTroop(*first, *lowest)) lowest = first;

    return *lowest;
}

/* draw MONS32 sprite in line, first valid = 0, count = 0 */
void Army::army_t::DrawMons32Line(s16 cx, s16 cy, u16 width, u8 first, u8 count, bool hide) const
{
    if(!isValid()) return;

    if(0 == count) count = GetCount();
    else
    if(Size() < count) count = Size();

    const u16 chunk = width / count;
    cx += chunk / 2;

    std::string str;
    Text text;
    text.Set(Font::SMALL);

    for(u8 ii = 0; ii < Size(); ++ii)
    {
	const Army::Troop & troop = army[ii];

    	if(troop.isValid())
	{
	    if(0 == first && count)
    	    {
		const Sprite & monster = AGG::GetICN(ICN::MONS32, troop.GetSpriteIndex());

    		Display::Get().Blit(monster, cx - monster.w() / 2, cy + 30 - monster.h());

    		if(hide)
		{
		    text.Set(Army::String(troop.GetCount()));
		}
		else
		{
    		    str.clear();
		    String::AddInt(str, troop.GetCount());
		    text.Set(str);
		}
		text.Blit(cx - text.w() / 2, cy + 28);

		cx += chunk;
		--count;
	    }
	    else
		--first;
	}
    }
}

std::vector<Army::Troop> Army::army_t::Optimize(void) const
{
    std::vector<Troop> optimize;
    optimize.reserve(ARMYMAXTROOPS);

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it) if((*it).isValid())
    {
	const Troop & troop = *it;

	std::vector<Troop>::iterator it2 = std::find(optimize.begin(), optimize.end(),
								static_cast<Monster>(troop));
	if(it2 == optimize.end())
	    optimize.push_back(troop);
	else
	    (*it2).SetCount((*it2).GetCount() + troop.GetCount());
    }
    return optimize;
}

void Army::army_t::ArrangeForBattle(void)
{
    if(GetControl() != Game::AI) return;

    std::vector<Troop> priority = Optimize();

    Clear();

    switch(priority.size())
    {
	case 1:
	{
	    const Monster & m = priority.back();
	    const u32 count = priority.back().GetCount();
	    if(49 < count)
	    {
		const u32 c = count / 5;
		army[0].Set(m, c);
		army[1].Set(m, c);
		army[2].Set(m, c + count - (c * 5));
		army[3].Set(m, c);
		army[4].Set(m, c);
	    }
	    else
	    if(20 < count)
	    {
		const u32 c = count / 3;
		army[1].Set(m, c);
		army[2].Set(m, c + count - (c * 3));
		army[3].Set(m, c);
	    }
	    else
		army[2].Set(m, count);
	    break;
	}
	case 2:
	{
	    // TODO: need modify army for 2 troops
	    Import(priority);
	    break;
	}
	case 3:
	{
	    // TODO: need modify army for 3 troops
	    Import(priority);
	    break;
	}
	case 4:
	{
	    // TODO: need modify army for 4 troops
	    Import(priority);
	    break;
	}
	case 5:
	{    // possible change orders monster
	    // store
	    Import(priority);
	    break;
	}
	default: break;
    }
}

u32 Army::army_t::CalculateExperience(void) const
{
    u32 res = 0;

    for(u8 ii = 0; ii < Size(); ++ii)
	res += army[ii].GetHitPoints();

    if(commander) res += 500;

    return res;

}

void Army::army_t::BattleInit(void)
{
    // reserve ARMYMAXTROOPS for summons and mirrors spell
    if(commander)
    {
	army.reserve(ARMYMAXTROOPS * 2);
	commander->ActionPreBattle();
    }
}

void Army::army_t::BattleQuit(void)
{
    std::for_each(army.begin(), army.end(), std::mem_fun_ref(&Troop::BattleQuit));
    if(army.size() > ARMYMAXTROOPS) army.resize(ARMYMAXTROOPS);
    if(commander)
	commander->ActionAfterBattle();
}

u32 Army::army_t::BattleKilled(void) const
{
    u32 res = 0;
    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	res += (*it).BattleKilled();

    return res;
}

void Army::army_t::Clear(void)
{
    std::for_each(army.begin(), army.end(), std::mem_fun_ref(&Troop::Reset));
}

void Army::army_t::BattleExportKilled(army_t & a) const
{
    a.army.resize(Size());

    for(u8 ii = 0; ii < Size(); ++ii)
    {
	a.army[ii].SetMonster(army[ii].GetID());
	a.army[ii].SetCount(army[ii].BattleKilled());
    }

    a.commander = commander;
}

void Army::army_t::Reset(bool soft)
{
    Clear();

    if(commander)
    {
    	const Monster mons1(commander->GetRace(), DWELLING_MONSTER1);

	if(soft)
	{
    	    const Monster mons2(commander->GetRace(), DWELLING_MONSTER2);

	    switch(Rand::Get(1, 3))
	    {
		case 1:
		    JoinTroop(mons1, 3 * mons1.GetGrown());
		    break;
		case 2:
		    JoinTroop(mons2, static_cast<u8>(1.5 * mons2.GetGrown()));
		    break;
		default:
		    JoinTroop(mons1, 2 * mons1.GetGrown());
		    JoinTroop(mons2, mons2.GetGrown());
		    break;
	    }
	}
	else
	{
	    JoinTroop(mons1, 1);
	}
    }
}

void Army::army_t::JoinStrongestFromArmy(army_t & army2)
{
    bool save_last = army2.commander && Skill::Primary::HEROES == army2.commander->GetType();

    std::vector<Troop> priority;
    priority.reserve(ARMYMAXTROOPS * 2);
    priority = Optimize();
    std::vector<Troop> priority2 = army2.Optimize();
    priority.insert(priority.end(), priority2.begin(), priority2.end());

    Clear();
    army2.Clear();

    // sort: strongest
    std::sort(priority.begin(), priority.end(), StrongestTroop);

    // weakest to army2
    while(Size() < priority.size())
    {
	army2.JoinTroop(priority.back());
	priority.pop_back();
    }

    // save half weak of strongest to army2
    if(save_last && !army2.isValid())
    {
	Troop & last = priority.back();
	u32 count = last.GetCount() / 2;
	army2.JoinTroop(last, last.GetCount() - count);
	last.SetCount(count);
    }

    // strongest to army
    while(priority.size())
    {
	JoinTroop(priority.back());
	priority.pop_back();
    }
}

void Army::army_t::KeepOnlyWeakestTroops(army_t & army2)
{

    bool save_last = commander && Skill::Primary::HEROES == commander->GetType();

    std::vector<Troop> priority;
    priority.reserve(ARMYMAXTROOPS * 2);
    priority = Optimize();
    std::vector<Troop> priority2 = army2.Optimize();
    priority.insert(priority.end(), priority2.begin(), priority2.end());

    Clear();
    army2.Clear();

    // sort: strongest
    std::sort(priority.begin(), priority.end(), StrongestTroop);

    // weakest to army
    while(Size() < priority.size())
    {
	JoinTroop(priority.back());
	priority.pop_back();
    }

    // save half weak of strongest to army
    if(save_last && !isValid())
    {
	Troop & last = priority.back();
	u32 count = last.GetCount() / 2;
	JoinTroop(last, last.GetCount() - count);
	last.SetCount(count);
    }

    // strongest to army2
    while(priority.size())
    {
	army2.JoinTroop(priority.back());
	priority.pop_back();
    }
}

void Army::army_t::UpgradeTroops(const Castle & castle)
{
    for(std::vector<Troop>::iterator
	it = army.begin(); it != army.end(); ++it) if((*it).isValid())
    {
	Troop & troop = *it;
        payment_t payment = troop.GetUpgradeCost();

	if(castle.GetRace() == troop.GetRace() &&
	   castle.isBuild(troop.GetUpgrade().GetDwelling()) &&
	   payment <= world.GetKingdom(castle.GetColor()).GetFunds())
	{
    	    world.GetKingdom(castle.GetColor()).OddFundsResource(payment);
            troop.Upgrade();
	}
    }
}

void Army::army_t::Dump(const char* prefix) const
{
    if(prefix)
    {
	VERBOSN(prefix);
    }
    else
    {
	VERBOSN("Army::Dump: " <<
	    "color(" << Color::String(commander ? commander->GetColor() : color) << ")");

	if(commander)
	    VERBOSN(", commander(" << commander->GetName() << ")");

	VERBOSN(" :");
    }

    for(std::vector<Troop>::const_iterator
	    it = army.begin(); it != army.end(); ++it)
	if((*it).isValid())
	    VERBOSN((*it).GetName() << "(" << std::dec << (*it).GetCount() << "), ");

    VERBOSE("");
}

u16 Army::army_t::GetAttack(bool hero) const
{
    u16 res = 0;
    u8 count = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()){ res += (*it).GetAttack(hero); ++count; }

    return count ? res / count : 0;
}

u16 Army::army_t::GetDefense(bool hero) const
{
    u16 res = 0;
    u8 count = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()){ res += (*it).GetDefense(hero); ++count; }

    return count ? res / count : 0;
}

u32 Army::army_t::GetHitPoints(void) const
{
    u32 res = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()) res += (*it).GetHitPoints();

    return res;
}

u32 Army::army_t::GetDamageMin(void) const
{
    u32 res = 0;
    u8 count = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()){ res += (*it).GetDamageMin(); ++count; }

    return count ? res / count : 0;
}

u32 Army::army_t::GetDamageMax(void) const
{
    u32 res = 0;
    u8 count = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()){ res += (*it).GetDamageMax(); ++count; }

    return count ? res / count : 0;
}

u32 Army::army_t::GetStrength(void) const
{
    u32 res = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
	if((*it).isValid()) res += (*it).GetStrength();

    return res;
}

bool Army::army_t::StrongerEnemyArmy(const army_t & army2) const
{
    if(! army2.isValid()) return true;

    const u16 a1 = GetAttack(false);
    const u16 d1 = GetDefense(false);
    double r1 = 0;

    const u16 a2 = army2.GetAttack(false);
    const u16 d2 = army2.GetDefense(false);
    double r2 = 0;

    if(a1 > d2)
        r1 = 1 + 0.1 * static_cast<double>(std::min(a1 - d2, 20));
    else
        r1 = 1 + 0.05 * static_cast<double>(std::min(d2 - a1, 14));

    if(a2 > d1)
        r2 = 1 + 0.1 * static_cast<double>(std::min(a2 - d1, 20));
    else
        r2 = 1 + 0.05 * static_cast<double>(std::min(d1 - a2, 14));

    const u32 s1 = GetStrength();
    const u32 s2 = army2.GetStrength();

    const double h1 = GetHitPoints();
    const double h2 = army2.GetHitPoints();

    DEBUG(DBG_AI, DBG_INFO, "r1: " << r1 << ", s1: " << s1 << ", h1: " << h1 \
			<< ", r2: " << r2 << ", s2: " << s2 << ", h2: " << h2);

    r1 *= s1 / h2;
    r2 *= s2 / h1;

    if(IS_DEBUG(DBG_GAME, DBG_INFO))
    {
	Dump();
	army2.Dump();
	DEBUG(DBG_GAME, DBG_INFO, "army1: " << r1 << ", army2: " << r2);
    }

    return 0 == r2 || 1 <= (r1 / r2);
}

void Army::army_t::SetCommander(HeroBase* c)
{
    commander = c;
}

HeroBase* Army::army_t::GetCommander(void)
{
    return (!commander || (Skill::Primary::CAPTAIN == commander->GetType() && !commander->isValid()) ? NULL : commander);
}

const HeroBase* Army::army_t::GetCommander(void) const
{
    return (!commander || (Skill::Primary::CAPTAIN == commander->GetType() && !commander->isValid()) ? NULL : commander);
}

u32 Army::army_t::ActionToSirens(void)
{
    u32 res = 0;

    for(std::vector<Troop>::iterator
	it = army.begin(); it != army.end(); ++it)
    if((*it).isValid())
    {
	const u32 kill = (*it).GetCount() * 30 / 100;

	if(kill)
	{
	    (*it).SetCount((*it).GetCount() - kill);
	    res += kill * static_cast<Monster>(*it).GetHitPoints();
	}
    }

    return res;
}

u8 Army::army_t::GetControl(void) const
{
    return commander ? commander->GetControl() : (color == Color::GRAY ? Game::AI : world.GetKingdom(color).Control());
}

u32 Army::army_t::GetSurrenderCost(void) const
{
    double res = 0;

    for(std::vector<Troop>::const_iterator
	it = army.begin(); it != army.end(); ++it)
    if((*it).isValid())
    {
	// FIXME: orig: 3 titan = 7500
	payment_t payment = (*it).GetCost();
	res += payment.gold;
    }

    if(commander)
    {
	switch(commander->GetLevelSkill(Skill::Secondary::DIPLOMACY))
	{
	    // 40%
	    case Skill::Level::BASIC: res = res * 40 / 100; break;
	    // 30%
	    case Skill::Level::ADVANCED: res = res * 30 / 100; break;
	    // 20%
	    case Skill::Level::EXPERT: res = res * 20 / 100; break;
	    // 50%
	    default: res = res * 50 / 100; break;
	}

	Artifact art(Artifact::STATESMAN_QUILL);
	if(commander->HasArtifact(art))
	    res -= res * art.ExtraValue() / 100;
    }

    // limit
    if(res < 100) res = 100.0;

    return static_cast<u32>(res);
}

u8 Army::GetJoinSolution(const Heroes & hero, const Maps::Tiles & tile, u32 & join, s32 & cost)
{
    const Army::Troop troop(tile);

    if(! troop.isValid()) return 0xFF;

    const float ratios = troop.isValid() ? hero.GetArmy().GetHitPoints() / troop.GetHitPoints() : 0;
    const bool check_free_stack = (hero.GetArmy().GetCount() < hero.GetArmy().Size() || hero.GetArmy().HasMonster(troop));
    const bool check_extra_condition = (!hero.HasArtifact(Artifact::HIDEOUS_MASK) && Morale::NORMAL <= hero.GetMorale());

    // force join for campain and others...
    const bool force_join = (5 == tile.GetQuantity4());

    if(tile.GetQuantity4() && check_free_stack && ((check_extra_condition && ratios >= 2) || force_join))
    {
        if(2 == tile.GetQuantity4() || force_join)
        {
	    join = troop.GetCount();
	    return 1;
	}
	else
        if(hero.HasSecondarySkill(Skill::Secondary::DIPLOMACY))
        {
            const Kingdom & kingdom = world.GetKingdom(hero.GetColor());
            payment_t payment = troop.GetCost();
            cost = payment.gold;
	    payment.Reset();
	    payment.gold = cost;

            // skill diplomacy
            const u32 to_join = Monster::GetCountFromHitPoints(troop,
			    troop.GetHitPoints() * hero.GetSecondaryValues(Skill::Secondary::DIPLOMACY) / 100);

            if(to_join && kingdom.AllowPayment(payment))
            {
		join = to_join;
		return 2;
	    }
	}
    }
    else
    if(ratios >= 5)
    {
	// ... surely flee before us

	if(hero.GetControl() == Game::AI) return Rand::Get(0, 10) < 5 ? 0 : 3;

	return 3;
    }

    return 0;
}
