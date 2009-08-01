/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file ai/formula/ai.hpp
 * Defines formula ai
 * */


#ifndef AI_FORMULA_AI_HPP_INCLUDED
#define AI_FORMULA_AI_HPP_INCLUDED

#include "callable_objects.hpp"
#include "candidates.hpp"
#include "function_table.hpp"

#include "../default/ai.hpp"
#include "../../formula.hpp"
#include "../../formula_callable.hpp"


#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace game_logic {

typedef std::pair< unit_map::unit_iterator, int> unit_formula_pair;

struct unit_formula_compare {
        bool operator() (const unit_formula_pair left,
                        const unit_formula_pair right) const
        {
                return left.second > right.second;
        }
};

typedef std::multiset< unit_formula_pair, game_logic::unit_formula_compare > unit_formula_set;

}

class formula_ai : public ai::ai_default {
public:
	explicit formula_ai(ai::default_ai_context &context, const config &cfg);
	virtual ~formula_ai() {};
	virtual void play_turn();
	virtual void new_turn();
	virtual std::string describe_self();
	virtual config to_config() const;

	typedef ai::move_map move_map;

	const move_map& srcdst() const { if(!move_maps_valid_) { prepare_move(); } return srcdst_; }

	std::string evaluate(const std::string& formula_str);

	//class responsible for looking for possible infinite loops when calling set_var or set_unit_var
	class gamestate_change_observer : public events::observer
	{
		static const int MAX_CALLS = 1000;
		int set_var_counter_;
		int set_unit_var_counter_;
		int continue_counter_;
	public:
		gamestate_change_observer();
		~gamestate_change_observer();

		void handle_generic_event(const std::string& /*event_name*/);

		bool set_var_check();

		bool set_unit_var_check();

		bool continue_check();
	};

	typedef game_logic::position_callable::move_map_backup move_map_backup;

	void swap_move_map(move_map_backup& backup);

	variant get_keeps() const;

	void on_create();

	int get_recursion_count() const;

	const variant& get_keeps_cache() const { return keeps_cache_; }

	// Check if given unit can reach another unit
	bool can_reach_unit(map_location unit_A, map_location unit_B) const;

	const std::map<map_location,paths>& get_possible_moves() const { prepare_move(); return possible_moves_; }

	void handle_exception(game_logic::formula_error& e) const;
	void handle_exception(game_logic::formula_error& e, const std::string& failed_operation) const;

        std::set<map_location> get_allowed_teleports(unit_map::iterator& unit_it) const;
	plain_route shortest_path_calculator(const map_location& src, const map_location& dst, unit_map::iterator& unit_it, std::set<map_location>& allowed_teleports) const;

	void invalidate_move_maps() const { move_maps_valid_ = false; }

        void store_outcome_position(const variant& var);

	/** Create a new formula from the string, using the symbol table which is stored in the AI.
	*
	*   @param formula_string the string from which a formula should be created
	*   @return pointer to created function or
	*   @retval game_logic::formula_ptr() if there were any problems
	*/
	game_logic::formula_ptr create_optional_formula(const std::string& formula_string);

	game_logic::candidate_action_ptr load_candidate_action_from_config(const config& cfg);

	/** Evaluate the fai candidate action */
	void evaluate_candidate_action(game_logic::candidate_action_ptr fai_ca);

	/**
	 * Execute the fai candidate action
	 * @return true if game state was changed
	 * @return false if game state was not changed
	 */
	bool execute_candidate_action(game_logic::candidate_action_ptr fai_ca);

private:
	const config &cfg_;
	ai::recursion_counter recursion_counter_;
	void display_message(const std::string& msg) const;
	bool do_recruitment();
	variant make_action(game_logic::const_formula_ptr formula_, const game_logic::formula_callable& variables);
	variant execute_variant(const variant& var, bool commandline=false);
	virtual variant get_value(const std::string& key) const;
	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	game_logic::const_formula_ptr recruit_formula_;
	game_logic::const_formula_ptr move_formula_;

	std::vector<variant> outcome_positions_;

	mutable std::map<map_location,paths> possible_moves_;

	void prepare_move() const;

        map_location path_calculator(const map_location& src, const map_location& dst, unit_map::iterator& unit_it) const;
	mutable bool move_maps_valid_;
	mutable move_map srcdst_, dstsrc_, full_srcdst_, full_dstsrc_, enemy_srcdst_, enemy_dstsrc_;
	mutable variant attacks_cache_;
	mutable variant keeps_cache_;

	gamestate_change_observer infinite_loop_guardian_;
	game_logic::map_formula_callable vars_;
	game_logic::ai_function_symbol_table function_table;
	game_logic::candidate_action_manager candidate_action_manager_;

	friend class ai::ai_default;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
