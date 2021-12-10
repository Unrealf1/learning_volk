#pragma once

#include "window.hpp"
#include "logger.hpp"

#include <VkBootstrap.h>

#include <optional>


namespace engine {
    
    class Renderer {
    public:
        Renderer(window_t window);
        ~Renderer();

        bool init();
        void render();

    private:
        window_t m_window;

        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_vk_debug_messenger;
        VkPhysicalDevice m_physical_device; 
	    VkDevice m_device;
        VkSurfaceKHR m_surface;
        VkSwapchainKHR m_swapchain; 
	    VkFormat m_swapchain_image_format;

        std::vector<VkImage> m_swapchain_images;
        std::vector<VkImageView> m_swapchain_image_views;

        logger_t m_logger;

    private:
        std::optional<vkb::Instance> create_vulk_instance();
    };

}

