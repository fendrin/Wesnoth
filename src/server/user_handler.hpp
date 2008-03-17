#ifndef USER_HANDLER_HPP_INCLUDED
#define USER_HANDLER_HPP_INCLUDED

#include "../global.hpp"

#include "../config.hpp"
#include "../log.hpp"
#include "../filesystem.hpp"
#include "../serialization/parser.hpp"
#include "../serialization/preprocessor.hpp"
#include "../serialization/string_utils.hpp"

#include <jwsmtp/jwsmtp.h>

//! @todo Generally write nice log/error messages for all user_handler functions

class user_handler {
    public:
        user_handler(const std::string& users_file);
        ~user_handler();

        config read_config() const;
        void load_config();
        void save_config();

        //! Remove users that registered but did never log in, etc.
        void clean_up();

        //! @todo It might be a good idea to have all these boolean functions
        //! rather throw exceptions to get more detailed error messages

        //! Adds a user.
        //! Returns false if adding fails (e.g. because a user with the same name already exists).
        bool add_user(const std::string& name, const std::string& mail, const std::string& password);
        //! Removes a user-
        //! Returns false if the user did not exist;
        bool remove_user(const std::string& name);

        bool password_reminder(const std::string& name);

        bool set_mail(const std::string& user, const std::string& mail) {
            return set_user_attribute(user, "mail", mail);
        }
        bool set_password(const std::string& user, const std::string& password) {
            return set_user_attribute(user, "password", password);
        }

        bool login(const std::string& name, const std::string& password);
        bool user_exists(const std::string& name);

    private:
        //! Sends an e-mail to a specified address. Requires access to an SMTP server.
        bool send_mail(const char* to_address, const char* subject, const char* message);

        bool set_user_attribute(const std::string& name,
                const std::string& attribute, const std::string& value);

        std::string users_file_;
        unsigned short mail_port_;

        config cfg_;
        config* users_;
};

#endif
