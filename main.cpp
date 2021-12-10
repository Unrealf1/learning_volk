#include <iostream>
#include <spdlog/spdlog.h>
#include <SDL_vulkan.h>
#include <SDL.h>

#include "engine/window.hpp"
#include "engine/renderer.hpp"
#include "engine/logger.hpp"

using namespace engine;

int main() {
    auto logger = get_default_logger();
    logger->info("starting tutorial...");

    engine::WindowParams params;
    params.flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	auto renderer = Renderer(engine::createWindow(params));
    renderer.init();
    
    SDL_Event event;
	bool should_quit = false;

	while (!should_quit) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
                should_quit = true;
            }
		}
        renderer.render();
	}
    logger->info("closing application");    
}
