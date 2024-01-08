#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
//#include <GL/gl.h>
#include <GL/glew.h>
#endif

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;
const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

SDL_Window* g_display_window;
bool g_game_is_running = true;

int main(int argc, char* argv[]) 
{
    SDL_Init(SDL_INIT_VIDEO);
    
    g_display_window = SDL_CreateWindow("Hello, World!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    
    SDL_Event event;
    while (g_game_is_running) 
    {
        while (SDL_PollEvent(&event)) 
        {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
            {
                g_game_is_running = false;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(g_display_window);
    }
    
    SDL_Quit();
    return 0;
}