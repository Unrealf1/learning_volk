#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>

namespace engine {
    
    using logger_t = std::shared_ptr<spdlog::logger>;

    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> get_default_sink();

    logger_t get_default_logger();

    logger_t create_logger(const std::string& name);
}
