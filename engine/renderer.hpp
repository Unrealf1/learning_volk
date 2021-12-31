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

        VkQueue m_graphics_que;
        uint32_t m_graphics_que_family;

        // this should probably go somewhere else
        VkCommandPool m_command_pool;
        VkCommandBuffer m_command_buffer;
        VkRenderPass m_render_pass;
	    std::vector<VkFramebuffer> m_framebuffers;

        VkSemaphore m_present_semaphore;
        VkSemaphore m_render_semaphore;
        VkFence m_render_fence;

        uint64_t m_frame_number = 0;

        logger_t m_logger;

    private:
        std::optional<vkb::Instance> create_vulk_instance();
        void prepare_commands();
        void prepare_pipelines();

        bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);

    };

}

