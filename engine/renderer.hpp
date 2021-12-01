#pragma once

#include "window.hpp"

namespace engine {
    
    class Renderer {
    public:
        Renderer(window_t window);
        void render();
    private:
        window_t m_window;
        
    };

}
