#include "renderer.hpp"
#include "vk_util.hpp"

#include <SDL_vulkan.h>

#include <optional>


namespace engine {

    Renderer::Renderer(window_t window)
        : m_window(std::move(window))
        , m_logger(create_logger("render")) {}

    Renderer::~Renderer() {
        vkDestroySwapchainKHR(
            m_device,
            m_swapchain,
            nullptr
        );

        for (auto& image_view : m_swapchain_image_views) {
            vkDestroyImageView(
                m_device,
                image_view,
                nullptr
            );
        }

        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkb::destroy_debug_utils_messenger(m_instance, m_vk_debug_messenger);
		vkDestroyInstance(m_instance, nullptr);
    }
    
    bool Renderer::init() {
        auto vkb_inst = create_vulk_instance();
        if (!vkb_inst.has_value()) {
            return false;
        }

        m_instance = vkb_inst.value().instance;
        m_vk_debug_messenger = vkb_inst.value().debug_messenger;
        
	    if(!SDL_Vulkan_CreateSurface(m_window.get(), m_instance, &m_surface)) {
            m_logger->critical("Failed to create vulkan surface");
            return false;
        }

        vkb::PhysicalDeviceSelector selector{ vkb_inst.value() };
        vkb::PhysicalDevice vkb_physical_device = selector
            .set_minimum_version(1, 1)
            .set_surface(m_surface)
            .select()
            .value();

        vkb::DeviceBuilder device_builder{ vkb_physical_device };

        vkb::Device vkb_device = device_builder.build().value();

        m_device = vkb_device.device;
        m_physical_device = vkb_physical_device.physical_device;

        m_logger->info("vulkan instance created successfully");

        // creating swapchain
   	    vkb::SwapchainBuilder swapchain_builder{
            m_physical_device,
            m_device,
            m_surface 
        };

        vkb::Swapchain vkb_swapchain = swapchain_builder
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            //.set_desired_extent(width, height)
            .build()
            .value();

        m_swapchain = vkb_swapchain.swapchain;
        m_swapchain_images = vkb_swapchain.get_images().value();
        m_swapchain_image_views = vkb_swapchain.get_image_views().value();

        m_swapchain_image_format = vkb_swapchain.image_format;


        return true;
    }

    void Renderer::render() {
        
    }

    std::optional<vkb::Instance> Renderer::create_vulk_instance() {
        vkb::InstanceBuilder builder;
        auto inst_ret = builder
            .set_app_name("Example Vulkan Application")
            .request_validation_layers(true)
            .require_api_version(1, 1, 0)
            .use_default_debug_messenger()
            .build();

        if (!inst_ret) {
            m_logger->critical(
                "Failed to create Vulkan instance. Error: {}",
                inst_ret.error().message()
            );
            return {};
        }

        vkb::Instance vkb_inst = inst_ret.value();
        return vkb_inst;
    }

    
}

