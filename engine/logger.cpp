#include "logger.hpp"

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> engine::get_default_sink() {
    static auto default_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    return default_sink;
}

std::shared_ptr<spdlog::logger> engine::get_default_logger() {
    static auto default_logger = std::make_shared<spdlog::logger>("general", get_default_sink());
    return default_logger;
}
