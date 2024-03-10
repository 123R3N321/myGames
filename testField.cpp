/**
I will try to make this into a framework
 **/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define NUMBER_OF_ENEMIES 3
#define FIXED_TIMESTEP 0.0166666f
#define ACC_OF_GRAVITY -9.81f
#define PLATFORM_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "include/glm/mat4x4.hpp"
#include "include/glm/gtc/matrix_transform.hpp"
#include "include/ShaderProgram.h"
#include "include/stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

#include <random>

/**
 * below is a convenient debug tool
 */

#define LOG(argument) std::cout << argument << '\n'



/**
 File path adjusted to relative, only compatible with Debian-based linux
 because the packages for glm and sdl are under "SDL2" directory
 for all other OS, remove "include/" from headers
 **/



/**
 * in the future we could change shaders
 * note we also have two sprites used here
 * they all have to be absolute paths
 * same as other headers, for other OS/envrionments, remove all path components before "include" but keep "include/" itsef
 */
const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl";

const char PLAYER_SPRITE_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Sun.png";
const char OTHER[] = "/home/ren/projects/myGames/include/assets/Earth.jpeg";

const char  SPRITESHEET_FILEPATH[]  = "/home/ren/projects/myGames/include/assets/george_0.png",
        PLATFORM_FILEPATH[]     = "/home/ren/projects/myGames/include/assets/platformPack_tile027.png",
        FONT_SPRITE_FILEPATH[]   = "/home/ren/projects/myGames/include/assets/font1.png";

GLuint  g_text_texture_id;

// ————— STRUCTS AND ENUMS —————//
struct GameState
{
    Entity* player;
    Entity* platforms;
};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH = 640,
        WINDOW_HEIGHT = 480;

const float BG_RED = 0.5f,
        BG_BLUE = 0.5f,
        BG_GREEN = 0.5f,
        BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
        VIEWPORT_Y = 0,
        VIEWPORT_WIDTH = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;


const float MILLISECONDS_IN_SECOND  = 1000.0;

const int NUMBER_OF_TEXTURES = 1;  // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0;  // this value MUST be zero


int target;     //we have a random goal at beginning of game
int endState;   //could be 0, running, 1, win, 2, lose via wrong landing, or 3, fallout
// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_time_accumulator = 0.0f;

const int FONTBANK_SIZE        = 16,
        FRAMES_PER_SECOND    = 4;


int fuel = 9999;

int rand3(){    //create random int among 0, 1, 2, 3
    std::random_device rd;
    std::mt19937 gen(rd()); // Seed the random number generator

    // Define a uniform distribution for integers between 0 and 3
    std::uniform_int_distribution<> dist(0, 3);

    // Generate a random integer between 0 and 3
    return dist(gen);

}

void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
                offset + (-0.5f * screen_size), 0.5f * screen_size,
                offset + (-0.5f * screen_size), -0.5f * screen_size,
                offset + (0.5f * screen_size), 0.5f * screen_size,
                offset + (0.5f * screen_size), -0.5f * screen_size,
                offset + (0.5f * screen_size), 0.5f * screen_size,
                offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
                u_coordinate, v_coordinate,
                u_coordinate, v_coordinate + height,
                u_coordinate + width, v_coordinate,
                u_coordinate + width, v_coordinate + height,
                u_coordinate + width, v_coordinate,
                u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


// ———— GENERAL FUNCTIONS ———— //
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    endState = 0;
    target = rand3();
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("George Lander",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ————— PLAYER ————— //
    // Existing
    g_game_state.player = new Entity();
    g_game_state.player->set_position(glm::vec3(0.0f));
    g_game_state.player->set_movement(glm::vec3(0.0f));
    g_game_state.player->set_acceleration(glm::vec3(0.0f, ACC_OF_GRAVITY * 0.1, 0.0f));
    g_game_state.player->set_speed(1.0f);
    g_game_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    g_game_state.player->m_walking[g_game_state.player->LEFT]   = new int[4] { 1, 5, 9,  13 };
    g_game_state.player->m_walking[g_game_state.player->RIGHT]  = new int[4] { 3, 7, 11, 15 };
    g_game_state.player->m_walking[g_game_state.player->UP]     = new int[4] { 2, 6, 10, 14 };
    g_game_state.player->m_walking[g_game_state.player->DOWN]   = new int[4] { 0, 4, 8,  12 };

    g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];  // start George looking right
    g_game_state.player->m_animation_frames  = 4;
    g_game_state.player->m_animation_index   = 0;
    g_game_state.player->m_animation_time    = 0.0f;
    g_game_state.player->m_animation_cols    = 4;
    g_game_state.player->m_animation_rows    = 4;
    g_game_state.player->set_height(0.9f);
    g_game_state.player->set_width(0.9f);

    // Jumping
    g_game_state.player->m_jumping_power = 3.0f;

    // ————— PLATFORM ————— //
    g_game_state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_game_state.platforms[i].m_texture_id = load_texture(PLATFORM_FILEPATH);
        g_game_state.platforms[i].set_position(glm::vec3(i - 1.0f, -3.0f, 0.0f));
        g_game_state.platforms[i].update(0.0f, NULL, 0);
    }


    glUseProgram(g_shader_program.get_program_id());
    g_text_texture_id = load_texture(FONT_SPRITE_FILEPATH);

    // ————— GENERAL ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void burnFuel(int speed){
    fuel -= speed;
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;

                    case SDLK_SPACE:
                        // Jump
                        if (g_game_state.player->m_collided_bottom) g_game_state.player->m_is_jumping = true;
                        break;

                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] && (fuel>0))
    {
        g_game_state.player->propel('l');
        burnFuel(3);
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT] && (fuel>0))
    {
        g_game_state.player->propel('r');
        burnFuel(3);
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->RIGHT];
    }

    if (key_state[SDL_SCANCODE_UP] && (fuel>0))
    {
        g_game_state.player->propel('u');
        burnFuel(10);
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->UP];
    }
    else if (key_state[SDL_SCANCODE_DOWN] && (fuel>0))
    {
        g_game_state.player->propel('d');
        burnFuel(1);
        g_game_state.player->m_animation_indices = g_game_state.player->m_walking[g_game_state.player->DOWN];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
    {
        g_game_state.player->set_movement(glm::normalize(g_game_state.player->get_movement()));
    }
}


void detectEndState(){
    float vert = g_game_state.player->get_position().y;
    float hori = g_game_state.player->get_position().x;

    if(g_game_state.player->m_collided_bottom){ //we landed
//        LOG("Landed.");
        g_game_state.player->m_is_active = false;
        LOG(endState);
        if((hori<0.5 && 1==target)||(hori>-0.5 && 0==target)){
            endState = 2;
        }else if(fabs(hori)>1){
            endState = 2;
        }else{
            endState = 1;   //win
        }

    }else{  //we just check if we flew too far
        if(fabs(vert)>10 || fabs(hori)>20){
            endState = 3;
            g_game_state.player->m_is_active = false;
        }
    }
}

void endMsg() {
    if (1 == endState) {    //win
        draw_text(&g_shader_program, g_text_texture_id, std::string("Winner!"), 0.25f, 0.0f,
                  glm::vec3(-1.00f, -1.0f, 0.0f));
    } else if (2 == endState) { //offtarget
        draw_text(&g_shader_program, g_text_texture_id, std::string("Off target, but at least you landed."), 0.25f, 0.0f,
                  glm::vec3(-4.00f, -1.0f, 0.0f));
    } else if (3 == endState) {
        draw_text(&g_shader_program, g_text_texture_id, std::string("Into the abyss...DEAD!"), 0.25f, 0.0f,
                  glm::vec3(-3.00f, -1.0f, 0.0f));
    }
}
int frame = 0;
void update()
{

//    LOG(g_game_state.player->get_position().x);
    // ————— DELTA TIME ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;

    // ————— FIXED TIMESTEP ————— //
    // STEP 1: Keep track of how much time has passed since last step
    delta_time += g_time_accumulator;
    detectEndState();
    // STEP 2: Accumulate the ammount of time passed while we're under our fixed timestep
    if (delta_time < FIXED_TIMESTEP)
    {
        g_time_accumulator = delta_time;
        return;
    }

    // STEP 3: Once we exceed our fixed timestep, apply that elapsed time into the objects' update function invocation
    while (delta_time >= FIXED_TIMESTEP)
    {
        // Notice that we're using FIXED_TIMESTEP as our delta time
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_time_accumulator = delta_time;

    if (0==fuel){
        g_game_state.player->m_is_active=false;
    }
}

std::string instruction(){
    if (0==target){ //25% land to left
        return("Land onto the left !");
    }else if(1 == target){  //25% land to right
        return("Land onto the right !");
    }else{  //2 and 3, 50% chance, the easy target
        return("Land onto the middle !");
    }
}

std::string guide(){
    std::string msg;
    float playerVal = g_game_state.player->get_position().x;  //cut off is -1.2:-0.5  -0.5:0.5    0.5:1.2
    if(playerVal<0.5 && 1 == target){   //need to go left
        msg = "move to the right!";
    }else if(playerVal>-0.5 && 0 == target){
        msg = "steer to the left, captain!";
    } else{
        if(fabs(playerVal)>1.2){
            msg = "we are off the edge! Danger! Danger!";
        }else{
            msg = "Maintain this...steady...";
        }
    }
    return msg;
}

void render()
{
    // ————— GENERAL ————— //
    glClear(GL_COLOR_BUFFER_BIT);


    //---text---//
    draw_text(&g_shader_program, g_text_texture_id, std::string("Fuel Remaining: "), 0.25f, 0.0f, glm::vec3(-4.00f, 3.0f, 0.0f));
    draw_text(&g_shader_program, g_text_texture_id, std::string(std::to_string(fuel)), 0.25f, 0.0f, glm::vec3(-4.00f, 2.8f, 0.0f));
    draw_text(&g_shader_program, g_text_texture_id, std::string(instruction()), 0.25f, 0.0f, glm::vec3(-2.00f, 2.0f, 0.0f));
    draw_text(&g_shader_program, g_text_texture_id, std::string(guide()), 0.25f, 0.0f, glm::vec3(-4.00f, 1.5f, 0.0f));
    endMsg();
    // ————— PLAYER ————— //
    g_game_state.player->render(&g_shader_program);

    // ————— PLATFORM ————— //
    for (int i = 0; i < PLATFORM_COUNT; i++) g_game_state.platforms[i].render(&g_shader_program);

    // ————— GENERAL ————— //
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }

// ————— DRIVER GAME LOOP ————— /
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