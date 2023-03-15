#include "Commands.hpp"

inline bool Commands::bad_format(const Message& msg) {
    return msg.getCommand() == static_cast<std::byte>(Command::BAD_FORMAT);
}

inline bool Commands::bad_values(const Message& msg) {
    return msg.getCommand() == static_cast<std::byte>(Command::BAD_VALUES);
}

/* Quick little function */
inline bool Commands::good(const Message& msg) {
    return msg.getCommand() == static_cast<std::byte>(Command::GOOD_MSG);
}
