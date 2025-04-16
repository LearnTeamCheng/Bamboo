#pragma once

namespace Bamboo
{
    class SDL_Window;
    class Graphics
    {
    public:
        SDL_Window *GetWindow() const { return m_window; }

    private:
        SDL_Window *m_window;
    };
}
