
/**
* Author: Ren
* Assignment: Rise of the AI
* Date due: 2024-03-23, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 1
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

#define ENEMY_COUNT 1   //add enemies

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

/**
 * 2 notes: I am really not sure if the mixer can work at all
 * And that once I remove this main file from cmake, it will fail to detect SDL, so don't panic
 * Just add it back to cmake and problem solved
 */

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "include/glm/mat4x4.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/ShaderProgram.h"
#include "include/stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "SmartEntity.h"
#include "Map.h"

// ————— GAME STATE ————— //
struct GameState
{
    Entity* player;
    Entity* enemies;

    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

// ————— CONSTANTS ————— //
const int   WINDOW_WIDTH = 1280,
        WINDOW_HEIGHT = 960;

const float BG_RED = 0.1922f,
        BG_BLUE = 0.549f,
        BG_GREEN = 0.9059f,
        BG_OPACITY = 1.0f;

const int   VIEWPORT_X = 0,
        VIEWPORT_Y = 0,
        VIEWPORT_WIDTH = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char GAME_WINDOW_NAME[] = "Hello, Maps!";

const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const char  SPRITESHEET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/george_0.png",
        MAP_TILESET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/tileset.png",
        BGM_FILEPATH[]          = "/home/ren/projects/myGames/include/assets/dooblydoo.mp3",
        JUMP_SFX_FILEPATH[]     = "/home/ren/projects/myGames/include/assets/bounce.wav";
//
//const char  SPRITESHEET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/george_0.png",
//        PLATFORM_FILEPATH[]     = "/home/ren/projects/myGames/include/assets/platformPack_tile027.png",
//        FONT_SPRITE_FILEPATH[]   = "/home/ren/projects/myGames/include/assets/font1.png";

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

unsigned int LEVEL_1_DATA[] =
        {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
                2, 2, 1, 1, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2
        };

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running  = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float   g_previous_ticks = 0.0f,
        g_accumulator = 0.0f;

// ————— GENERAL FUNCTIONS ————— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture_id;
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 4, 1);

    // ————— GEORGE SET-UP ————— //
    // Existing
    g_game_state.player = new Entity();
    g_game_state.player->set_entity_type(PLAYER);
    g_game_state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_speed(2.5f);
    g_game_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    g_game_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    g_game_state.player->m_walking[g_game_state.player->LEFT] = new int[4] { 1, 5, 9, 13 };
    g_game_state.player->m_walking[g_game_state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
    g_game_state.player->m_walking[g_game_state.player->UP] = new int[4] { 2, 6, 10, 14 };
    g_game_state.player->m_walking[g_game_state.player->DOWN] = new int[4] { 0, 4, 8, 12 };

    g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];  // start George looking left
    g_game_state.player->m_animation_frames = 4;
    g_game_state.player->m_animation_index = 0;
    g_game_state.player->m_animation_time = 0.0f;
    g_game_state.player->m_animation_cols = 4;
    g_game_state.player->m_animation_rows = 4;
    g_game_state.player->set_height(0.8f);
    g_game_state.player->set_width(0.8f);

    // Jumping
    g_game_state.player->m_jumping_power = 5.0f;


    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);

    /**
     * if you don want music then comment in the line below
     */
//    Mix_HaltMusic();

    g_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);



    ///////////////////////////below my new stuff//////////////////////////////////////////////////////////////////
    // ————— ENEMY SET-UP ————— //
    // Existing
    g_game_state.enemies = new Entity[ENEMY_COUNT];
    g_game_state.enemies[0].set_entity_type(ENEMY);
    g_game_state.enemies[0].set_ai_type(GUARD);
    g_game_state.enemies[0].set_ai_state(IDLE);
    g_game_state.enemies[0].m_texture_id = load_texture(SPRITESHEET_FILEPATH);
    g_game_state.enemies[0].set_position(glm::vec3(2.5f, 0.0f, 0.0f));
    g_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[0].set_speed(0.5f);
    g_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));



    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running  = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running  = false;
                        break;

                    case SDLK_SPACE:
                        // Jump
                        if (g_game_state.player->m_collided_bottom)
                        {
                            g_game_state.player->m_is_jumping = true;
                            Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                        }
                        break;

                    case SDLK_k:
                        // Jump
                        if (g_game_state.player->m_collided_bottom)
                        {
                            g_game_state.player->m_is_jumping = true;
                            Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                        }
                        break;

                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] || key_state[SDL_SCANCODE_A])
    {
        g_game_state.player->move_left();
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT] || key_state[SDL_SCANCODE_D])
    {
        g_game_state.player->move_right();
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, NULL, 0, g_game_state.map);
        for (int i = 0; i < ENEMY_COUNT; i++) g_game_state.enemies[i].update(FIXED_TIMESTEP, g_game_state.player, NULL, 0, g_game_state.map);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    g_view_matrix = glm::mat4(1.0f);
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_game_state.player->get_position().x, 0.0f, 0.0f));//moving camera to follow player
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);

    for (int i = 0; i < ENEMY_COUNT; i++)    g_game_state.enemies[i].render(&g_shader_program);

    g_game_state.map->render(&g_shader_program);

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_game_state.enemies;
    delete    g_game_state.player;
    delete    g_game_state.map;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running )
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}