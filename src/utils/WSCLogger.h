#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>

#include "WSCCommon.h"

// Convenience macros for logging with source location
#define WSCLog(level, msg) WSCLogger::level(msg, __FILE__, __LINE__, __FUNCTION__)

class WSCLogger {
   public:
    static void init(const std::string& log_name, const std::string& log_file = "log.txt",
                     spdlog::level::level_enum log_level = spdlog::level::info) {
        getInstance().doInit(log_name, log_file, log_level);
    }

    static void setLogLevel(spdlog::level::level_enum level) {
        getInstance().doSetLogLevel(level);
    }

    static void setLogFile(const std::string& log_file) { getInstance().doSetLogFile(log_file); }

    static void setConsoleLogLevel(spdlog::level::level_enum level) {
        getInstance().doSetConsoleLogLevel(level);
    }

    static void setFileLogLevel(spdlog::level::level_enum level) {
        getInstance().doSetFileLogLevel(level);
    }

    static void setPattern(const std::string& pattern) { getInstance().doSetPattern(pattern); }

    static void debug(const std::string& msg, const char* file, int line, const char* function) {
        log_with_location(spdlog::level::debug, msg, file, line, function);
    }

    static void info(const std::string& msg, const char* file, int line, const char* function) {
        log_with_location(spdlog::level::info, msg, file, line, function);
    }

    static void warn(const std::string& msg, const char* file, int line, const char* function) {
        log_with_location(spdlog::level::warn, msg, file, line, function);
    }

    static void error(const std::string& msg, const char* file, int line, const char* function) {
        log_with_location(spdlog::level::err, msg, file, line, function);
    }

   private:
    WSCLogger() = default;
    WSCLogger(const WSCLogger&) = delete;
    WSCLogger& operator=(const WSCLogger&) = delete;

    static WSCLogger& getInstance() {
        static WSCLogger instance;
        return instance;
    }

    static void log_with_location(spdlog::level::level_enum level, const std::string& msg,
                                  const char* file, int line, const char* function) {
        const auto& logger = getInstance().logger;
        if (logger->should_log(level)) {
            std::string filename = std::string(file);
            size_t last_slash = filename.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                filename = filename.substr(last_slash + 1);
            }
            std::string location = "[" + filename + ":" + std::to_string(line) + "]";
            if (level == spdlog::level::trace) {
                location += "[" + std::string(function) + "]";
            }
            logger->log(level, "{} {}", location, msg);
        }
    }

    void doInit(const std::string& log_name, const std::string& log_file,
                spdlog::level::level_enum log_level) {
        std::vector<spdlog::sink_ptr> sinks;

        // Console sink with color
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(console_sink);

        // File sink
        std::string logDir = WSCUtils::getLogDirectory();
        std::string logPath = logDir + "/" + log_file;
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath, true);
        sinks.push_back(file_sink);

        logger = std::make_shared<spdlog::logger>(log_name, begin(sinks), end(sinks));
        doSetPattern("%^[%L]%$ [%Y-%m-%d %H:%M:%S] [thread %t] %v");
        logger->set_level(log_level);
        logger->flush_on(log_level);
    }

    void doSetLogLevel(spdlog::level::level_enum level) { logger->set_level(level); }

    void doSetLogFile(const std::string& log_file) {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        logger->sinks().at(1) = file_sink;
    }

    void doSetConsoleLogLevel(spdlog::level::level_enum level) {
        logger->sinks()[0]->set_level(level);
    }

    void doSetFileLogLevel(spdlog::level::level_enum level) {
        logger->sinks()[1]->set_level(level);
    }

    void doSetPattern(const std::string& pattern) {
        logger->set_pattern(pattern);
        for (auto& sink : logger->sinks()) {
            sink->set_pattern(pattern);
        }
    }
    std::shared_ptr<spdlog::logger> logger;
};