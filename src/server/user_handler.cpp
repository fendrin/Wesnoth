#include "user_handler.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>


user_handler::user_handler(const std::string& users_file) :
        users_file_(users_file),
        cfg_(read_config())
{
    load_config();
}

user_handler::~user_handler() {
    save_config();
}

config user_handler::read_config() const {
	config configuration;
	if (users_file_ == "") return configuration;
	scoped_istream stream = preprocess_file(users_file_);
	std::string errors;
	try {
		read(configuration, *stream, &errors);
		if (errors.empty()) {
			std::cout << "Server configuration from file: '" << users_file_
				<< "' read.\n";
		} else {
			std::cerr << "ERROR: Errors reading configuration file: '"
				<< errors << "'.\n";
		}
	} catch(config::error& e) {
		std::cerr << "ERROR: Could not read configuration file: '"
			<< users_file_ << "': '" << e.message << "'.\n";
	}
	return configuration;
}

void user_handler::load_config() {

    //Load configuration and initialize with default values if we don't find values
    cfg_["from_address"] = cfg_["from_address"].empty() ? "NOREPLY@wesnoth.org" : cfg_["from_address"];
    cfg_["mail_server"] = cfg_["mail_server"].empty() ? "127.0.0.1" : cfg_["mail_server"];

    //"mail_user" and "mail_password" may stay empty

    if(cfg_["mail_port"].empty()) {
        mail_port_ = jwsmtp::mailer::SMTP_PORT;
        //! @todo catch bad lexical cast
        unsigned short default_port = jwsmtp::mailer::SMTP_PORT;
        cfg_["mail_port"] = lexical_cast_default<std::string>(default_port);
    } else {
        //! @todo catch bad lexical cast
        mail_port_ = lexical_cast_default<unsigned short>(cfg_["mail_port"]);
    }

    //Check if we already have users and
    //if we don't create a child for them
    if(cfg_.child("users")) {
        users_ = cfg_.child("users");
    } else {
        users_ = &(cfg_.add_child("users"));
    }

    std::cout << cfg_ << std::endl;
}

void user_handler::save_config() {
	try {
		scoped_ostream users_file = ostream_file(users_file_);
		write(*users_file , cfg_);
	} catch(io_exception&) {
		std::cerr << "error writing to users file '" << get_prefs_file() << "'\n";
	}
}

bool user_handler::send_mail(const char* to_address, const char* subject, const char* message) {
    std::cout << "sending mail..." << "\n";

    jwsmtp::mailer m(to_address, cfg_["from_address"].c_str(), subject, message,
            cfg_["mail_server"].c_str(), mail_port_, false);
    if(!(cfg_["mail_user"].empty())) {
        m.username(cfg_["mail_user"]);
        m.password( cfg_["mail_password"]);
    }
    //! @todo Sending the mail in a new thread
    //! (as suggested on http://johnwiggins.net/jwsmtp/
    //! might be a very good idea.
    //! To bad I am not familiar with boost::thread

    m.send();

    std::cout << m.response() << "\n";

    if(m.response().substr(0,3) != "250") {
        return false;
    }
    return true;

}

void user_handler::clean_up() {
    //! @todo Write this function :)
}

void user_handler::add_user(const std::string& name,
        const std::string& mail, const std::string& password) {

    //! @todo Check if provided values are sane
    //! (e.g. the email is either empty or looks like user@domain)


    //Check if this user already exists
    if(user_exists(name)) {
        throw error("Could not add new user. A user with the name \"" + name + "\" already exists.");
    }

    //! @todo I guess we should only allow every email address to be registered only once

    config& user = users_->add_child(name);
    user["mail"] = mail;
    user["password"] = password;

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();

    //Don't send a confirmation mail if we don't have an email
    if(mail.empty()) {
        return;
    }

    std::stringstream msg;
    msg << "Thank you for registering on the Battle for Wesnoth Multiplayer Server.\n\n" <<
            "Your username: " << name << "\n" <<
            "Your password: " << user["password"];

    send_mail(user["mail"].c_str(), "Wesnoth Multiplayer Server Registration", msg.str().c_str());
}

void user_handler::password_reminder(const std::string& name) {
    if(!user_exists(name)) {
        throw error("Could not send password reminder. No user with the name \"" + name + "\" exists.");
    }

    config& user = *(users_->child(name));

    if(user["mail"].empty()) {
        throw error("Could not send password reminder. The email address of the user \"" + name + "\" is empty");
    }

    std::stringstream msg;
    msg << "Your username: " << name << "\n" <<
            "Your password: " << user["password"];

    //If sending does not return true warn that no message was sent.
    if(!(send_mail(user["mail"].c_str(), "Wesnoth Multiplayer Server Password Reminder", msg.str().c_str()))) {
        throw error("Could not send password reminder. There was an error sending the reminder email");
    }

}

void user_handler::remove_user(const std::string& name) {
    //Return if the user does not exist
    if(!user_exists(name)) {
        throw error("Could not remove user. No user with the name \"" + name + "\" exists.");
    }
    users_->remove_child(name, 0);

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();
}

bool user_handler::login(const std::string& name, const std::string& password) {
    //Return if the user does not exist
    if(!user_exists(name)) {
        return false;
    }

    config& user = *(users_->child(name));
    return (user["password"] == password);
}

void user_handler::set_user_attribute(const std::string& name,
        const std::string& attribute, const std::string& value) {

    //Return if the user does not exist
    if(!user_exists(name)) {
        throw error("Could not set attribute \"" + attribute  + "\" for user \"" + name +
        "\". No user with the name with this name exists.");
    }

    config& user = *(users_->child(name));
    user[attribute] = value;

    //! @todo To save performance it we should of course not save
    //! the whole config everytime something changes
    save_config();
}

bool user_handler::user_exists(const std::string& name) {
    return ((users_->child(name)));
}
