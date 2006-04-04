#include "play_controller.hpp"

#include "config_adapter.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "sound.hpp"
#include "unit_display.hpp"
#include "wassert.hpp"

#define LOG_NG LOG_STREAM(info, engine)

play_controller::play_controller(const config& level, const game_data& gameinfo, game_state& state_of_game, 
								 int ticks, int num_turns, const config& game_config, CVideo& video,
								 bool skip_replay) : 
	level_(level), ticks_(ticks), first_human_team_(-1), gameinfo_(gameinfo),
	gamestate_(state_of_game), status_(level, num_turns), statistics_context_(level["name"]),
	game_config_(game_config), map_(game_config, level["map_data"]), verify_manager_(units_),
	labels_manager_(), help_manager_(&game_config, &gameinfo, &map_), 
	team_manager_(teams_), xp_modifier_(atoi(level["experience_modifier"].c_str())),
	loading_game_(level["playing_team"].empty() == false),
	menu_handler_(gui_, units_, teams_, level, gameinfo, map_, game_config, status_, state_of_game, undo_stack_, redo_stack_),
	mouse_handler_(gui_, teams_, units_, map_, status_, gameinfo, undo_stack_, redo_stack_)
{
	player_number_ = 1;
	current_turn_ = 1;
	first_player_ = atoi(level_["playing_team"].c_str());
	if(first_player_ < 0 || first_player_ >= int(teams_.size())) {
		first_player_ = 0;
	}
	skip_replay_ = skip_replay;
	browse_ = false;

	init(video);
	key_events_handler_ = new hotkey::basic_handler(gui_);
}

play_controller::~play_controller(){
	delete key_events_handler_;
	delete halo_manager_;
	delete prefs_disp_manager_;
	delete tooltips_manager_;
	delete events_manager_;
	delete gui_;
}

void play_controller::init(CVideo& video){
	//if the recorder has no event, adds an "game start" event to the
	//recorder, whose only goal is to initialize the RNG
	if(recorder.empty()) {
		recorder.add_start();
	} else {
		recorder.pre_replay();
	}
	recorder.set_skip(false);

	const config::child_list& unit_cfg = level_.get_children("side");

	if(level_["modify_placing"] == "true") {
		LOG_NG << "modifying placing...\n";
		place_sides_in_preferred_locations(map_,unit_cfg);
	}

	LOG_NG << "initializing teams..." << unit_cfg.size() << "\n";;
	LOG_NG << (SDL_GetTicks() - ticks_) << "\n";

	std::set<std::string> seen_save_ids;

	for(config::child_list::const_iterator ui = unit_cfg.begin(); ui != unit_cfg.end(); ++ui) {
		std::string save_id = get_unique_saveid(**ui, seen_save_ids);
		seen_save_ids.insert(save_id);
		if (first_human_team_ == -1){
			first_human_team_ = get_first_human_team(ui, unit_cfg);
		}
		get_player_info(**ui, gamestate_, save_id, teams_, level_, gameinfo_, map_, units_);
	}

	preferences::encounter_recruitable_units(teams_);
	preferences::encounter_start_units(units_);
	preferences::encounter_recallable_units(gamestate_);
	preferences::encounter_map_terrain(map_);

	LOG_NG << "initialized teams... " << (SDL_GetTicks() - ticks_) << "\n";
	LOG_NG << "initializing display... " << (SDL_GetTicks() - ticks_) << "\n";

	const config* theme_cfg = get_theme(game_config_, level_["theme"]);
	gui_ = new display(units_,video,map_,status_,teams_,*theme_cfg, game_config_, level_);
	mouse_handler_.set_gui(gui_);
	menu_handler_.set_gui(gui_);
	theme::set_known_themes(&game_config_);

	LOG_NG << "done initializing display... " << (SDL_GetTicks() - ticks_) << "\n";

	if(first_human_team_ != -1) {
		gui_->set_team(first_human_team_);
	}

	init_managers();

	gui_->labels().read(level_);

	LOG_NG << "start music... " << (SDL_GetTicks() - ticks_) << "\n";

	const std::string& music = level_["music"];
	if(music != "") {
		sound::play_music_repeatedly(music);
	}

	//find a list of 'items' (i.e. overlays) on the level, and add them
	const config::child_list& overlays = level_.get_children("item");
	for(config::child_list::const_iterator overlay = overlays.begin(); overlay != overlays.end(); ++overlay) {
		gui_->add_overlay(gamemap::location(**overlay),(**overlay)["image"], (**overlay)["halo"]);
	}
}

void play_controller::init_managers(){
	LOG_NG << "initializing managers... " << (SDL_GetTicks() - ticks_) << "\n";
	prefs_disp_manager_ = new preferences::display_manager(gui_);
	tooltips_manager_ = new tooltips::manager(gui_->video());

	//this *needs* to be created before the show_intro and show_map_scene
	//as that functions use the manager state_of_game
	events_manager_ = new game_events::manager(level_,*gui_,map_,units_,teams_,
	                                    gamestate_,status_,gameinfo_);

	halo_manager_ = new halo::manager(*gui_);
	LOG_NG << "done initializing managers... " << (SDL_GetTicks() - ticks_) << "\n";
}

int placing_score(const config& side, const gamemap& map, const gamemap::location& pos)
{
	int positions = 0, liked = 0;
	const std::string& terrain_liked = side["terrain_liked"];
	for(int i = pos.x-8; i != pos.x+8; ++i) {
		for(int j = pos.y-8; j != pos.y+8; ++j) {
			const gamemap::location pos(i,j);
			if(map.on_board(pos)) {
				++positions;
				if(std::count(terrain_liked.begin(),terrain_liked.end(),map[i][j])) {
					++liked;
				}
			}
		}
	}

	return (100*liked)/positions;
}

struct placing_info {
	int side, score;
	gamemap::location pos;
};

bool operator<(const placing_info& a, const placing_info& b) { return a.score > b.score; }
bool operator==(const placing_info& a, const placing_info& b) { return a.score == b.score; }

void play_controller::place_sides_in_preferred_locations(gamemap& map, const config::child_list& sides)
{
	std::vector<placing_info> placings;

	const int num_pos = map.num_valid_starting_positions();

	for(config::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		const int side_num = s - sides.begin() + 1;
		for(int p = 1; p <= num_pos; ++p) {
			const gamemap::location& pos = map.starting_position(p);
			const int score = placing_score(**s,map,pos);
			placing_info obj;
			obj.side = side_num;
			obj.score = score;
			obj.pos = pos;
			placings.push_back(obj);
		}
	}

	std::sort(placings.begin(),placings.end());
	std::set<int> placed;
	std::set<gamemap::location> positions_taken;

	for(std::vector<placing_info>::const_iterator i = placings.begin(); i != placings.end() && placed.size() != sides.size(); ++i) {
		if(placed.count(i->side) == 0 && positions_taken.count(i->pos) == 0) {
			placed.insert(i->side);
			positions_taken.insert(i->pos);
			map.set_starting_position(i->side,i->pos);
			LOG_NG << "placing side " << i->side << " at " << i->pos << '\n';
		}
	}
}

void play_controller::objectives(){
	menu_handler_.objectives(player_number_);
}

void play_controller::show_statistics(){
	menu_handler_.show_statistics();
}

void play_controller::unit_list(){
	menu_handler_.unit_list();
}

void play_controller::status_table(){
	menu_handler_.status_table();
}

void play_controller::save_game(){
	menu_handler_.save_game("",gui::OK_CANCEL);
}

void play_controller::load_game(){
	menu_handler_.load_game();
}

void play_controller::preferences(){
	menu_handler_.preferences();
}

void play_controller::cycle_units(){
	mouse_handler_.cycle_units();
}

void play_controller::cycle_back_units(){
	mouse_handler_.cycle_back_units();
}

void play_controller::show_chat_log(){
	menu_handler_.show_chat_log();
}

void play_controller::show_help(){
	menu_handler_.show_help();
}

void play_controller::undo(){
	menu_handler_.undo(player_number_, mouse_handler_);
}

void play_controller::redo(){
	menu_handler_.redo(player_number_, mouse_handler_);
}

void play_controller::show_enemy_moves(bool ignore_units){
	menu_handler_.show_enemy_moves(ignore_units, player_number_);
}

void play_controller::goto_leader(){
	menu_handler_.goto_leader(player_number_);
}

void play_controller::unit_description(){
	menu_handler_.unit_description(mouse_handler_);
}

void play_controller::toggle_grid(){
	menu_handler_.toggle_grid();
}

void play_controller::search(){
	menu_handler_.search();
}

const int play_controller::get_xp_modifier(){
	return xp_modifier_;
}

const int play_controller::get_ticks(){
	return ticks_;
}

void play_controller::fire_prestart(bool execute){
	//pre-start events must be executed before any GUI operation,
	//as those may cause the display to be refreshed.
	if (execute){
		update_locker lock_display(gui_->video());
		game_events::fire("prestart");
	}
}

void play_controller::fire_start(bool execute){
	if(execute) {
		game_events::fire("start");
		gamestate_.set_variable("turn_number", "1");
	}
}

void play_controller::init_gui(){
	gui_->begin_game();
	gui_->adjust_colours(0,0,0);

	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		::clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,(t-teams_.begin()));
	}
}

void play_controller::init_side(const int team_index){
	log_scope("player turn");
	team& current_team = teams_[team_index];

	//if a side is dead, don't do their turn
	if(!current_team.is_empty() || team_units(units_,player_number_) == 0) {
		if(team_manager_.is_observer()) {
			gui_->set_team(size_t(player_number_-1));
		}

		std::stringstream player_number_str;
		player_number_str << player_number_;
		gamestate_.set_variable("side_number",player_number_str.str());

		//fire side turn event only if real side change occurs not counting changes from void to a side
		if (team_index != first_player_ || current_turn_ > 1) {
			game_events::fire("side turn");
		}

		//we want to work out if units for this player should get healed, and the
		//player should get income now. healing/income happen if it's not the first
		//turn of processing, or if we are loading a game, and this is not the
		//player it started with.
		const bool turn_refresh = current_turn_ > 1 || loading_game_ && team_index != first_player_;

		if(turn_refresh) {
			for(unit_map::iterator i = units_.begin(); i != units_.end(); ++i) {
				if(i->second.side() == (size_t)player_number_) {
					i->second.new_turn();
				}
			}

			current_team.new_turn();

			//if the expense is less than the number of villages owned,
			//then we don't have to pay anything at all
			const int expense = team_upkeep(units_,player_number_) -
								current_team.villages().size();
			if(expense > 0) {
				current_team.spend_gold(expense);
			}

			calculate_healing((*gui_),status_,map_,units_,player_number_,teams_, !recorder.is_skipping());
		}

		current_team.set_time_of_day(int(status_.turn()),status_.get_time_of_day());

		gui_->set_playing_team(size_t(player_number_-1));

		if (!recorder.is_skipping()){
			::clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,player_number_-1);
		}

		if (!recorder.is_skipping()){
			gui_->scroll_to_leader(units_, player_number_);
		}
	}
}

bool play_controller::do_replay(const bool replaying){
	bool result = false;
	if(replaying) {
		const hotkey::basic_handler key_events_handler(gui_);
		LOG_NG << "doing replay " << player_number_ << "\n";
		try {
			result = ::do_replay(*gui_,map_,gameinfo_,units_,teams_,
						          player_number_,status_,gamestate_);
		} catch(replay::error&) {
			gui::show_dialog(*gui_,NULL,"",_("The file you have tried to load is corrupt"),gui::OK_ONLY);

			result = false;
		}
		LOG_NG << "result of replay: " << (result?"true":"false") << "\n";
	}
	return result;
}

void play_controller::check_music(const bool replaying){
	if(!replaying && current_team().music().empty() == false && 
			(teams_[gui_->viewing_team()].knows_about_team(player_number_-1) || teams_[gui_->viewing_team()].has_seen(player_number_-1))) {
		LOG_NG << "playing music: '" << current_team().music() << "'\n";
		sound::play_music_repeatedly(current_team().music());
	} else if(!replaying && current_team().music().empty() == false){
		LOG_NG << "playing music: '" << game_config::anonymous_music<< "'\n";
		sound::play_music_repeatedly(game_config::anonymous_music);
	}
	// else leave old music playing, it's a scenario specific music
}

void play_controller::finish_side_turn(){
	for(unit_map::iterator uit = units_.begin(); uit != units_.end(); ++uit) {
		if(uit->second.side() == player_number_)
			uit->second.end_turn();
	}

	//This implements "delayed map sharing." It's meant as an alternative to shared vision.
	if(current_team().copy_ally_shroud()) {
		gui_->recalculate_minimap();
		gui_->invalidate_all();
	}

	game_events::pump();
}

void play_controller::finish_turn(){
	std::stringstream event_stream;
	event_stream << status_.turn();

	{
		LOG_NG << "turn event..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";
		update_locker lock_display(gui_->video(),recorder.is_skipping());
		const std::string turn_num = event_stream.str();
		gamestate_.set_variable("turn_number",turn_num);
		game_events::fire("turn " + turn_num);
		game_events::fire("new turn");
	}
}

bool play_controller::enemies_visible() const
{
	// If we aren't using fog/shroud, this is easy :)
	if(current_team().uses_fog() == false && current_team().uses_shroud() == false)
		return true;

	//See if any enemies are visible
	for(unit_map::const_iterator u = units_.begin(); u != units_.end(); ++u)
		if(current_team().is_enemy(u->second.side()) && !gui_->fogged(u->first.x,u->first.y))
			return true;

	return false;
}

bool play_controller::can_execute_command(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {

	//commands we can always do
	case hotkey::HOTKEY_LEADER:
	case hotkey::HOTKEY_CYCLE_UNITS:
	case hotkey::HOTKEY_CYCLE_BACK_UNITS:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_SCREENSHOT:
	case hotkey::HOTKEY_ACCELERATED:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_STATUS_TABLE:
	case hotkey::HOTKEY_MUTE:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_OBJECTIVES:
	case hotkey::HOTKEY_UNIT_LIST:
	case hotkey::HOTKEY_STATISTICS:
	case hotkey::HOTKEY_QUIT_GAME:
	case hotkey::HOTKEY_SEARCH:
	case hotkey::HOTKEY_HELP:
	case hotkey::HOTKEY_USER_CMD:
	case hotkey::HOTKEY_SAVE_GAME:
		return true;

	case hotkey::HOTKEY_SHOW_ENEMY_MOVES:
	case hotkey::HOTKEY_BEST_ENEMY_MOVES:
		return enemies_visible();

	case hotkey::HOTKEY_LOAD_GAME:
		return network::nconnections() == 0; //can only load games if not in a network game

	case hotkey::HOTKEY_CHAT_LOG:
		return network::nconnections() > 0;

	case hotkey::HOTKEY_REDO:
		return !browse_ && !redo_stack_.empty() && !events::commands_disabled;
	case hotkey::HOTKEY_UNDO:
		return !browse_ && !undo_stack_.empty() && !events::commands_disabled;

	case hotkey::HOTKEY_UNIT_DESCRIPTION:
		return menu_handler_.current_unit(mouse_handler_) != units_.end();

	case hotkey::HOTKEY_RENAME_UNIT:
		return !events::commands_disabled &&
			menu_handler_.current_unit(mouse_handler_) != units_.end() &&
			!(menu_handler_.current_unit(mouse_handler_)->second.unrenamable()) &&
			menu_handler_.current_unit(mouse_handler_)->second.side() == gui_->viewing_team()+1;

	default:
		return false;
	}
}

void play_controller::enter_textbox()
{
	if(menu_handler_.get_textbox().active() == false) {
		return;
	}

	switch(menu_handler_.get_textbox().mode_) {
	case gui::TEXTBOX_SEARCH:
		menu_handler_.do_search(menu_handler_.get_textbox().box_->text());
		break;
	case gui::TEXTBOX_MESSAGE:
		menu_handler_.do_speak();
		break;
	case gui::TEXTBOX_COMMAND:
		menu_handler_.do_command(menu_handler_.get_textbox().box_->text(), player_number_, mouse_handler_);
		break;
	default:
		LOG_STREAM(err, display) << "unknown textbox mode\n";
	}

	menu_handler_.get_textbox().close(*gui_);
}

void play_controller::handle_event(const SDL_Event& event)
{
	if(gui::in_dialog()) {
		return;
	}

	switch(event.type) {
	case SDL_KEYDOWN:
		//detect key press events, unless there is a textbox present on-screen
		//in which case the key press events should go only to it.
		if(menu_handler_.get_textbox().active() == false) {
			hotkey::key_event(*gui_,event.key,this);
		} else if(event.key.keysym.sym == SDLK_ESCAPE) {
			menu_handler_.get_textbox().close(*gui_);
		} else if(event.key.keysym.sym == SDLK_TAB) {
			menu_handler_.get_textbox().tab(teams_, units_, *gui_);
		} else if(event.key.keysym.sym == SDLK_RETURN) {
			enter_textbox();
		}

		//intentionally fall-through
	case SDL_KEYUP:
		
		//if the user has pressed 1 through 9, we want to show how far
		//the unit can move in that many turns
		if(event.key.keysym.sym >= '1' && event.key.keysym.sym <= '7') {
			const int new_path_turns = (event.type == SDL_KEYDOWN) ?
			                           event.key.keysym.sym - '1' : 0;

			if(new_path_turns != mouse_handler_.get_path_turns()) {
				mouse_handler_.set_path_turns(new_path_turns);

				const unit_map::iterator u = mouse_handler_.selected_unit();

				if(u != units_.end() && u->second.side() == player_number_) {
					const bool ignore_zocs = u->second.type().is_skirmisher();
					const bool teleport = u->second.type().teleports();
					mouse_handler_.set_current_paths(paths(map_,status_,gameinfo_,units_,u->first,
					                       teams_,ignore_zocs,teleport, teams_[gui_->viewing_team()],
					                       mouse_handler_.get_path_turns()));
					gui_->highlight_reach(mouse_handler_.get_current_paths());
				}
			}
		}
		
		break;
	case SDL_MOUSEMOTION:
		// ignore old mouse motion events in the event queue
		SDL_Event new_event;

		if(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
					SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0) {
			while(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
						SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0);
			mouse_handler_.mouse_motion(new_event.motion, player_number_, browse_);
		} else {
			mouse_handler_.mouse_motion(event.motion, player_number_, browse_);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		mouse_handler_.mouse_press(event.button, player_number_, browse_);
		if (mouse_handler_.get_undo()){
			menu_handler_.undo(player_number_, mouse_handler_);
		}
		if (mouse_handler_.get_show_menu()){
			show_menu(gui_->get_theme().context_menu()->items(),event.button.x,event.button.y,true);
		}
		break;
	default:
		break;
	}
}

void play_controller::play_slice()
{
	CKey key;

	events::pump();
	events::raise_process_event();

	events::raise_draw_event();

	const theme::menu* const m = gui_->menu_pressed();
	if(m != NULL) {
		const SDL_Rect& menu_loc = m->location(gui_->screen_area());
		show_menu(m->items(),menu_loc.x+1,menu_loc.y + menu_loc.h + 1,false);
		return;
	}

	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	const int scroll_threshold = 5;

	if(key[SDLK_UP] || mousey < scroll_threshold)
		gui_->scroll(0,-preferences::scroll_speed());

	if(key[SDLK_DOWN] || mousey > gui_->y()-scroll_threshold)
		gui_->scroll(0,preferences::scroll_speed());

	if(key[SDLK_LEFT] || mousex < scroll_threshold)
		gui_->scroll(-preferences::scroll_speed(),0);

	if(key[SDLK_RIGHT] || mousex > gui_->x()-scroll_threshold)
		gui_->scroll(preferences::scroll_speed(),0);

	gui_->draw();

	if(!browse_ && current_team().objectives_changed()) {
		dialogs::show_objectives(*gui_, level_, current_team().objectives());
		current_team().reset_objectives_changed();
	}
}

void play_controller::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{
	std::vector<std::string> items = items_arg;
	hotkey::HOTKEY_COMMAND command;
	for(std::vector<std::string>::iterator i = items.begin(); i != items.end();){
		command = hotkey::get_hotkey(*i).get_id();
		if (!can_execute_command(command) || (context_menu && !in_context_menu(command))){
			i = items.erase(i);
		}
		else{ i++; }
	}

	if(items.empty())
		return;

	command_executor::show_menu(items, xloc, yloc, context_menu, *gui_);
}

// Indicates whether the command should be in the context menu or not.
// Independant of whether or not we can actually execute the command.
bool play_controller::in_context_menu(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {
	//Only display these if the mouse is over a castle or keep tile
	case hotkey::HOTKEY_RECRUIT:
	case hotkey::HOTKEY_REPEAT_RECRUIT:
	case hotkey::HOTKEY_RECALL: {
		// last_hex_ is set by turn_info::mouse_motion
		// Enable recruit/recall on castle/keep tiles
		const unit_map::const_iterator leader = team_leader(player_number_,units_);
		if (leader != units_.end()) {
			return can_recruit_on(map_, leader->first, mouse_handler_.get_last_hex());
		} else {
			return false;
		}
	}
	default:
		return true;
	}
}

