#pragma once

#include <source_location>

#include "logger.hpp"


namespace engine {

    bool check_vk(
        int result,
        std::source_location location = std::source_location::current()
    ) {
        if (result == 0) {
            return true;
        }

        auto logger = get_default_logger();
        logger->error(
            "Vulkan error in function {} ({}:{})", 
            location.function_name(), 
            location.file_name(), 
            location.line()
        );

        return false;
    }
}
