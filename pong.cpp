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

// —— NEW STUFF for the pong game—— //
#include <ctime>   //
#include "cmath"   //
#include <random>  //
// ——————————————— //

/**
 * in the future we could change shaders
 * note we also have two sprites used here
 * they all have to be absolute paths
 * same as other headers, for other OS/envrionments, remove all path components before "include" but keep "include/" itsef
 */
const char V_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/vertex_textured.glsl",
        F_SHADER_PATH[] = "/home/ren/projects/myGames/include/shaders/fragment_textured.glsl";

/**
 * Above is my header setup that I use for all my projects
 */





/**
 * Overall design thinking//scafolding
 *
 *  This will be the last single-file project
 *          -for all future work, switch to more elegant and efficient separate compilation
 *
 *  check y coordinate to wrap around movement of padding
 *          -the decision of wrap around is for more gameplay fun
 *          -too hard. Give up on this.
 *
 *  Wall bounce mechanism
 *          -this is easy, flip y velocity value
 *
 *  Gameover screen?
 *          -tentatively, stop all movements, thats it. Sprite is too much hassle
 *
 *
 */


/**
 * below are all the new consts we need
 */

float left_side_pos = -4.5f;    //where to put the left pad
float right_side_pos = 4.5f;    //where to put right pad

float top_bound = 4.2f;     //screen boundary
float bot_bound = -4.2f;

bool pause = false;             //start and pause the game, toggle on p
bool pve = false;               //switch to player vs machine, toggle on e

float delta_time;
float g_previous_ticks  = 0.0f;

const float g_ai_speed = 1.3f;  // ai speed
float g_left_speed = 2.2f;        //player speed
float g_right_speed = 2.2f;

int rounds = 0;                 //calculate how many rounds have passed
float flip = 1.0f;
float left_flip = 1.0f;
float right_flip = 1.0f;


bool end_game = false;






const int WINDOW_WIDTH  = 1280,
        WINDOW_HEIGHT = 960;

const float BG_RED     = 0.9608f,
        BG_BLUE    = 0.9608f,
        BG_GREEN   = 0.9608f,
        BG_OPACITY = 1.0f;

const int VIEWPORT_X      = 0,
        VIEWPORT_Y      = 0,
        VIEWPORT_WIDTH  = WINDOW_WIDTH,
        VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// absolute path.
const char BANZAI_SPRITE_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Banzai.png";
const char PLAYER_SPRITE_FILEPATH[] = "/home/ren/projects/myGames/include/assets/Goomba.png";

const float MILLISECONDS_IN_SECOND     = 1000.0;
const float MINIMUM_COLLISION_DISTANCE = 0.8f;

const int   NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL    = 0;
const GLint TEXTURE_BORDER     = 0;

SDL_Window* g_display_window;
bool  g_game_is_running = true;

ShaderProgram g_shader_program;
glm::mat4     g_view_matrix,
        g_banzai_matrix,
        g_projection_matrix,
        g_left_matrix,
        g_right_matrix;

GLuint g_goomba_texture_id,
        g_banzai_texture_id;




glm::vec3 g_banzai_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_banzai_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_left_position  = glm::vec3(left_side_pos, 0.0f, 0.0f);      //initialize body on the left side
glm::vec3 g_left_movement  = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_right_position  = glm::vec3(right_side_pos, 0.0f, 0.0f);
glm::vec3 g_right_movement  = glm::vec3(0.0f, 0.0f, 0.0f);      //initialize body on the right side

float g_banzai_speed = 1.0f;

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
    g_display_window = SDL_CreateWindow("Hello, Collisions!",
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

    g_banzai_matrix       = glm::mat4(1.0f);

    g_left_matrix = glm::mat4(1.0f);
    g_left_matrix = glm::translate(g_left_matrix, glm::vec3(left_side_pos, 0.0f, 0.0f));    //keep it to left
    g_left_position    += g_left_movement;

    g_right_matrix = glm::mat4(1.0f);
    g_right_matrix = glm::translate(g_left_matrix, glm::vec3(right_side_pos, 0.0f, 0.0f));    //keep it to right
    g_right_position    += g_right_movement;


    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_goomba_texture_id = load_texture(BANZAI_SPRITE_FILEPATH);
    g_banzai_texture_id  = load_texture(PLAYER_SPRITE_FILEPATH);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void process_input()
{
    g_banzai_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        g_game_is_running = false;
                        break;

                    case SDLK_p:
                        pause = !pause;     //pause and resume the game
                        break;

                    case SDLK_t:
                        pve = !pve;         //switch to pve
                        break;

                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

//    if (key_state[SDL_SCANCODE_LEFT])
//    {
//        g_banzai_movement.x = -1.0f;
//    }
//    else if (key_state[SDL_SCANCODE_RIGHT])     //we need else for opposite side movement
//    {
//        g_banzai_movement.x = 1.0f;
//    }

    if (key_state[SDL_SCANCODE_W] && fabs(g_left_position.y)< top_bound)              //this is player on the left, and also check up/down limit
    {
        g_left_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S] && fabs(g_left_position.y)< top_bound)      //else for opposite direction.
    {
        g_left_movement.y = -1.0f;
    }

    if (key_state[SDL_SCANCODE_UP] && !pve && fabs(g_right_position.y)< top_bound)         //this is the player on the right, remember, if we have pve, we disable player control
    {
        g_right_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] && !pve && fabs(g_right_position.y)< top_bound)      //else for opposite direction.
    {
        g_right_movement.y = -1.0f;
    }

//    if (glm::length(g_banzai_movement) > 1.0f)
//    {
//        g_banzai_movement = glm::normalize(g_banzai_movement);
//    }
}

float measure_distance(glm::vec3 &position_a, glm::vec3 &position_b)  //
{                                                                   //
    // —————————————————  Distance Formula ———————————————————————— //
    return sqrt(                                                    //
            pow(position_b[0] - position_a[0], 2) +             //this is return (x1 - x2)^2 + (y1 - y2)^2 < detection range.
            pow(position_b[1] - position_a[1], 2)               //
    );                         //
}

float detect_y_direction(glm::vec3 &position_a, glm::vec3 &position_b)  //
{                                                                   //
    // —————————————————  Distance Formula ———————————————————————— //
    return   position_a[1] - position_b[1];                                                  //
                                 //
}


void ai_control(){      //only call it when pve is true, we control movement of right player with ai
    float range = measure_distance(g_banzai_position, g_right_position);


    if(g_banzai_movement.y>0){      //a smart(dumb) ai that can speed up with banzai, and does not move if banzai not flying to it
        if(g_banzai_movement.x<2.0f){
            g_right_movement.y = 1.0f;
        }else{
            if(range<2.5f){
                g_right_movement.y = 1.0f * detect_y_direction(g_banzai_position, g_right_position);
            }else{
                g_right_movement.y = 1.5f;
            }
        }
    }else if(g_banzai_movement.y<0){
        if(g_banzai_movement.x<2.0f){
            g_right_movement.y = -1.0f;
        }else{
            if(range<2.5f){
                g_right_movement.y = -1.0f * detect_y_direction(g_banzai_position, g_right_position);
            }else{
                g_right_movement.y = -1.5f;
            }
        }    }else{
        g_right_movement.y = 0.0f;  //stand still
    }

}

void timer(){           //must call this in update() so time will flow!
    float ticks = (float) SDL_GetTicks() / 1000.0F; // get the current number of ticks
    delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
}


// ————————————————————————— NEW STUFF ———————————————————————————— //
bool check_collision(glm::vec3 &position_a, glm::vec3 &position_b)  //
{                                                                   //
    // —————————————————  Distance Formula ———————————————————————— //
    return sqrt(                                                    //
            pow(position_b[0] - position_a[0], 2) +             //this is return (x1 - x2)^2 + (y1 - y2)^2 < detection range.
            pow(position_b[1] - position_a[1], 2)               //
    ) < MINIMUM_COLLISION_DISTANCE;                         //
}                                                                   //
// ———————————————————————————————————————————————————————————————— //


float rand_factor(){
    // Seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Define a distribution for floating point numbers between -1.0f and 1.0f
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // Generate a random float number
    float random_number = dist(gen);

    // Output the random number
    return random_number;
}

void banzai_control(){                  //this one is fully automated
    //this section is mid flight


    //this section is collision
    if(check_collision(g_banzai_position, g_left_position) || check_collision(g_banzai_position, g_right_position)){

        g_banzai_movement.y *= rand_factor();

        rounds++;

        std::cout << std::time(nullptr) << ": Collision.\n";    //

    }

    g_banzai_movement.x = -1.0f * g_banzai_speed + (-0.01f)*sqrt(rounds);
    g_banzai_movement.y =(-1.0f * g_banzai_speed + (-0.01f)*sqrt(rounds));

}

void detect_end_game(){
    if(fabs(g_banzai_position.x)>5.0f){
        end_game = true;
    }
}

void update()
{
    if(!end_game) {     //end game!

        detect_end_game();
        timer();
        if (!pause){        //when we pause we can still control the Goomba! fun!
            banzai_control();
            if (pve) {
                ai_control();
            }
        }

        g_banzai_matrix = glm::mat4(1.0f);



        g_banzai_position += g_banzai_movement * g_banzai_speed * delta_time*flip;
        g_banzai_matrix = glm::translate(g_banzai_matrix, g_banzai_position);

        if(check_collision(g_banzai_position, g_right_position)|| check_collision(g_banzai_position, g_left_position)){
            g_banzai_matrix = glm::rotate(g_banzai_matrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            flip = -flip;
        } //this is 180 deg turn
        if(fabs(g_banzai_position.y)>=4.0f){

        } //this is 180 deg turn





        g_left_matrix = glm::mat4(1.0f);


        g_left_position += g_left_movement * g_left_speed * delta_time*left_flip;
        g_left_matrix = glm::translate(g_left_matrix, g_left_position);
        if(fabs(g_left_position.y)>=4.0f){
            left_flip = -left_flip;
        } //this is 180 deg turn

        g_right_matrix = glm::mat4(1.0f);
        g_right_position += g_right_movement * g_right_speed * delta_time*right_flip;
        g_right_matrix = glm::translate(g_right_matrix, g_right_position);
        if(fabs(g_right_position.y)>=4.0f){
            right_flip = -right_flip;
        } //this is 180 deg turn











    }


    // —————————————————————— NEW STUFF ——————————————————————— //
//    if (check_collision(g_banzai_position, g_left_position))   //
//    {                                                           //
//        std::cout << std::time(nullptr) << ": Collision.\n";    //
//    }                                                           //
    // —————————————————————————————————————————————————————————//
}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = {
            -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
            0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };




    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_banzai_matrix, g_goomba_texture_id);
    draw_object(g_left_matrix, g_banzai_texture_id);
    draw_object(g_right_matrix, g_banzai_texture_id);


    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

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