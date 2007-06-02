/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "multiplayer.hpp"
#include "multiplayer_ui.hpp"
#include "multiplayer_connect.hpp"
#include "multiplayer_wait.hpp"
#include "multiplayer_lobby.hpp"
#include "multiplayer_create.hpp"
#include "network.hpp"
#include "playcampaign.hpp"
#include "playmp_controller.hpp"
#include "preferences.hpp"
#include "preferences_display.hpp"
#include "random.hpp"
#include "replay.hpp"
#include "video.hpp"
#include "statistics.hpp"
#include "serialization/string_utils.hpp"
#include "upload_log.hpp"
#include "wml_separators.hpp"

#define LOG_NW LOG_STREAM(info, network)

namespace {

class network_game_manager
{
public:
	// Add a constructor to avoid stupid warnings with some versions of GCC
	network_game_manager() {};

	~network_game_manager()
	{
		if(network::nconnections() > 0) {
			LOG_NW << "sending leave_game\n";
			config cfg;
			cfg.add_child("leave_game");
			network::send_data(cfg);
			LOG_NW << "sent leave_game\n";
		}
	};
};


void run_lobby_loop(display& disp, mp::ui& ui)
{
	disp.video().modeChanged();
	bool first = true;
	font::cache_mode(font::CACHE_LOBBY);
	while (ui.get_result() == mp::ui::CONTINUE) {
		if (disp.video().modeChanged() || first) {
			SDL_Rect lobby_pos = { 0, 0, disp.video().getx(), disp.video().gety() };
			ui.set_location(lobby_pos);
			first = false;
		}

		events::raise_process_event();
		events::raise_draw_event();
		events::pump();

		ui.process_network();

		disp.flip();
		disp.delay(20);
	}
	font::cache_mode(font::CACHE_GAME);
}

enum server_type {
	ABORT_SERVER,
	WESNOTHD_SERVER,
	SIMPLE_SERVER
};

class server_list_action : public gui::dialog_button_action
{
public:
	server_list_action(display& disp, std::string& result) : disp_(disp), result_(result)
	{}
private:
	gui::dialog_button_action::RESULT button_pressed(int /*menu_selection*/)
	{
		std::vector<std::string> servers;
		const std::vector<game_config::server_info>& pref_servers = preferences::server_list();
		std::vector<game_config::server_info>::const_iterator server;
		for(server = pref_servers.begin(); server != pref_servers.end(); ++server) {
			servers.push_back(server->address + HELP_STRING_SEPARATOR + server->name);
		}
		int res = gui::show_dialog(disp_, NULL, _("List of Servers"), 
			_("Choose a known server from the list"), gui::OK_CANCEL, &servers);
		if(res >= 0) {
			result_ = pref_servers[res].address;
		} else {
			result_ = "invalid";
		}
		return gui::dialog_button_action::CLOSE_DIALOG;
	}

	display& disp_;
	std::string& result_;
};

server_type open_connection(display& disp, const std::string& original_host)
{
	std::string h = original_host;

	if(h.empty()) {
		h = preferences::network_host();
		std::string list_return;
		do {
			if(!(list_return.empty() || list_return == "invalid")) {
				h = list_return;
			}
			list_return = "";
			server_list_action sla(disp, list_return);
			gui::dialog_button sla_button(&sla,_("View List"));
			std::vector<gui::dialog_button> buttons;
			buttons.push_back(sla_button);
			const int res = gui::show_dialog(disp, NULL, _("Connect to Host"), "",
				gui::OK_CANCEL, NULL, NULL,
				_("Choose host to connect to: "), &h,
				-1, NULL, NULL, -1, -1, NULL, &buttons);

			if(list_return.empty() && (res != 0 || h.empty())) {
				return ABORT_SERVER;
			}
		} while(!list_return.empty());
	}

	network::connection sock;

	const int pos = h.find_first_of(":");
	std::string host;
	unsigned int port;

	if(pos == -1) {
		host = h;
		port = 15000;
	} else {
		host = h.substr(0, pos);
		port = lexical_cast_default<unsigned int>(h.substr(pos + 1), 15000);
	}

	// shown_hosts is used to prevent the client being locked in a redirect
	// loop.
	typedef std::pair<std::string, int> hostpair;
	std::set<hostpair> shown_hosts;
	shown_hosts.insert(hostpair(host, port));

	config::child_list redirects;
	config data;
	sock = gui::network_connect_dialog(disp,_("Connecting to Server..."),host,port);

	do {

		if (!sock) {
			return ABORT_SERVER;
		}

		data.clear();
		network::connection data_res = gui::network_receive_dialog(
				disp,_("Reading from Server..."),data);
		mp::check_response(data_res, data);

		// Backwards-compatibility "version" attribute
		const std::string& version = data["version"];
		if(version.empty() == false && version != game_config::version) {
			utils::string_map i18n_symbols;
			i18n_symbols["version1"] = version;
			i18n_symbols["version2"] = game_config::version;
			const std::string errorstring = vgettext("The server requires version '$version1' while you are using version '$version2'", i18n_symbols);
			throw network::error(errorstring);
		}

		// Check for "redirect" messages
		if(data.child("redirect")) {
			config* redirect = data.child("redirect");

			host = (*redirect)["host"];
			port = lexical_cast_default<unsigned int>((*redirect)["port"], 15000);

			if(shown_hosts.find(hostpair(host,port)) != shown_hosts.end()) {
				throw network::error(_("Server-side redirect loop"));
			}
			shown_hosts.insert(hostpair(host, port));

			if(network::nconnections() > 0)
				network::disconnect();
			sock = gui::network_connect_dialog(disp,_("Connecting to Server..."),host,port);
			continue;
		}

		if(data.child("version")) {
			config cfg;
			config res;
			cfg["version"] = game_config::version;
			res.add_child("version", cfg);
			network::send_data(res);
		}

		//if we got a direction to login
		if(data.child("mustlogin")) {
			bool first_time = true;
			config* error = NULL;

			do {
				if(error != NULL) {
					gui::show_dialog(disp,NULL,"",(*error)["message"],gui::OK_ONLY);
				}

				std::string login = preferences::login();

				if(!first_time) {
					const int res = gui::show_dialog(disp, NULL, "",
							_("You must log in to this server"), gui::OK_CANCEL,
							NULL, NULL, _("Login: "), &login);
					if(res != 0 || login.empty()) {
						return ABORT_SERVER;
					}
					if(login.size() > 18) {
						gui::show_error_message(disp, _("The login name you chose is too long, please use a login with less than 18 characters"));
						return ABORT_SERVER;
					}

					preferences::set_login(login);
				}

				first_time = false;

				config response;
				response.add_child("login")["username"] = login;
				network::send_data(response);

				network::connection data_res = network::receive_data(data, 0, 3000);
				if(!data_res) {
					throw network::error(_("Connection timed out"));
				}

				error = data.child("error");
			} while(error != NULL);
		}
	} while(!(data.child("join_lobby") || data.child("join_game")));

	if (h != preferences::server_list().front().address)
		preferences::set_network_host(h);

	if (data.child("join_lobby")) {
		return WESNOTHD_SERVER;
	} else {
		return SIMPLE_SERVER;
	}

}


// The multiplayer logic consists in 4 screens:
//
// lobby <-> create <-> connect <-> (game)
//       <------------> wait    <-> (game)
//
// To each of this screen corresponds a dialog, each dialog being defined in
// the multiplayer_(screen) file.
//
// The functions enter_(screen)_mode are simple functions that take care of
// creating the dialogs, then, according to the dialog result, of calling other
// of those screen functions.

void enter_wait_mode(display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist, bool observe)
{
	mp::ui::result res;
	game_state state;
	network_game_manager m;
	upload_log nolog(false);

	gamelist.clear();
	statistics::fresh_stats();

	{
		mp::wait ui(disp, game_config, data, chat, gamelist);

		ui.join_game(observe);

		run_lobby_loop(disp, ui);
		res = ui.get_result();

		if (res == mp::ui::PLAY) {
			ui.start_game();
			//FIXME commented a pointeless if since the else does exactly the same thing
			//if (preferences::skip_mp_replay()){
				//FIXME implement true skip replay
				//state = ui.request_snapshot();
			//	state = ui.get_state();
			//}
			//else{
				state = ui.get_state();
			//}
		}
	}

	switch (res) {
	case mp::ui::PLAY:
		play_game(disp, state, game_config, data, disp.video(), nolog, IO_CLIENT, preferences::skip_mp_replay());
		recorder.clear();

		break;
	case mp::ui::QUIT:
	default:
		break;
	}
}

void enter_connect_mode(display& disp, const config& game_config, game_data& data,
		mp::chat& chat, config& gamelist, const mp::create::parameters& params,
		mp::controller default_controller, bool is_server)
{
	mp::ui::result res;
	game_state state;
	const network::manager net_manager;
	const network::server_manager serv_manager(15000, is_server ?
			network::server_manager::TRY_CREATE_SERVER :
			network::server_manager::NO_SERVER);
	network_game_manager m;
	upload_log nolog(false);

	gamelist.clear();
	statistics::fresh_stats();

	{
		mp::connect ui(disp, game_config, data, chat, gamelist, params, default_controller);
		run_lobby_loop(disp, ui);

		res = ui.get_result();

		// start_game() updates the parameters to reflect game start,
		// so it must be called before get_level()
		if (res == mp::ui::PLAY) {
			ui.start_game();
			state = ui.get_state();
			const config* const era_cfg = game_config.find_child("era","id",params.era);
			game_events::add_events(era_cfg->get_children("event"),"all");
		}
	}

	switch (res) {
	case mp::ui::PLAY:
		play_game(disp, state, game_config, data, disp.video(), nolog, IO_SERVER);
		recorder.clear();

		break;
	case mp::ui::QUIT:
	default:
		break;
	}
}

void enter_create_mode(display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist, mp::controller default_controller, bool is_server)
{
	mp::ui::result res;
	mp::create::parameters params;

	{
		mp::create ui(disp, game_config, chat, gamelist);
		run_lobby_loop(disp, ui);
		res = ui.get_result();
		params = ui.get_parameters();
	}

	switch (res) {
	case mp::ui::CREATE:
		enter_connect_mode(disp, game_config, data, chat, gamelist, params, default_controller, is_server);
		break;
	case mp::ui::QUIT:
	default:
		//update lobby content
		config request;
		request.add_child("refresh_lobby");
		network::send_data(request);
		break;
	}
}

void enter_lobby_mode(display& disp, const config& game_config, game_data& data, mp::chat& chat, config& gamelist)
{
	mp::ui::result res;

	while (true) {
		{
			mp::lobby ui(disp, game_config, chat, gamelist);
			run_lobby_loop(disp, ui);
			res = ui.get_result();
		}

		switch (res) {
		case mp::ui::JOIN:
			try {
				enter_wait_mode(disp, game_config, data, chat, gamelist, false);
			} catch(network::error& error) {
				if(!error.message.empty()) {
					if(error.message == _("No multiplayer sides available in this game") ||
					   error.message == _("Era not available") ||
					   error.message == _("No multiplayer sides found")) {
						gui::show_error_message(disp, error.message);
						break;
					}
				}
				throw error;
			}
			break;
		case mp::ui::OBSERVE:
			try {
				enter_wait_mode(disp, game_config, data, chat, gamelist, true);
			} catch(network::error& error) {
				if(!error.message.empty()) {
					if(error.message == _("No multiplayer sides available in this game") ||
					   error.message == _("Era not available") ||
					   error.message == _("No multiplayer sides found")) {
						gui::show_error_message(disp, error.message);
						break;
					}
				}
				throw error;
			}
			break;
		case mp::ui::CREATE:
			try {
				enter_create_mode(disp, game_config, data, chat, gamelist, mp::CNTR_NETWORK, false);
			} catch(network::error& error) {
				if (!error.message.empty())
					gui::show_error_message(disp, error.message);
				// Exit the lobby on network errors
				return;
			}
			break;
		case mp::ui::QUIT:
			return;
		case mp::ui::PREFERENCES:
			{
				const preferences::display_manager disp_manager(&disp);
				preferences::show_preferences_dialog(disp,game_config);
				//update lobby content
				config request;
				request.add_child("refresh_lobby");
				network::send_data(request);
			}
			break;
		default:
			return;
		}
	}
}

}

namespace mp {

void start_server(display& disp, const config& game_config, game_data& data,
		mp::controller default_controller, bool is_server)
{
	const set_random_generator generator_setter(&recorder);
	mp::chat chat;
	config gamelist;
	playmp_controller::set_replay_last_turn(0);

	enter_create_mode(disp, game_config, data, chat, gamelist, default_controller, is_server);
}

void start_client(display& disp, const config& game_config, game_data& data,
		const std::string host)
{
	const set_random_generator generator_setter(&recorder);
	const network::manager net_manager;

	mp::chat chat;
	config gamelist;
	server_type type = open_connection(disp, host);

	switch(type) {
	case WESNOTHD_SERVER:
		enter_lobby_mode(disp, game_config, data, chat, gamelist);
		break;
	case SIMPLE_SERVER:
		playmp_controller::set_replay_last_turn(0);
		enter_wait_mode(disp, game_config, data, chat, gamelist, false);
		break;
	case ABORT_SERVER:
		break;
	}
}

}

