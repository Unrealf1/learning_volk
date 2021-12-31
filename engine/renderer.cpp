#include "renderer.hpp"
#include "vk_util.hpp"

#include <SDL_vulkan.h>

#include <optional>
#include <fstream>
#include <filesystem>


namespace engine {

    Renderer::Renderer(window_t window)
        : m_window(std::move(window))
        , m_logger(create_logger("render")) {}

    Renderer::~Renderer() {
        vkDestroyCommandPool(
            m_device, 
            m_command_pool, 
            nullptr
        );

        vkDestroySwapchainKHR(
            m_device,
            m_swapchain,
            nullptr
        );

        vkDestroyRenderPass(
            m_device,
            m_render_pass,
            nullptr
        );

        for (auto& framebuffer : m_framebuffers) {
            vkDestroyFramebuffer(
                m_device,
                framebuffer,
                nullptr
            );
        }

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
        // creating instance
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

        // creating device
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

        auto [window_width, window_height] = getWindowDimentions(m_window);

        vkb::Swapchain vkb_swapchain = swapchain_builder
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(window_width, window_height) 
            .build()
            .value();

        m_swapchain = vkb_swapchain.swapchain;
        m_swapchain_images = vkb_swapchain.get_images().value();
        m_swapchain_image_views = vkb_swapchain.get_image_views().value();

        m_swapchain_image_format = vkb_swapchain.image_format;

        // creating que
        m_graphics_que = vkb_device.get_queue(vkb::QueueType::graphics).value();
        m_graphics_que_family = vkb_device.get_queue_index(
            vkb::QueueType::graphics
        ).value();

        prepare_commands();
        prepare_pipelines();

        return true;
    }

    void Renderer::render() {
        check_vk(vkWaitForFences(m_device, 1, &m_render_fence, true, 1000000000));//1sec
        check_vk(vkResetFences(m_device, 1, &m_render_fence));

        uint32_t swapchain_image_index;
        check_vk(vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, m_present_semaphore, nullptr, &swapchain_image_index));
        
        check_vk(vkResetCommandBuffer(m_command_buffer, 0));
        
        // create command buffer
        VkCommandBuffer cmd = m_command_buffer;
       	VkCommandBufferBeginInfo begin_info = {};
       	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
       	begin_info.pNext = nullptr;
       	begin_info.pInheritanceInfo = nullptr;
       	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
       	check_vk(vkBeginCommandBuffer(cmd, &begin_info));

       	VkClearValue clearValue;
       	float flash = abs(sin(m_frame_number / 120.f));
       	clearValue.color = { { 1.0f, flash, 0.0f, 1.0f } };
       
       	//start the main renderpass.
       	VkRenderPassBeginInfo rpInfo = {};
       	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
       	rpInfo.pNext = nullptr;
       	rpInfo.renderPass = m_render_pass;
       	rpInfo.renderArea.offset.x = 0;
       	rpInfo.renderArea.offset.y = 0;
       	//rpInfo.renderArea.extent = _windowExtent;
       	rpInfo.framebuffer = m_framebuffers[swapchain_image_index];
       	rpInfo.clearValueCount = 1;
       	rpInfo.pClearValues = &clearValue;
       	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdEndRenderPass(cmd);
        check_vk(vkEndCommandBuffer(cmd));

        VkSubmitInfo submit = {};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
       	submit.pNext = nullptr;
       	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
       	submit.pWaitDstStageMask = &waitStage;
       	submit.waitSemaphoreCount = 1;
       	submit.pWaitSemaphores = &m_present_semaphore;
       	submit.signalSemaphoreCount = 1;
       	submit.pSignalSemaphores = &m_render_semaphore;
       	submit.commandBufferCount = 1;
       	submit.pCommandBuffers = &cmd;
        check_vk(vkQueueSubmit(m_graphics_que, 1, &submit, m_render_fence));

        VkPresentInfoKHR present_info = {};
       	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
       	present_info.pNext = nullptr;
       	present_info.pSwapchains = &m_swapchain;
       	present_info.swapchainCount = 1;
       	present_info.pWaitSemaphores = &m_render_semaphore;
       	present_info.waitSemaphoreCount = 1;
       	present_info.pImageIndices = &swapchain_image_index;

       	check_vk(vkQueuePresentKHR(m_graphics_que, &present_info));

        ++m_frame_number;
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

    void Renderer::prepare_commands() {
        // creating command pool
        VkCommandPoolCreateInfo pool_info;
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.pNext = nullptr;
        pool_info.queueFamilyIndex = m_graphics_que_family;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        check_vk(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool));
        
        // creating command buffer
       	VkCommandBufferAllocateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_info.pNext = nullptr;
        buffer_info.commandPool = m_command_pool;
        buffer_info.commandBufferCount = 1;
        buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        check_vk(vkAllocateCommandBuffers(m_device, &buffer_info, &m_command_buffer));

        // create renderpass
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = m_swapchain_image_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        check_vk(vkCreateRenderPass(
            m_device, 
            &render_pass_info, 
            nullptr, 
            &m_render_pass
        ));

        auto [window_width, window_height] = getWindowDimentions(m_window);
        // create framebuffers
        VkFramebufferCreateInfo fb_info = {};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = nullptr;
        fb_info.renderPass = m_render_pass;
        fb_info.attachmentCount = 1;
        fb_info.width = window_width; 
        fb_info.height = window_height;
        fb_info.layers = 1;
        const uint32_t swapchain_imagecount = m_swapchain_images.size();
        m_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);
        for (int i = 0; i < swapchain_imagecount; ++i) {
            fb_info.pAttachments = &m_swapchain_image_views[i];
            check_vk(vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_framebuffers[i]));
        }

        // create sync
        VkFenceCreateInfo fence_create_info = {};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.pNext = nullptr;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        check_vk(vkCreateFence(m_device, &fence_create_info, nullptr, &m_render_fence));

        VkSemaphoreCreateInfo semaphore_create_info = {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext = nullptr;
        semaphore_create_info.flags = 0;
        check_vk(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_present_semaphore));
        check_vk(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_render_semaphore));

    }

    void Renderer::prepare_pipelines() {
        VkShaderModule triangleFragShader;
        if (!load_shader_module("resources/shaders/frag.spv", &triangleFragShader)) {
            m_logger->error("failed to load shader");
        }

        VkShaderModule triangleVertShader;
        if (!load_shader_module("resources/shaders/vert.spv", &triangleVertShader)) {
            m_logger->error("failed to load shader");
        }
    }


    bool Renderer::load_shader_module(const char* filePath, VkShaderModule* outShaderModule) {
        std::ifstream shader_file(filePath, std::ios::binary);

        if (!shader_file.is_open()) {
            return false;
        }
        auto filesize = std::filesystem::file_size(filePath);
        std::vector<uint32_t> shader_code(filesize / sizeof(uint32_t));

        m_logger->info("loading shader of size {}", filesize);
        m_logger->info("code size is {}", shader_code.size());
        shader_file.read(reinterpret_cast<char*>(shader_code.data()), filesize);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;

        createInfo.codeSize = shader_code.size() * sizeof(uint32_t);
        createInfo.pCode = shader_code.data();

        //check that the creation goes well.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return false;
        }
        *outShaderModule = shaderModule;
        return true;
    }
    
}

