/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef UNIT_TYPES_H_INCLUDED
#define UNIT_TYPES_H_INCLUDED

#include "unit_animation.hpp"
#include "config.hpp"
#include "color_range.hpp"
#include "map.hpp"
#include "race.hpp"
#include "util.hpp"

#include <string>
#include <vector>


class unit_type;
class unit;
class game_data;
class gamestatus;
class team;
typedef std::map<gamemap::location,unit> unit_map;
class weapon_special_list
{
	public:
		
		bool empty() const;
		
		int highest(const std::string& key, int def=0) const;
		int lowest(const std::string& key, int def=100) const;
		
		config::child_list cfgs;
	private:
		
};
//the 'attack type' is the type of attack, how many times it strikes,
//and how much damage it does.
class attack_type
{
public:
	enum RANGE { SHORT_RANGE, LONG_RANGE };

	attack_type(const config& cfg, const std::string& id, const std::string& image_fighting);
	const t_string& name() const;
	const std::string& id() const;
	const std::string& type() const;
	const std::string& icon() const;
	RANGE range_type() const;
	const std::string& range() const;
	int damage() const;
	int num_attacks() const;
	double attack_weight() const;
	double defense_weight() const;
	
	bool get_special_bool(const std::string& special,bool force=false) const;
	weapon_special_list get_specials(const std::string& special) const;
	std::vector<std::string> special_tooltips() const;
	std::string weapon_specials(bool force=false) const;
	void set_specials_context(const gamemap::location& aloc,const gamemap::location& dloc,
                              const game_data* gamedata, unit_map* unitmap, 
							  const gamemap* map, const gamestatus* game_status, 
							  const std::vector<team>* teams,bool attacker,attack_type* other_attack);
	void set_specials_context(const gamemap::location& loc,const unit& un);
	//this function returns a random animation out of the possible
	//animations for this attack. It will not return the same attack
	//each time.
	bool matches_filter(const config& cfg,int set_,bool self=false) const;
	bool apply_modification(const config& cfg,std::string* description,int set_);
	
	int movement_used() const;
	
	const config& get_cfg() const;
	gamemap::location aloc_,dloc_;
	bool attacker_;
	const game_data* gamedata_;
	mutable unit_map* unitmap_; 
	const gamemap* map_;
	const gamestatus* game_status_;
	const std::vector<team>* teams_;
	mutable attack_type* other_attack_;
	/*
	 * cfg: a weapon special WML structure
	 */
	bool special_active(const config& cfg,bool self) const;
	bool special_affects_opponent(const config& cfg) const;
	bool special_affects_self(const config& cfg) const;
	
	struct attack_animation
	{
		typedef enum { HIT, MISS, HIT_OR_MISS } hit_type;

		explicit attack_animation(const config& cfg);
		explicit attack_animation(const std::string &image,const hit_type for_hit= HIT_OR_MISS):animation(image,-200,100),hits(for_hit) {};
		int matches(bool hit,gamemap::location::DIRECTION dir=gamemap::location::NDIRECTIONS) const;

		std::vector<gamemap::location::DIRECTION> directions;
		unit_animation animation;
		unit_animation missile_animation;
		hit_type hits;

	};
	config cfg_;
	const attack_animation& animation(bool hit,gamemap::location::DIRECTION dir=gamemap::location::NDIRECTIONS) const;
private:
	std::vector<attack_animation> animation_;
	t_string description_;
	std::string id_;
	std::string type_;
	std::string icon_;
	RANGE range_type_;
	std::string range_;
	int damage_;
	int num_attacks_;
	double attack_weight_;
	double defense_weight_;

};

class unit_movement_type;

//the 'unit movement type' is the basic size of the unit - flying, small land,
//large land, etc etc.
class unit_movement_type
{
public:
	//this class assumes that the passed in reference will remain valid
	//for at least as long as the class instance
	unit_movement_type(const config& cfg, const unit_movement_type* parent=NULL);

	const t_string& name() const;
	int movement_cost(const gamemap& map, gamemap::TERRAIN terrain, int recurse_count=0) const;
	int defense_modifier(const gamemap& map, gamemap::TERRAIN terrain, int recurse_count=0) const;
	int damage_against(const attack_type& attack) const;
	int resistance_against(const attack_type& attack) const;

	string_map damage_table() const;

	void set_parent(const unit_movement_type* parent);

	bool is_flying() const;
	const std::map<gamemap::TERRAIN,int>& movement_costs() const;
	const std::map<gamemap::TERRAIN,int>& defense_mods() const;

private:
	const config cfg_;

	mutable std::map<gamemap::TERRAIN,int> moveCosts_;
	mutable std::map<gamemap::TERRAIN,int> defenseMods_;

	const unit_movement_type* parent_;
};

typedef std::map<std::string,unit_movement_type> movement_type_map;

class ability_filter
{
public:
	ability_filter();
	bool matches_filter(const std::string& terrain, int lawful_bonus) const;
	void add_filters(const config* cfg);
	void add_terrain_filter(const std::string& terrains);
	void add_tod_filter(const std::string& times);
	void unfilter();
private:
	std::vector<std::string> terrain_filter_chaotic;
	std::vector<std::string> terrain_filter_neutral;
	std::vector<std::string> terrain_filter_lawful;
};

class unit_type
{
public:
	friend class unit;
	unit_type(const config& cfg, const movement_type_map& movement_types,
	          const race_map& races, const std::vector<config*>& traits);
	unit_type(const unit_type& o);

	~unit_type();

    // adds an additional advancement path to a unit type
    // this is used to implement the [advancefrom] tag
    void add_advancement(const unit_type &advance_to,int experience);


	const unit_type& get_gender_unit_type(unit_race::GENDER gender) const;
	const unit_type& get_variation(const std::string& name) const;
    //info on the type of unit that the unit reanimates as
    const std::string& undead_variation() const;

	int num_traits() const;

	std::string generate_description() const;

	//the name of the unit in the current language setting
	const t_string& language_name() const;

	const std::string& id() const;
	//Disabling this one for consistency with other similar structures,
	//where name() is always the user-visible, translated, name.
	//language_name should eventually be renamed name()
	// const std::string& name() const;
	const std::string& image() const;
	const std::string& image_ellipse() const;
	const std::string& image_halo() const;
	const std::string& image_profile() const;
	const std::string& image_fighting(attack_type::RANGE range) const;
	const std::string& image_leading() const;
	const std::string& image_healing() const;
	const std::string& image_halo_healing() const;
	const std::string& unit_description() const;
	const std::string& get_hit_sound() const;
	const std::string& die_sound() const;

    const std::vector<Uint32>& flag_rgb() const;

	int hitpoints() const;
	std::vector<attack_type> attacks() const;
	const unit_movement_type& movement_type() const;

	int experience_needed() const;
	std::vector<std::string> advances_to() const;
	const config::child_list& modification_advancements() const;
	const std::string& usage() const;

	struct experience_accelerator {
		experience_accelerator(int modifier);
		~experience_accelerator();
	private:
		int old_value_;
	};

	int level() const;
	int movement() const;
	int cost() const;

	enum ALIGNMENT { LAWFUL, NEUTRAL, CHAOTIC };

	ALIGNMENT alignment() const;
	static const char* alignment_description(ALIGNMENT align);
	static const char* alignment_id(ALIGNMENT align);

	fixed_t alpha() const;

	const std::vector<std::string>& abilities() const;
	const std::vector<std::string>& ability_tooltips() const;

	bool not_living() const;
	bool can_advance() const;

	bool has_zoc() const;

	bool has_ability(const std::string& ability) const;

	const std::vector<config*>& possible_traits() const;

	const std::vector<unit_race::GENDER>& genders() const;

	const std::string& race() const;

	const defensive_animation& defend_animation(bool hits, std::string range) const;
	const unit_animation& teleport_animation() const;
	const unit_animation* extra_animation(std::string flag) const;
	const death_animation& die_animation(const attack_type* attack) const;
	const movement_animation& move_animation(const std::string terrain,gamemap::location::DIRECTION) const;

private:
	void operator=(const unit_type& o);

	unit_type* gender_types_[2];

	typedef std::map<std::string,unit_type*> variations_map;
	variations_map variations_;

	config cfg_;

	const unit_race* race_;

	fixed_t alpha_;

	std::vector<std::string> abilities_;
	std::vector<std::string> ability_tooltips_;

	mutable std::string id_;

	bool zoc_;

    std::vector<std::string> advances_to_;
    int experience_needed_;
	

	ALIGNMENT alignment_;

	unit_movement_type movementType_;

	std::vector<config*> possibleTraits_;

	std::vector<unit_race::GENDER> genders_;

	std::vector<defensive_animation> defensive_animations_;

	std::vector<unit_animation> teleport_animations_;

	std::multimap<std::string,unit_animation> extra_animations_;

	std::vector<death_animation> death_animations_;

	std::vector<movement_animation> movement_animations_;

    std::vector<Uint32> flag_rgb_;
};

struct game_data
{
	game_data();
	game_data(const config& cfg);
	void set_config(const config& cfg);
	void clear();

	movement_type_map movement_types;
	typedef std::map<std::string,unit_type> unit_type_map;
	unit_type_map unit_types;
	race_map races;
};

#endif
