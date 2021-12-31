#include "window.hpp"
#include <SDL.h>

#include "logger.hpp"

namespace engine {

    WindowParams::WindowParams()
    : title("new window")
    , x(SDL_WINDOWPOS_UNDEFINED)
    , y(SDL_WINDOWPOS_UNDEFINED)
    , w(200)
    , h(200)
    , flags(SDL_WINDOW_OPENGL) 
    {}

    void destroySdlWindow(::SDL_Window* window) {
       SDL_DestroyWindow(window);
    }

    window_t createWindow(const WindowParams& params) {
        static bool sdl_ready = false;
        if (!sdl_ready) {
            if (SDL_Init(SDL_INIT_VIDEO)) {
                auto logger = get_default_logger();
                logger->error("could not initialize SDL: {}", SDL_GetError());
            }
        }

        auto raw = SDL_CreateWindow(
           params.title,
           params.x,
           params.y,
           params.w,
           params.h,
           params.flags
        );

        if (raw == nullptr) {
            auto logger = get_default_logger();
            logger->error("could not create window: {}", SDL_GetError());
        }

        return window_t(raw, destroySdlWindow);
    }
    

    std::pair<int, int> getWindowDimentions(const window_t& window) {
        int width;
        int height;
        
        SDL_GetWindowSize(window.get(), &width, &height);

        return { width, height };
    }
}
