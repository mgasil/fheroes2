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
#include <cstring>
#include <algorithm>
#include <fstream>
#include "xmlccwrap.h"
#include "maps_fileinfo.h"

#define LENGTHNAME		16
#define LENGTHDESCRIPTION	143

Race::race_t ByteToRace(u8 byte)
{
    switch(byte)
    {
	case 0x00:	return Race::KNGT;
	case 0x01:	return Race::BARB;
	case 0x02:	return Race::SORC;
	case 0x03:	return Race::WRLK;
	case 0x04:	return Race::WZRD;
	case 0x05:	return Race::NECR;
	case 0x06:	return Race::MULT;
	case 0x07:	return Race::RAND;

	default: 	break;
    }

    return Race::BOMG;
}

Maps::FileInfo::FileInfo() : difficulty(Difficulty::EASY),
    kingdom_colors(0), allow_colors(0), rnd_colors(0), localtime(0), with_heroes(false)
{
    for(u8 ii = 0; ii < KINGDOMMAX; ++ii) races[ii] = Race::BOMG;
}

const std::string & Maps::FileInfo::FileMaps(void) const
{
    return file;
}

const std::string & Maps::FileInfo::Name(void) const
{
    return name;
}

const std::string & Maps::FileInfo::Description(void) const
{
    return description;
}

const time_t & Maps::FileInfo::Time(void) const
{
    return localtime;
}

const Size & Maps::FileInfo::SizeMaps(void) const
{
    return size;
}

Difficulty::difficulty_t Maps::FileInfo::Difficulty(void) const
{
    return difficulty;
}

u8 Maps::FileInfo::KingdomColors(void) const
{
    return kingdom_colors;
}

u8 Maps::FileInfo::AllowColors(void) const
{
    return allow_colors;
}

u8 Maps::FileInfo::ConditionsWins(void) const
{
    return conditions_wins;
}

u8 Maps::FileInfo::ConditionsLoss(void) const
{
    return conditions_loss;
}

Race::race_t Maps::FileInfo::KingdomRace(Color::color_t color) const
{
    switch(color)
    {
        case Color::BLUE:	return races[0];
        case Color::GREEN:	return races[1];
        case Color::RED:	return races[2];
        case Color::YELLOW:	return races[3];
        case Color::ORANGE:	return races[4];
        case Color::PURPLE:	return races[5];

        default: break;
    }

    return Race::BOMG;
}

void Maps::FileInfo::SetKingdomRace(Color::color_t color, Race::race_t race)
{
    switch(color)
    {
        case Color::BLUE:	races[0] = race; break;
        case Color::GREEN:	races[1] = race; break;
        case Color::RED:	races[2] = race; break;
        case Color::YELLOW:	races[3] = race; break;
        case Color::ORANGE:	races[4] = race; break;
        case Color::PURPLE:	races[5] = race; break;

        default: break;
    }
}

void Maps::FileInfo::SetKingdomColors(const u8 colors)
{
    kingdom_colors = colors;
}

bool Maps::FileInfo::PlayWithHeroes(void) const
{
    return with_heroes;
}

bool Maps::FileInfo::Read(const std::string &filename)
{
    if(ReadBIN(filename)) return true;
    else
    if(ReadXML(filename)) return true;

    return false;
}

bool Maps::FileInfo::ReadXML(const std::string &filename)
{
    TiXmlDocument doc;

    // prepare small xml in to memory (for fast loading)
    std::fstream fd(filename.c_str(), std::ios::in | std::ios::binary);
    if(! fd || fd.fail())
    {
        Error::Warning("Maps::FileInfo::ReadXML: " + filename +", file not found.");
        return false;
    }

    std::vector<char> buf(1024, 0);
    fd.read(&buf[0], buf.size() - 1);
    fd.close();

    const char* pred1 = "<game";
    const char* pred2 = "</fheroes2>";
    std::vector<char>::iterator it = std::search(buf.begin(), buf.end(), pred1, pred1 + std::strlen(pred1));

    if(it == buf.end())
    {
        Error::Verbose("Maps::FileInfo::ReadXML: tag not found, broken file " + filename);
        return false;
    }

    buf.resize(it - buf.begin() + std::strlen(pred2) + 1);
    std::copy(pred2, pred2 + std::strlen(pred2) + 1, it);

    // parse block
    doc.Parse(&buf[0]);

    if(doc.Error())
    {
        Error::Verbose("Maps::FileInfo::ReadXML: parse error");
        return false;
    }

    TiXmlElement* root = doc.FirstChildElement();

    if(!root || std::strcmp("fheroes2", root->Value()))
    {
        Error::Verbose("Maps::FileInfo::ReadXML: 1 broken file " + filename);
        return false;
    }

    TiXmlElement *node;
    TiXmlElement *maps = root->FirstChildElement("maps");

    if(!maps)
    {
        Error::Verbose("Maps::FileInfo::ReadXML: 2 broken file " + filename);
        return false;
    }

    int res;

    // fheroes2 version
    //str = root->Attribute("version");
    // fheroes2 build
    //root->Attribute("build", &res);

    // locatime
    root->Attribute("time", &res);
    localtime = res;

    // maps
    maps->Attribute("width", &res);
    size.w = res;
    maps->Attribute("height", &res);
    size.h = res;
    //
    file = filename;
    //
    node = maps->FirstChildElement("name");
    if(node) name = node->GetText();
    //
    node = maps->FirstChildElement("description");
    if(node) description = node->GetText();
    //
    node = maps->FirstChildElement("races");
    if(node)
    {
        node->Attribute("blue", &res);
        races[0] = ByteToRace(res);
        node->Attribute("green", &res);
        races[1] = ByteToRace(res);
        node->Attribute("red", &res);
        races[2] = ByteToRace(res);
        node->Attribute("yellow", &res);
        races[3] = ByteToRace(res);
        node->Attribute("orange", &res);
        races[4] = ByteToRace(res);
        node->Attribute("purple", &res);
        races[5] = ByteToRace(res);
    }
    //
    maps->Attribute("difficulty", &res);
    difficulty = Difficulty::Get(res);
    maps->Attribute("kingdom_colors", &res);
    kingdom_colors = res;
    maps->Attribute("allow_colors", &res);
    allow_colors = res;
    maps->Attribute("rnd_colors", &res);
    rnd_colors = res;
    maps->Attribute("conditions_wins", &res);
    conditions_wins = res;
    maps->Attribute("wins1", &res);
    wins1 = res;
    maps->Attribute("wins2", &res);
    wins2 = res;
    maps->Attribute("wins3", &res);
    wins3 = res;
    maps->Attribute("wins4", &res);
    wins4 = res;
    maps->Attribute("conditions_loss", &res);
    conditions_loss = res;
    maps->Attribute("loss1", &res);
    loss1 = res;
    maps->Attribute("loss2", &res);
    loss2 = res;

    return true;
}

bool Maps::FileInfo::ReadBIN(const std::string & filename)
{
    std::fstream fd(filename.c_str(), std::ios::in | std::ios::binary);

    if(! fd || fd.fail())
    {
	Error::Warning("Maps::FileInfo: " + filename +", file not found.");
	return false;
    }

    file = filename;
    kingdom_colors = 0;
    allow_colors = 0;
    rnd_colors = 0;
    localtime = 0;

    u8  byte8;
    u16 byte16;
    u32 byte32;

    // magic byte
    fd.read(reinterpret_cast<char *>(&byte32), sizeof(byte32));
    SwapLE32(byte32);

    if(byte32 != 0x0000005C)
    {
	Error::Warning("Maps::FileInfo: " + filename +", incorrect maps file.");
	fd.close();
	return false;
    }

    // level
    fd.read(reinterpret_cast<char *>(&byte16), sizeof(byte16));
    SwapLE16(byte16);

    switch(byte16)
    {
	case 0x00:
	    difficulty = Difficulty::EASY;
	    break;

	case 0x01:
	    difficulty = Difficulty::NORMAL;
	    break;

	case 0x02:
	    difficulty = Difficulty::HARD;
	    break;

	case 0x03:
	    difficulty = Difficulty::EXPERT;
	    break;

	default:
	    Error::Warning("Maps::FileInfo: incorrect difficulty maps: " + filename + ". Load EASY default.");
	break;
    }

    // width
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    size.w = byte8;

    // height
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    size.h = byte8;

    // kingdom color blue
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::BLUE;

    // kingdom color green
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::GREEN;

    // kingdom color red
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::RED;

    // kingdom color yellow
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::YELLOW;

    // kingdom color orange
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::ORANGE;

    // kingdom color purple
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::PURPLE;

    // allow color blue
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) allow_colors |= Color::BLUE;

    // allow color green
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) allow_colors |= Color::GREEN;

    // allow color red
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) allow_colors |= Color::RED;

    // allow color yellow
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) allow_colors |= Color::YELLOW;

    // allow color orange
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) allow_colors |= Color::ORANGE;

    // allow color purple
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) allow_colors |= Color::PURPLE;

    // rnd color blue
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) rnd_colors |= Color::BLUE;

    // rnd color green
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) rnd_colors |= Color::GREEN;

    // rnd color red
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) rnd_colors |= Color::RED;

    // rnd color yellow
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) rnd_colors |= Color::YELLOW;

    // rnd color orange
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) rnd_colors |= Color::ORANGE;

    // rnd color purple
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) rnd_colors |= Color::PURPLE;

    // kingdom count
    // fd.seekg(0x1A, std::ios_base::beg);
    // fd.read(&byte8, 1);

    // wins
    fd.seekg(0x1D, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    conditions_wins = byte8;

    // data wins
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    wins1 = byte8;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    wins2 = byte8;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    wins3 = byte8;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    wins4 = byte8;

    // loss
    fd.seekg(0x22, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    conditions_loss = byte8;

    // data loss
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    loss1 = byte8;

    // data loss
    fd.seekg(0x2e, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    loss2 = byte8;

    // start with hero
    fd.seekg(0x25, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    with_heroes = 0 == byte8;

    // race color
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[0] = ByteToRace(byte8);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[1] = ByteToRace(byte8);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[2] = ByteToRace(byte8);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[3] = ByteToRace(byte8);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[4] = ByteToRace(byte8);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[5] = ByteToRace(byte8);

    // name
    char bufname[LENGTHNAME];
    fd.seekg(0x3A, std::ios_base::beg);
    fd.read(bufname, LENGTHNAME);
    bufname[LENGTHNAME - 1] = 0;
    name = bufname;

    // description
    char bufdescription[LENGTHDESCRIPTION];
    fd.seekg(0x76, std::ios_base::beg);
    fd.read(bufdescription, LENGTHDESCRIPTION);
    bufdescription[LENGTHDESCRIPTION - 1] = 0;
    description = bufdescription;

    fd.close();
    
    return true;
}

bool Maps::FileInfo::PredicateForSorting(const FileInfo & fi1, const FileInfo & fi2)
{
    if(fi1.name.empty() || fi2.name.empty()) return false;

    return std::tolower(fi1.name[0]) < std::tolower(fi2.name[0]);
}

u8 Maps::FileInfo::Wins1(void) const
{
    return wins1;
}

u8 Maps::FileInfo::Wins2(void) const
{
    return wins2;
}

u8 Maps::FileInfo::Wins3(void) const
{
    return wins3;
}

u8 Maps::FileInfo::Wins4(void) const
{
    return wins4;
}

u8 Maps::FileInfo::Loss1(void) const
{
    return loss1;
}

u8 Maps::FileInfo::Loss2(void) const
{
    return loss2;
}
