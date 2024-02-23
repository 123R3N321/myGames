#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

/**
 * below is a convenient debug tool
 */

#define LOG(argument) std::cout << argument << '\n'



/**
 File path adjusted to relative, only compatible with Debian-based linux
 because the packages for glm and sdl are under "SDL2" directory
 for all other OS, remove "include/" from headers
 **/


#define GL_GLEXT_PROTOTYPES 1
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"
#include "include/glm/mat4x4.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/ShaderProgram.h"
#include "include/stb_image.h"

/**
 * in the future we could change shaders
 * note we also have two sprites used here
 * they all have to be absolute paths
 * same as other headers, for other OS/envrionments, remove all path components before "include" but keep "include/" itsef
 */
const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl";

/**
 * Above is my header setup that i use for all my projects
 */


const int WINDOW_WIDTH  = 640,
        WINDOW_HEIGHT = 480;

const float BG_RED     = 0.9608f,
        BG_BLUE    = 0.9608f,
        BG_GREEN   = 0.9608f,
        BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
        VIEWPORT_Y      = 0,
        VIEWPORT_WIDTH  = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char FLOWER_SPRITE[] = "/home/ren/projects/myGames/include/assets/flower.png";

const glm::vec3 FLOWER_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
        FLOWER_INIT_SCA = glm::vec3(1.5f, 1.5f, 0.0f);

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0,
        TEXTURE_BORDER   = 0;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND     = 90.0f;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_flower_program;
GLuint        g_flower_texture_id;

glm::mat4 g_view_matrix,
        g_flower_model_matrix,
        g_projection_matrix;

float g_previous_ticks  = 0.0f;
float g_rot_angle = 0.0f;

const float ROT_SPEED = 100.0f;
const float TRANS_SPEED = 2.0;

// ———————————————— PART 1 ———————————————— //
/**
 * STEP 1: Set up the necessary data structures

As the notes from last class explain, in order efficiently translate objects in OpenGL, we need:

    A vector to keep track of how much we have moved in all three Cartesian directions.
    A vector to keep track of whether the player has moved in a given frame or not, and if so, in what direction.
    A value that tells update by how much we want to move in any given direction per frame.

 */


// ———————————————— PART 1 ———————————————— //


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("User input exercise",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    g_flower_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_flower_model_matrix = glm::mat4(1.0f);
    g_flower_model_matrix = glm::translate(g_flower_model_matrix, FLOWER_INIT_POS);
    g_flower_model_matrix = glm::scale(g_flower_model_matrix, FLOWER_INIT_SCA);

    g_flower_program.set_projection_matrix(g_projection_matrix);
    g_flower_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_flower_program.get_program_id());
    g_flower_texture_id = load_texture(FLOWER_SPRITE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:

            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = !g_game_is_running;
                break;

                // ———————————————— BONUS ———————————————— //
                /**
                 * BONUS POINT: Apply rotation mechanism

To earn a bonus point for this exercise, you will add a
                 rotation mechanism depending on the user input:

    r key: Our flower would start rotating clockwise (without stopping)
    c key: Our flower would start rotating counter-clockwise (without stopping)
    s key: Our flower would stop any rotation

For this part of the exercise, you should use keystrokes i
                 */

                // ———————————————— BONUS ———————————————— //
        }
    }

    // ———————————————— PART 2 ———————————————— //
/**
 * STEP 2: Keep track of whether the user wants to move or not

The second data structure from step 1 is used to keep track of whether
 the user wants to move in a given direction. That is, if the user is holding down the:

    Up-arrow key: Our data structure would mark a 1.0f in the y-direction.
    Down-arrow key: Our data structure would mark a -1.0f in the y-direction.
    Left-arrow key: Our data structure would mark a -1.0f in the x-direction.
    Right-arrow key: Our data structure would mark a 1.0f in the x-direction.

You can check out the relevant notes here (code block 5).
 Also remember that if the user releases the key, the
 flower should stop moving as well (to do this you would need to reset the movement vector).

Keep in mind that, if the user is moving in both the x- and y-direction,
 you will need to normalise this behaviour so that the sprite doesn't move faster than necessary.
 */
    // ———————————————— PART 2 ———————————————— //
}


void update()
{
    /** ———— DELTA TIME CALCULATIONS ———— **/
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // ———————————————— PART 3 ———————————————— //
//todo: transformation

    /** ———— RESETTING MODEL MATRIX ———— **/
    g_flower_model_matrix = glm::mat4(1.0f);
    g_flower_model_matrix = glm::scale(g_flower_model_matrix, FLOWER_INIT_SCA);

    /** ———— ROTATING SPRITE ———— **/
    g_flower_model_matrix = glm::rotate(g_flower_model_matrix, glm::radians(g_rot_angle), glm::vec3(0.0f, 1.0f, 0.0f));
    // ———————————————— PART 3 ———————————————— //
//todo: transformation

}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float flower_vertices[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float flower_texture_coordinates[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(g_flower_program.get_position_attribute(), 2, GL_FLOAT, false, 0, flower_vertices);
    glEnableVertexAttribArray(g_flower_program.get_position_attribute());

    glVertexAttribPointer(g_flower_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, flower_texture_coordinates);
    glEnableVertexAttribArray(g_flower_program.get_tex_coordinate_attribute());

    g_flower_program.set_model_matrix(g_flower_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_flower_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_flower_program.get_position_attribute());
    glDisableVertexAttribArray(g_flower_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}