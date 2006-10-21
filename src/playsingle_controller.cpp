/*
   Copyright (C) 2006 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "playsingle_controller.hpp"

#include "ai_interface.hpp"
#include "game_errors.hpp"
#include "gettext.hpp"
#include "intro.hpp"
#include "marked-up_text.hpp"
#include "sound.hpp"

#define LOG_NG LOG_STREAM(info, engine)

LEVEL_RESULT playsingle_scenario(const game_data& gameinfo, const config& game_config,
		const config* level, CVideo& video, game_state& state_of_game,
		const std::vector<config*>& story, upload_log& log, bool skip_replay)
{
	const int ticks = SDL_GetTicks();
	const int num_turns = atoi((*level)["turns"].c_str());
	LOG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
	playsingle_controller playcontroller(*level, gameinfo, state_of_game, ticks, num_turns, game_config, video, skip_replay);
	LOG_NG << "created objects... " << (SDL_GetTicks() - playcontroller.get_ticks()) << "\n";

	return playcontroller.play_scenario(story, log, skip_replay);
}

playsingle_controller::playsingle_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
											 const int ticks, const int num_turns, const config& game_config, CVideo& video,
											 bool skip_replay)
	: play_controller(level, gameinfo, state_of_game, ticks, num_turns, game_config, video, skip_replay),
	cursor_setter(cursor::NORMAL), replay_sender_(recorder)
{
	end_turn_ = false;
	replaying_ = false;
}

void playsingle_controller::init_gui(){
	LOG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks_) << "\n";
	play_controller::init_gui();

	if(first_human_team_ != -1) {
		gui_->scroll_to_tile(map_.starting_position(first_human_team_ + 1).x,
			               map_.starting_position(first_human_team_ + 1).y, display::WARP);
	}
	gui_->scroll_to_tile(map_.starting_position(1).x,map_.starting_position(1).y,display::WARP);

	update_locker lock_display(gui_->video(),recorder.is_skipping());
	events::raise_draw_event();
	gui_->draw();
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		::clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,(t-teams_.begin()));
	}
}

void playsingle_controller::recruit(){
	if (!browse_)
		menu_handler_.recruit(browse_, player_number_, mouse_handler_.get_last_hex());
}

void playsingle_controller::repeat_recruit(){
	if (!browse_)
		menu_handler_.repeat_recruit(player_number_, mouse_handler_.get_last_hex());
}

void playsingle_controller::recall(){
	if (!browse_)
		menu_handler_.recall(player_number_, mouse_handler_.get_last_hex());
}

void playsingle_controller::toggle_shroud_updates(){
	menu_handler_.toggle_shroud_updates(player_number_);
}

void playsingle_controller::update_shroud_now(){
	menu_handler_.update_shroud_now(player_number_);
}

void playsingle_controller::end_turn(){
	if (!browse_){
		end_turn_ = menu_handler_.end_turn(player_number_);
	}
}

void playsingle_controller::rename_unit(){
	menu_handler_.rename_unit(mouse_handler_);
}

void playsingle_controller::create_unit(){
	menu_handler_.create_unit(mouse_handler_);
}

void playsingle_controller::change_unit_side(){
	menu_handler_.change_unit_side(mouse_handler_);
}

void playsingle_controller::label_terrain(){
	menu_handler_.label_terrain(mouse_handler_);
}

void playsingle_controller::continue_move(){
	menu_handler_.continue_move(mouse_handler_, player_number_);
}

void playsingle_controller::unit_hold_position(){
	if (!browse_)
		menu_handler_.unit_hold_position(mouse_handler_, player_number_);
}

void playsingle_controller::end_unit_turn(){
	if (!browse_)
		menu_handler_.end_unit_turn(mouse_handler_, player_number_);
}

void playsingle_controller::user_command(){
	menu_handler_.user_command();
}

LEVEL_RESULT playsingle_controller::play_scenario(const std::vector<config*>& story, upload_log& log,
												  bool skip_replay)
{
	LOG_NG << "in playsingle_controller::play_scenario()...\n";

	// Start music.
	const config::child_list& m = level_.get_children("music");
	config::const_child_iterator i;
	for (i = m.begin(); i != m.end(); i++) {
		sound::play_music_config(**i);
	}
	sound::commit_music_changes();

	if(!skip_replay) {
		for(std::vector<config*>::const_iterator story_i = story.begin(); story_i != story.end(); ++story_i) {

			show_intro(*gui_,**story_i, level_);
		}
	}
	gui_->labels().read(level_);

	//find a list of 'items' (i.e. overlays) on the level, and add them
	const config::child_list& overlays = level_.get_children("item");
	for(config::child_list::const_iterator overlay = overlays.begin(); overlay != overlays.end(); ++overlay) {
		gui_->add_overlay(gamemap::location(**overlay),(**overlay)["image"], (**overlay)["halo"]);
	}

	victory_conditions::set_victory_when_enemies_defeated(
						level_["victory_when_enemies_defeated"] != "no");

	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks_) << "\n";
	try {
		// log before prestart events: they do weird things.
		if (first_human_team_ != -1) {
			log.start(gamestate_, teams_[first_human_team_], first_human_team_ + 1, units_,
					  loading_game_ ? gamestate_.get_variable("turn_number") : "", status_.number_of_turns());
		}

		fire_prestart(!loading_game_);
		init_gui();

		LOG_NG << "first_time..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";

		fire_start(!loading_game_);
		gui_->recalculate_minimap();

		replaying_ = (recorder.at_end() == false);

		LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";

		// Initialize countdown clock.
		std::vector<team>::iterator t;
		for(t = teams_.begin(); t != teams_.end(); ++t) {
			std::string countd_enabled = level_["mp_countdown"].c_str();
			if ( countd_enabled == "yes" && !loading_game_ ){
				t->set_countdown_time(1000 * lexical_cast_default<int>(level_["mp_countdown_init_time"],0));
			}
		}

		bool save = false;
		for(; ; first_player_ = 1) {
			play_turn(save);
			save = true;
		} //end for loop

	} catch(game::load_game_exception&) {
		// Loading a new game is effectively a quit.
		log.quit(status_.turn());
		throw;
	} catch(end_level_exception& end_level) {
		bool obs = team_manager_.is_observer();
		if (game_config::exit_at_end) {
			exit(0);
		}
		if (end_level.result == DEFEAT || end_level.result == VICTORY) {
			// if we're a player, and the result is victory/defeat, then send a message to notify
			// the server of the reason for the game ending
			if (!obs) {
				config cfg;
				config& info = cfg.add_child("info");
				info["type"] = "termination";
				info["condition"] = "game over";
				if (end_level.result == VICTORY)
					info["result"] = "victory";
				else
					info["result"] = "defeat";
				network::send_data(cfg);
			} else {
				gui::show_dialog(*gui_, NULL, _("Game Over"),
				                 _("The game is over."), gui::OK_ONLY);
				return QUIT;
			}
		}

		if(end_level.result == QUIT) {
			log.quit(status_.turn());
			return end_level.result;
		} else if(end_level.result == DEFEAT) {
			log.defeat(status_.turn());
			try {
				game_events::fire("defeat");
			} catch(end_level_exception&) {
			}

			if (!obs)
				return DEFEAT;
			else
				return QUIT;
		} else if (end_level.result == VICTORY || end_level.result == LEVEL_CONTINUE ||
		           end_level.result == LEVEL_CONTINUE_NO_SAVE) {
			try {
				game_events::fire("victory");
			} catch(end_level_exception&) {
			}
			if (first_human_team_ != -1)
				log.victory(status_.turn(), teams_[first_human_team_].gold());

			if(gamestate_.scenario == (level_)["id"]) {
				gamestate_.scenario = (level_)["next_scenario"];
			}

			const bool has_next_scenario = !gamestate_.scenario.empty() &&
			                               gamestate_.scenario != "null";

			//add all the units that survived the scenario
			for(unit_map::iterator un = units_.begin(); un != units_.end(); ++un) {
				player_info *player=gamestate_.get_player(teams_[un->second.side()-1].save_id());

				if(player) {
					un->second.new_turn();
					un->second.new_level();
					player->available_units.push_back(un->second);
				}
			}

			//'continue' is like a victory, except it doesn't announce victory,
			//and the player retains 100% of gold.
			if(end_level.result == LEVEL_CONTINUE || end_level.result == LEVEL_CONTINUE_NO_SAVE) {
				for(std::vector<team>::iterator i=teams_.begin(); i!=teams_.end(); ++i) {
					player_info *player=gamestate_.get_player(i->save_id());
					if(player) {
						player->gold = i->gold();
					}
				}

				return end_level.result == LEVEL_CONTINUE_NO_SAVE ? LEVEL_CONTINUE_NO_SAVE : VICTORY;
			}


			std::stringstream report;

			for(std::vector<team>::iterator i=teams_.begin(); i!=teams_.end(); ++i) {
				if (!i->is_persistent())
					continue;

				player_info *player=gamestate_.get_player(i->save_id());

				const int remaining_gold = i->gold();
				const int finishing_bonus_per_turn =
				             map_.villages().size() * game_config::village_income +
				             game_config::base_income;
				const int turns_left = maximum<int>(0,status_.number_of_turns() - status_.turn());
				const int finishing_bonus = end_level.gold_bonus ?
				             (finishing_bonus_per_turn * turns_left) : 0;

				if(player) {
					player->gold = ((remaining_gold + finishing_bonus) * 80) / 100;

					if(gamestate_.players.size()>1) {
						if(i!=teams_.begin()) {
							report << "\n";
						}

						report << font::BOLD_TEXT << i->save_id() << "\n";
					}

					report << _("Remaining gold: ")
					       << remaining_gold << "\n";
					if(end_level.gold_bonus) {
						report << _("Early finish bonus: ")
						       << finishing_bonus_per_turn
						       << " " << _("per turn") << "\n"
						       << _("Turns finished early: ")
						       << turns_left << "\n"
						       << _("Bonus: ")
						       << finishing_bonus << "\n"
						       << _("Gold: ")
						       << (remaining_gold+finishing_bonus);
					}

					// xgettext:no-c-format
					report << '\n' << _("80% of gold is retained for the next scenario") << '\n'
					       << _("Retained Gold: ") << player->gold;
				}
			}

			if (!obs)
				gui::show_dialog(*gui_, NULL, _("Victory"),
				                 _("You have emerged victorious!"), gui::OK_ONLY);

			if (gamestate_.players.size() > 0 && has_next_scenario ||
					gamestate_.campaign_type == "test")
				gui::show_dialog(*gui_, NULL, _("Scenario Report"), report.str(), gui::OK_ONLY);

			return VICTORY;
		}
		image::flush_cache();
	} //end catch
	catch(replay::error&) {
		gui::show_dialog(*gui_,NULL,"",_("The file you have tried to load is corrupt"),
		                 gui::OK_ONLY);
		return QUIT;
	}
	catch(network::error& e) {
		bool disconnect = false;
		if(e.socket) {
			e.disconnect();
			disconnect = true;
		}

		menu_handler_.save_game(_("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"),gui::YES_NO);
		if(disconnect) {
			throw network::error();
		} else {
			return QUIT;
		}
	}

	return QUIT;
}

void playsingle_controller::play_turn(bool save)
{
	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();

	LOG_NG << "turn: " << status_.turn() << "\n";

	for(player_number_ = first_player_; player_number_ <= teams_.size(); player_number_++) {
		//if a side is dead, don't do their turn
		if(!teams_[player_number_ - 1].is_empty() && team_units(units_,player_number_) > 0) {
			init_side(player_number_ - 1);

			if (replaying_){
				/* YogiHH: I can't see why we need another key_handler here in addition
				to the one defined in play_controller. Since this is causing problems with
				double execution of hotkeys i will comment it out
				const hotkey::basic_handler key_events_handler(gui_);
				*/
				LOG_NG << "doing replay " << player_number_ << "\n";
				try {
					replaying_ = ::do_replay(*gui_,map_,gameinfo_,units_,teams_,
										  player_number_,status_,gamestate_);
				} catch(replay::error&) {
					gui::show_dialog(*gui_,NULL,"",_("The file you have tried to load is corrupt"),gui::OK_ONLY);

					replaying_ = false;
				}
				LOG_NG << "result of replay: " << (replaying_?"true":"false") << "\n";
			}
			else{
				play_side(player_number_, save);
			}

			finish_side_turn();
			check_victory(units_,teams_);
		}
	}

	//time has run out
	check_time_over();

	finish_turn();
}

void playsingle_controller::play_side(const unsigned int team_index, bool save)
{
//goto this label if the type of a team (human/ai/networked) has changed mid-turn
redo_turn:
	//although this flag is used only in this method it has to be a class member
	//since derived classes rely on it
	player_type_changed_ = false;
	end_turn_ = false;

	if(current_team().is_human()) {
		LOG_NG << "is human...\n";
		try{
			before_human_turn(save);
			play_human_turn();
			after_human_turn();
		} catch(end_turn_exception& end_turn) {
			if (end_turn.redo == team_index) {
				player_type_changed_ = true;
				// if new controller is not human,
				// reset gui to prev human one
				if (!teams_[team_index-1].is_human()) {
					int t = find_human_team_before(team_index);
					if (t > 0) {
						gui_->set_team(t-1);
						gui_->recalculate_minimap();
						gui_->invalidate_all();
						gui_->draw();
						gui_->update_display();
					}
				}
			}
		}

		if(game_config::debug)
			display::clear_debug_highlights();

		LOG_NG << "human finished turn...\n";
	} else if(current_team().is_ai()) {
		play_ai_turn();
	}

	if (player_type_changed_) { goto redo_turn; }
}

void playsingle_controller::before_human_turn(bool save)
{
	log_scope("player turn");
	browse_ = false;

	gui_->set_team(player_number_ - 1);
	gui_->recalculate_minimap();
	gui_->invalidate_all();
	gui_->draw();
	gui_->update_display();

	if (save) {
		menu_handler_.autosave(gamestate_.label, status_.turn(), gamestate_.starting_pos);
	}

	if(preferences::turn_bell()) {
		sound::play_sound(game_config::sounds::turn_bell);
	}

	if(preferences::turn_dialog()) {
		gui::show_dialog(*gui_,NULL,"",_("It is now your turn"),gui::MESSAGE);
	}

	const std::string& turn_cmd = preferences::turn_cmd();
	if(turn_cmd.empty() == false) {
		system(turn_cmd.c_str());
	}

	//execute gotos - first collect gotos in a list
	std::vector<gamemap::location> gotos;

	for(unit_map::iterator ui = units_.begin(); ui != units_.end(); ++ui) {
		if(ui->second.get_goto() == ui->first)
			ui->second.set_goto(gamemap::location());

		if(ui->second.side() == player_number_ && map_.on_board(ui->second.get_goto()))
			gotos.push_back(ui->first);
	}

	for(std::vector<gamemap::location>::const_iterator g = gotos.begin(); g != gotos.end(); ++g) {
		unit_map::const_iterator ui = units_.find(*g);
		menu_handler_.move_unit_to_loc(ui,ui->second.get_goto(),false, player_number_, mouse_handler_);
	}
}

void playsingle_controller::play_human_turn(){
	gui_->enable_menu("endturn", true);
	while(!end_turn_) {
		play_slice();

		gui_->invalidate_animations();
		gui_->draw();
	}
}

void playsingle_controller::after_human_turn(){
	menu_handler_.clear_undo_stack(player_number_);
	gui_->unhighlight_reach();
}

void playsingle_controller::play_ai_turn(){
	LOG_NG << "is ai...\n";
	gui_->enable_menu("endturn", false);
	browse_ = true;
	gui_->recalculate_minimap();

	const cursor::setter cursor_setter(cursor::WAIT);

	turn_info turn_data(gameinfo_,gamestate_,status_,*gui_,
						map_,teams_,player_number_,units_, replay_sender_, undo_stack_);

	ai_interface::info ai_info(*gui_,map_,gameinfo_,units_,teams_,player_number_,status_, turn_data);
	util::scoped_ptr<ai_interface> ai_obj(create_ai(current_team().ai_algorithm(),ai_info));
	ai_obj->user_interact().attach_handler(this);
	ai_obj->unit_recruited().attach_handler(this);
	ai_obj->unit_moved().attach_handler(this);
	ai_obj->enemy_attacked().attach_handler(this);
	ai_obj->play_turn();
	recorder.end_turn();
	turn_data.sync_network();

	gui_->recalculate_minimap();
	::clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,player_number_-1);
	gui_->invalidate_unit();
	gui_->invalidate_game_status();
	gui_->invalidate_all();
	gui_->draw();
	gui_->delay(500);
}

void playsingle_controller::handle_generic_event(const std::string& name){
	if (name == "ai_user_interact"){
		play_slice();
		gui_->draw();
	}
}

void playsingle_controller::check_time_over(){
	if(!status_.next_turn()) {

		if(non_interactive()) {
			std::cout << "time over (draw)\n";
		}

		LOG_NG << "firing time over event...\n";
		game_events::fire("time over");
		LOG_NG << "done firing time over event...\n";

		throw end_level_exception(DEFEAT);
	}
}

bool playsingle_controller::can_execute_command(hotkey::HOTKEY_COMMAND command) const
{
	switch (command){
		case hotkey::HOTKEY_UNIT_HOLD_POSITION:
		case hotkey::HOTKEY_END_UNIT_TURN:
		case hotkey::HOTKEY_RECRUIT:
		case hotkey::HOTKEY_REPEAT_RECRUIT:
		case hotkey::HOTKEY_RECALL:
		case hotkey::HOTKEY_ENDTURN:
			return !browse_ && !events::commands_disabled;

		case hotkey::HOTKEY_DELAY_SHROUD:
			return !browse_ && (current_team().uses_fog() || current_team().uses_shroud());
		case hotkey::HOTKEY_UPDATE_SHROUD:
			return !browse_ && !events::commands_disabled && current_team().auto_shroud_updates() == false;

		//commands we can only do if in debug mode
		case hotkey::HOTKEY_CREATE_UNIT:
		case hotkey::HOTKEY_CHANGE_UNIT_SIDE:
			return !events::commands_disabled && game_config::debug && map_.on_board(mouse_handler_.get_last_hex());

		case hotkey::HOTKEY_LABEL_TERRAIN:
			return !events::commands_disabled && map_.on_board(mouse_handler_.get_last_hex())
				&& !gui_->shrouded(mouse_handler_.get_last_hex().x, mouse_handler_.get_last_hex().y)
				&& !is_observer();

		case hotkey::HOTKEY_CONTINUE_MOVE: {
			if(browse_ || events::commands_disabled)
				return false;

			if( (menu_handler_.current_unit(mouse_handler_) != units_.end())
				&& (menu_handler_.current_unit(mouse_handler_)->second.move_interrupted()))
				return true;
			const unit_map::const_iterator i = units_.find(mouse_handler_.get_selected_hex());
			if (i == units_.end()) return false;
			return i->second.move_interrupted();
		}
		default: return play_controller::can_execute_command(command);
	}
}
