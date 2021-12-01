#pragma once

#include <memory>


struct SDL_Window;

namespace engine {

    void destroySdlWindow(SDL_Window*);

    struct WindowParams {
        WindowParams();

        const char* title;
        int x;
        int y;
        int w;
        int h;
        uint32_t flags;
    };

    using window_t = std::unique_ptr<SDL_Window, decltype(&destroySdlWindow)>;

    window_t createWindow(const WindowParams&);
}

