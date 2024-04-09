// Various quality of life logging things with spdlog
#pragma once

#include <csignal>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#define $trace(...) SPDLOG_TRACE(__VA_ARGS__)
#define $debug(...) SPDLOG_DEBUG(__VA_ARGS__)
#define $info(...) SPDLOG_INFO(__VA_ARGS__)
#define $warn(...) SPDLOG_WARN(__VA_ARGS__)
#define $error(...) SPDLOG_ERROR(__VA_ARGS__)
#define $critical(...) SPDLOG_CRITICAL(__VA_ARGS__)

void setupLogging();

/* clang-format off */
#ifdef BUILD_DEBUG
    /**
     * @brief assertion with critical logging level
     * @param condition condition to assert
     * @param ... arguments to log
     */
    #define $assert(condition, ...) {if (!(condition)) $critical(__VA_ARGS__); };
    
    /**
     * @brief assertion with given logging level
     * @param level spdlog logging level
     * @param condition condition to assert
     * @param ... arguments to log
     */
    #define $assert_lvl(level, condition, ...) {if (!(condition)) SPDLOG_LOGGER_CALL(spdlog::default_logger_raw(), (level), __VA_ARGS__);};
#else
    #define $assert(condition, ...)             { (condition); }
    #define $assert_lvl(lvl, condition, ...)    { (condition); }
#endif
/* clang-format on */

// Concept for having a std::string _format() const method
template <typename T>
concept c_Formattable = requires(const T& a) {
    {
        a._format()
    } -> std::convertible_to<std::string>;
};

std::ostream& operator<<(std::ostream& os, const c_Formattable auto& obj)
{
    return os << obj._format();
}
