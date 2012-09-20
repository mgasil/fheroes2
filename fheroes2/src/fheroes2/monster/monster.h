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

#ifndef H2MONSTER_H
#define H2MONSTER_H

#include <string>
#include "icn.h"
#include "m82.h"
#include "payment.h"
#include "gamedefs.h"

class Spell;

class Monster
{
public:
    enum { JOIN_CONDITION_SKIP  = 0, JOIN_CONDITION_MONEY = 1, JOIN_CONDITION_FREE  = 2, JOIN_CONDITION_FORCE = 3 };

    enum level_t { LEVEL0, LEVEL1, LEVEL2, LEVEL3, LEVEL4 };

    enum monster_t
    {
	UNKNOWN,

	PEASANT,
	ARCHER,
	RANGER,
	PIKEMAN,
	VETERAN_PIKEMAN,
	SWORDSMAN,
	MASTER_SWORDSMAN,
	CAVALRY,
	CHAMPION,
	PALADIN,
	CRUSADER,
	GOBLIN,
	ORC,
	ORC_CHIEF,
	WOLF,
	OGRE,
	OGRE_LORD,
	TROLL,
	WAR_TROLL,
	CYCLOPS,
	SPRITE,
	DWARF,
	BATTLE_DWARF,
	ELF,
	GRAND_ELF,
	DRUID,
	GREATER_DRUID,
	UNICORN,
	PHOENIX,
	CENTAUR,
	GARGOYLE,
	GRIFFIN,
	MINOTAUR,
	MINOTAUR_KING,
	HYDRA,
	GREEN_DRAGON,
	RED_DRAGON,
	BLACK_DRAGON,
	HALFLING,
	BOAR,
	IRON_GOLEM,
	STEEL_GOLEM,
	ROC,
	MAGE,
	ARCHMAGE,
	GIANT,
	TITAN,
	SKELETON,
	ZOMBIE,
	MUTANT_ZOMBIE,
	MUMMY,
	ROYAL_MUMMY,
	VAMPIRE,
	VAMPIRE_LORD,
	LICH,
	POWER_LICH,
	BONE_DRAGON,

	ROGUE,
	NOMAD,
	GHOST,
	GENIE,
	MEDUSA,
	EARTH_ELEMENT,
	AIR_ELEMENT,
	FIRE_ELEMENT,
	WATER_ELEMENT,

	MONSTER_RND1,
	MONSTER_RND2,
	MONSTER_RND3,
	MONSTER_RND4,
	MONSTER_RND
    };

    Monster(u8 = UNKNOWN);
    Monster(const Spell &);
    Monster(u8 race, u32 dw);
    virtual ~Monster(){}

    bool operator< (const Monster &) const;
    bool operator== (const Monster &) const;
    bool operator!= (const Monster &) const;

    u8 operator() (void) const;
    u8 GetID(void) const;

    void Upgrade(void);
    Monster GetUpgrade(void) const;
    Monster GetDowngrade(void) const;

    virtual u16 GetAttack(void) const;
    virtual u16 GetDefense(void) const;
    virtual u8 GetColor(void) const;
    virtual s8 GetMorale(void) const;
    virtual s8 GetLuck(void) const;
    virtual u8 GetRace(void) const;

    u8  GetDamageMin(void) const;
    u8  GetDamageMax(void) const;
    u8  GetShots(void) const;
    u16 GetHitPoints(void) const;
    u8  GetSpeed(void) const;
    u8  GetGrown(void) const;
    u8  GetLevel(void) const;
    u16 GetRNDSize(bool skip) const;

    const char* GetName(void) const;
    const char* GetMultiName(void) const;
    const char* GetPluralName(u32) const;

    bool isValid(void) const;
    bool isElemental(void) const;
    bool isUndead(void) const;
    bool isFly(void) const;
    bool isWide(void) const;
    bool isArchers(void) const;
    bool isAllowUpgrade(void) const;
    bool isTwiceAttack(void) const;
    bool isResurectLife(void) const;
    bool isDoubleCellAttack(void) const;
    bool isMultiCellAttack(void) const;
    bool isAlwayResponse(void) const;
    bool isHideAttack(void) const;
    bool isDragons(void) const;
    bool isAffectedByMorale(void) const;
    bool isAlive(void) const;

    ICN::icn_t ICNMonh(void) const;

    u8		GetSpriteIndex(void) const;
    payment_t	GetCost(void) const;
    payment_t	GetUpgradeCost(void) const;
    u32		GetDwelling(void) const;

    static Monster Rand(level_t = LEVEL0);
    static u8 Rand4WeekOf(void);
    static u8 Rand4MonthOf(void);

    static u32 GetCountFromHitPoints(const Monster &, u32);

    static void UpdateStats(const std::string &);
    static float GetUpgradeRatio(void);

protected:
    static Monster FromDwelling(u8 race, u32 dw);

    u8 id;
};

struct MonsterStaticData
{
    // wrapper for stream
    static MonsterStaticData & Get(void);
};

StreamBase & operator<< (StreamBase &, const MonsterStaticData &);
StreamBase & operator>> (StreamBase &, MonsterStaticData &);

#endif
