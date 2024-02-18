/**
* Author: Ren
* Assignment: Simple 2D Scene
* Date due: 2024-02-17, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/



#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

//#include "/home/ren/Downloads/CS3113OfficialWindowsSetup/WindowsSetup/SDL/SDL2/include/SDL_mouse.h"

//additional include for sprite linking
//#include "stb_image.h"
#include "include/stb_image.h"

//#include "SDL.h"
#include "/home/ren/Downloads/CS3113OfficialWindowsSetup/WindowsSetup/SDL/SDL2/include/SDL.h"

//#include "SDL_opengl.h"
#include "/home/ren/Downloads/CS3113OfficialWindowsSetup/WindowsSetup/SDL/SDL2/include/SDL_opengl.h"

//#include "/usr/include/glm/mat4x4.hpp"                // 4x4 Matrix
#include "/home/ren/projects/myGames/include/glm/mat4x4.hpp"                // 4x4 Matrix

#include "include/glm/gtc/matrix_transform.hpp"  // Matrix transformation methods
#include "include/ShaderProgram.h"               // We'll talk about these later in the course

#define LOG(argument) std::cout << argument << '\n'

const char V_SHADER_PATH[] ="/home/ren/projects/myGames/include/shaders/vertex.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment.glsl";

const int WINDOW_WIDTH = 640,
        WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
        BG_BLUE = 0.549f,
        BG_GREEN = 0.9059f,
        BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
        VIEWPORT_Y = 0,
        VIEWPORT_WIDTH = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const int TRIANGLE_RED     = 1.0,
        TRIANGLE_BLUE    = 0.4,
        TRIANGLE_GREEN   = 0.4,
        TRIANGLE_OPACITY = 1.0,
        MAX_FRAME        = 40;  //framerate??

SDL_Window* g_display_window;

bool g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix,
        g_model_matrix,
        g_satellite_matrix, //this is the orbiting object
        g_sub_satellite_matrix, //this the object that orbits the orbiting project
        g_projection_matrix;

// ——————————— GLOBAL VARS AND CONSTS FOR TRANSFORMATIONS ——————————— //

float delta_time;
float g_previous_ticks  = 0.0f;
float speed = 2.0f;
float x_pos = 0.0f;
float y_pos = 0.0f;

const float radius = 2.0f;

glm::vec3 g_player_position = glm::vec3(0.0f, 0.0f, 0.0f);     //
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

//below is texture stuff

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

//const char EARTH_FILEPATH[] = "include/assets/Earth.jpeg";
const char EARTH_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Earth.jpeg";

//const char SUN_FILEPATH[] = "include/assets/Sun.jpeg";
const char SUN_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Sun.png";

//const char MOON_FILEPATH[] = "include/assets/Moon.jpeg";
const char MOON_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Moon.jpeg";

GLuint g_earth_texture_id;
GLuint g_sun_texture_id;
GLuint g_moon_texture_id;

// —————————————————————————————————————————————————————————————————— //


GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        LOG(filepath);
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}



void initialise()
{
//    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Homework1 on Steroid",
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

    g_view_matrix       = glm::mat4(1.0f);
    g_model_matrix      = glm::mat4(1.0f);
    g_satellite_matrix = glm::mat4(1.0f);   //value for orbiting object assigned
    g_sub_satellite_matrix = glm::mat4(1.0f);   //value for sub_orbiting object assigned

    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    g_shader_program.set_colour(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_earth_texture_id = load_texture(EARTH_FILEPATH);
    g_sun_texture_id = load_texture(SUN_FILEPATH);
    g_moon_texture_id = load_texture(MOON_FILEPATH);

//    LOG(g_earth_texture_id);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


enum Coordinate { x_coordinate, y_coordinate };


const Coordinate X_COORDINATE = x_coordinate;
const Coordinate Y_COORDINATE = y_coordinate;

const float ORTHO_WIDTH  = 7.5f,
        ORTHO_HEIGHT = 10.0f;

float get_screen_to_ortho(float coordinate, Coordinate axis)
{
    switch(axis)
    {
        case x_coordinate: return ((coordinate / WINDOW_WIDTH) * ORTHO_WIDTH) - (ORTHO_WIDTH / 2.0);
        case y_coordinate: return (((WINDOW_HEIGHT - coordinate) / WINDOW_HEIGHT) * ORTHO_HEIGHT) - (ORTHO_HEIGHT / 2.0);
        default          : return 0.0f;
    }
}

int mouseX, mouseY;
float orthoX, orthoY;

void process_input()
{
    g_player_movement = glm::vec3(0.0f);

    SDL_GetMouseState(&mouseX, &mouseY);
    orthoX = get_screen_to_ortho(mouseX, X_COORDINATE);
    orthoY = get_screen_to_ortho(mouseY, Y_COORDINATE);


    //
    //
    // –––––––––––––––––––––––––––––––– KEYSTROKES ––––––––––––––––––––––––– //
    //
    SDL_Event event;                                                         //
    while (SDL_PollEvent(&event))                                            //
    {                                                                        //
        switch (event.type)                                                  //
        {                                                                    //
            // End game                                                      //
            case SDL_QUIT:                                                   //
            case SDL_WINDOWEVENT_CLOSE:                                      //
                g_game_is_running = false;                                   //
                break;                                                       //
                //
            case SDL_KEYDOWN:                                                //
                switch (event.key.keysym.sym)                                //
                {                                                            //
                    case SDLK_LEFT:                                          //
                        // Move the player left                              //
                        break;                                               //
                        //
                    case SDLK_RIGHT:                                         //
                        // Move the player right                             //
                        g_player_movement.x = 1.0f;                          //
                        break;                                               //
                        //
                    case SDLK_q:                                             //
                        // Quit the game with a keystroke                    //
                        g_game_is_running = false;                           //
                        break;                                               //
                        //
                    default:                                                 //
                        break;                                               //
                }

                //
                //
            default:                                                         //
                break;                                                       //
        }                                                                    //
    }                                                                        //
    //
    // ––––––––––––––––––––––––––––––– KEY HOLD –––––––––––––––––––––––––––– //
    //
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);                     //
    //
    if (key_state[SDL_SCANCODE_LEFT])                                        //
    {                                                                        //
        g_player_movement.x = -1.0f;                                         //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_RIGHT])                                  //
    {                                                                        //
        g_player_movement.x = 1.0f;                                          //
    }                                                                        //
    //
    if (key_state[SDL_SCANCODE_UP])                                          //
    {                                                                        //
        g_player_movement.y = 1.0f;                                          //
    }                                                                        //
    else if (key_state[SDL_SCANCODE_DOWN])                                   //
    {                                                                        //
        g_player_movement.y = -1.0f;                                         //
    }                                                                        //
    //
    // This makes sure that the player can't "cheat" their way into moving   //
    // faster                                                                //
    if (glm::length(g_player_movement) > 1.0f)                               //
    {                                                                        //
        g_player_movement = glm::normalize(g_player_movement);               //
    }                                                                        //
    // ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––– //
}
void timer(){
    float ticks = (float) SDL_GetTicks() / 1000.0F; // get the current number of ticks
    delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
}

float INIT_TRIANGLE_ANGLE = glm::radians(23.5);

void update()
{
    // ——————————— YOUR ORBIT TRANSFORMATIONS SHOULD GO HERE ——————————— //
    timer();    //makes sure delta time is flowing
    x_pos+=speed*delta_time;    //intuitive: distance = speed * time
    y_pos+=speed*delta_time;
    INIT_TRIANGLE_ANGLE = glm::radians(x_pos*18.0f);
//    g_model_matrix = glm::mat4(1.0f);
//    g_model_matrix = glm::translate(g_model_matrix, glm::vec3 (radius*cos(x_pos), radius*sin(y_pos),0.0f));


    g_player_position += g_player_movement * speed * delta_time;   //
    //
    g_model_matrix = glm::mat4(1.0f);                                       //
//    g_model_matrix = glm::translate(g_model_matrix, g_player_position);
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(orthoX,orthoY,0.0f));
    g_model_matrix = glm::rotate(g_model_matrix, INIT_TRIANGLE_ANGLE, glm::vec3(0.0f, 0.0f, 1.0f));
    g_satellite_matrix = glm::mat4(1.0f);
    g_satellite_matrix = glm::translate(g_model_matrix, glm::vec3 (2.0f * radius*cos(x_pos), radius*sin(y_pos),0.0f));
    g_satellite_matrix = glm::scale(g_satellite_matrix, glm::vec3 (0.8f,0.8f,0.8f));    //scale it down

    g_sub_satellite_matrix = glm::mat4(1.0f);
    g_sub_satellite_matrix = glm::translate(g_satellite_matrix, glm::vec3 (radius*cos(2.0f*x_pos), radius*sin(2.0f*y_pos),0.0f));
    g_sub_satellite_matrix = glm::scale(g_sub_satellite_matrix, glm::vec3 (0.5f,0.5f,0.5f));    //scale it down

    // ————————————————————————————————————————————————————————————————— //
}

//void render() {
//    glClear(GL_COLOR_BUFFER_BIT);
//
//    g_shader_program.set_model_matrix(g_model_matrix);
//
//    float vertices[] =
//            {
//                    0.5f, -0.5f,
//                    0.0f,  0.5f,
//                    -0.5f, -0.5f
//            };
//
//    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
//    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
//    glDrawArrays(GL_TRIANGLES, 0, 3);
//    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
//
//    SDL_GL_SwapWindow(g_display_window);
//}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
//    LOG(GL_TEXTURE_2D);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}


void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] =
            {
                    -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
                    -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,   // triangle 2
            };



    float texture_coordinates[] =
            {
                    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
                    0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
            };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
// Object 1:
//    g_shader_program.set_model_matrix(g_model_matrix);

    draw_object(g_model_matrix, g_sun_texture_id);
//    glBindTexture(GL_TEXTURE_2D, g_AAAAA_texture_id);
//    glDrawArrays(GL_TRIANGLES, 0, 6);
// Object 2:
//    g_shader_program.set_model_matrix(g_satellite_matrix);

    draw_object(g_satellite_matrix, g_earth_texture_id);
//    glBindTexture(GL_TEXTURE_2D, g_BBBBB_texture_id);
//    glDrawArrays(GL_TRIANGLES, 0, 6);

//object3
//    g_shader_program.set_model_matrix(g_sub_satellite_matrix);

    draw_object(g_sub_satellite_matrix, g_moon_texture_id);
//    glBindTexture(GL_TEXTURE_2D, g_BBBBB_texture_id);
//    glDrawArrays(GL_TRIANGLES, 0, 6);


    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}




void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
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
