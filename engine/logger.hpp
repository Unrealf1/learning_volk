#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>

namespace engine {
    
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> get_default_sink();

    std::shared_ptr<spdlog::logger> get_default_logger();
}
