/*
 Copyright (C) 2013 by L.Sebilleau <l.sebilleau@free.fr>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/*
 Yet Another Map Generator.

 This class is the main generator class. Once instanced and configured, it can create a random map and can produce a buffer which BfW can use.
 Each generator owns a single map, but multiple instances can be used. Generator can be reused too.

 Public functions are used in this order in the standalone tool:
 - Constructor
 - Configuration creation and modification.
 - SetUp : install and check configuration.
 - CreateMap : create the map in memory.
 - GetMap : write the map data to a buffer.
 - ResetMap : free the old map and reset the generator to default values.

 Provisions have been made to make this class a child of map_generator, but the implementation is not finished yet.
 */

#ifndef YA_MAPGEN_HPP
#define YA_MAPGEN_HPP

#include "yamg_params.hpp"
#include "yamg_hex.hpp"
#include "yamg_hexheap.hpp"
#include "config.hpp"

#ifndef YAMG_STANDALONE
#include "generators/mapgen.hpp"
#endif

/**
 This structure is used to store one hex and it's neighbors
 */
typedef struct _voisins {
	yamg_hex *center;
	yamg_hex *no;
	yamg_hex *nw;
	yamg_hex *sw;
	yamg_hex *so;
	yamg_hex *se;
	yamg_hex *ne;
} voisins, *pVoisins;

/**
 This structure is used to store the burgs
 */
typedef struct _burg {
	_burg *next;
	yamg_hex *center;
	yamg_hex *road1;
	yamg_hex *road2;
} burg, *pBurg;

//============================== Class definition =========================

#ifdef YAMG_STANDALONE
class ya_mapgen
#else
class ya_mapgen: public map_generator
#endif
{
public:
	ya_mapgen(const config& cfg);
	ya_mapgen();
	virtual ~ya_mapgen();

	//----------- Inherited methods from map_generator -----------------

//#ifndef YAMG_STANDALONE
	bool allow_user_config() const;
	void user_config(display& disp);
	std::string name() const; // {return "yamg";};
	std::string config_name() const; // {return "generator";};
	std::string create_map(const std::vector<std::string>& args);
//#endif

//----------------- Methods -------------

	unsigned int setUp(yamg_params *); ///< uses parameter list object to configure
	int createMap();           ///< do the job, return OK if everything is OK
	int getMap(char *buf);     ///< write the map to a buffer
	void resetMap();           ///< reset the generator to new use

protected:
	unsigned int status_;    ///< this is the state of the generator.
	unsigned int siz_; ///< the 'real' size of the map (used allocation and computing altitudes) 2^n + 1
	yamg_params *parms_;     ///< its parameter list.
	yamg_hex ***map_;        ///< the generated map
	yamg_hex *summits_;      ///< a list of hexes, various purposes
	yamg_hex *endpoints_;    ///< something to store a list of road startpoints
	yamg_hex *castles_;      ///< a list of castle keeps

	int table_[M_NUMLEVEL];  ///< array defining layers boundaries.
	int sLim_;               ///< the snow limit floor.
	unsigned int riv_;       ///< the reference water level
	const char *terrains_[M_NUMLEVEL]; ///< terrains to use for each layer (overloaded with snow)
	yamg_hexheap *heap_;          ///< an utility heap to sort hexes

	//----------------- Functions -------------

	unsigned int createEmptyMap(); ///< creates an empty map according parameters height and width
	unsigned int freeMap();         ///< frees all memory used

	void createAltitudes(unsigned int x, unsigned int xm, unsigned int y,
			unsigned int ym, unsigned int rough); ///< compute hexes altitudes
	int normalizeMap();             ///< normalize altitudes
	void setBaseTerrains(int range);    ///< set hexes base terrains
	void customTerrains();          ///< terrain customization

	void makeRivers();              ///< create river and lakes
	int calcWContribs(yamg_hex *h); ///< calculate rain flowing
	void erodeTerrains(yamg_hex *h, int report); ///< erode terrains to make rivers

	void makeBurgs();       ///< creates the burgs (some agglomerated villages)
	void makeCastles();     ///< creates the castles
	void makeForests();     ///< set forests overlays
	void makeHouses();      ///< creates some houses (villages)
	void makeRoads();       ///< creates roads

	void getVoisins(yamg_hex *h, pVoisins p);               ///< get neighbours of some hex
	int fillWith(const char *over[], yamg_hex *h, int num); ///< utility to fill overlays
	void clearDoneFlag();                                   ///< reset done flag on all hexes
	yamg_hex *selNeigh(yamg_hex *it);                       ///< lists the available hexes for roads
	void storeVoisins(yamg_hex *it, unsigned int layMin, unsigned int layMax); ///< get neighbours and store them in the heap

private:
};

int mRand(int limit);                 ///< returns a random number 0 < n < limit
void init_Rand(unsigned int seed);    ///< init random number generator

#ifdef INTERN_RAND
void init_genrand(unsigned long s);   ///< embedded RNG initialization
unsigned long genrand(void);          ///< RNG production
#endif

#endif // YA_MAPGEN_HPP
