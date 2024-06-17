#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <exception>
#include <source_location>
#include <string>

class RuntimeException : public std::exception {
    std::string message;
public:
    RuntimeException(const std::string &msg, const std::source_location &location)
            : message(msg + " (at " + location.file_name() + ":" + std::to_string(location.line()) + ")") {}

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
};

inline void panic(const std::string &message, const std::source_location &location = std::source_location::current()) {
    throw RuntimeException(message, location);
}

#endif
